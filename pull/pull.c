#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h> 
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>

AVFormatContext *in_format_ctx;
AVPacket *pkt;
AVStream *stream;
AVCodecContext *pCodecCtx;
AVCodec *pCodec;
AVFrame *per_frame;

#define URL "rtmp://120.77.214.213:1935/live_video/video"
#define OUT "helloworld.pnm"
#define JPG "hello.jpg"
#define VERSION 1.01
#define FLAG 1
#define MJPG 1
void generate_file_name(int index,char *filename)
{
	if (!index)
		return;
	memset(filename,0,strlen(filename));
	sprintf(filename,"jpg/no_%d.jpg",index);
}


int save_jpeg(AVFrame *pFrame, char *out_name)
{
    AVCodecContext *pCodeCtx = NULL;
    
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //output format as mjpeg
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);

    if (avio_open(&pFormatCtx->pb, out_name, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Couldn't open output file.");
        return -1;
    }

    AVStream *pAVStream = avformat_new_stream(pFormatCtx, 0);
    if (pAVStream == NULL) {
        return -1;
    }

    AVCodecParameters *parameters = NULL;
    parameters = pAVStream->codecpar;
    parameters->codec_id = pFormatCtx->oformat->video_codec;
    parameters->codec_type = AVMEDIA_TYPE_VIDEO;
    parameters->format = AV_PIX_FMT_YUVJ420P;
    parameters->width = pFrame->width;
    parameters->height = pFrame->height;

    AVCodec *pCodec = avcodec_find_encoder(pAVStream->codecpar->codec_id);

    if (!pCodec) {
        printf("Could not find encoder\n");
        return -1;
    }

    pCodeCtx = avcodec_alloc_context3(pCodec);
    if (!pCodeCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    if ((avcodec_parameters_to_context(pCodeCtx, pAVStream->codecpar)) < 0) {
        fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        return -1;
    }

    pCodeCtx->time_base = (AVRational) {1, 25};

	if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.");
        return -1;
    }

    int ret = avformat_write_header(pFormatCtx, NULL);
    if (ret < 0) {
        printf("write_header fail\n");
        return -1;
    }

    //Encode
    AVPacket pkt;
    av_new_packet(&pkt, (pFrame->width)*(pFrame->height) * 3);

	/*send frame*/
    ret = avcodec_send_frame(pCodeCtx, pFrame);
    if (ret < 0) {
        printf("Could not avcodec_send_frame.");
        return -1;
    }
	/*receive decode packet*/
    ret = avcodec_receive_packet(pCodeCtx, &pkt);
    if (ret < 0) {
        printf("Could not avcodec_receive_packet");
        return -1;
    }
	
	/*write the data to file*/
    ret = av_write_frame(pFormatCtx, &pkt);

    if (ret < 0) {
        printf("Could not av_write_frame");
        return -1;
    }

    av_packet_unref(&pkt);

	/*write the end of file*/
    av_write_trailer(pFormatCtx);

    avcodec_close(pCodeCtx);
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
	return 0;
}

void init_register_network()
{
		/*
		*	Init the network
		*	register all av
		*/
       	av_register_all();
       	avformat_network_init();
}

void test_ffmpeg_rtmp_client()
{
	int i;
	int ret = -1;
	int video_stream_index = -1;
	int audio_stream_index = -1;
	int index = 0;
	char filename[32] = "";

	//init register and network
	init_register_network();

	//alloc context
	in_format_ctx = avformat_alloc_context();

	//open video file -> rtmp path
	ret = avformat_open_input(&in_format_ctx, URL, NULL, NULL);
	if (ret < 0)
	{
		fprintf(stderr, "fail to open url: %s, return value: %d\n", URL, ret);
		return;
	}

	// Read packets of a media file to get stream information
	ret = avformat_find_stream_info(in_format_ctx, NULL);
	if (ret < 0) 
	{
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return;
	}
	
	/*Fetch index of audio and video from streams*/ 
	for (i = 0; i < in_format_ctx->nb_streams; i++) 
	{
		stream = in_format_ctx->streams[i];
		fprintf(stdout, "type of the encoded data: %d\n", stream->codecpar->codec_id);
		//video type
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) 
			video_stream_index = i;
		//audio type
		else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			audio_stream_index = i;
	}
 
	if (video_stream_index == -1 || audio_stream_index == -1)
	{
		fprintf(stderr, "no video audio stream\n");
		return;
	}

	/*Fetch the context from decoder and coder from video*/
	pCodecCtx = in_format_ctx->streams[video_stream_index]->codec;
	
	/*identity*/
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (NULL == pCodec)
	{
		printf("could not find decoder......\n");
		return;
	}

	//open decoder 
	if (avcodec_open2(pCodecCtx,pCodec,NULL)<0)
	{
		printf("avcodec open failed\n");
		return;
	}
	printf("The width of video is %d, while the height is %d\n",pCodecCtx->width,pCodecCtx->height);

	//alloc for rgb memory
	per_frame = av_frame_alloc();

	/*
	*	read each frame using av_read_frame
	*	check the index of av
	*	write in file 
	*/

	/*avoid segmentation fault*/
	pkt = av_packet_alloc();
	av_dump_format(in_format_ctx,0,OUT,0);

	while (1) 
	{
		//read each frame
		ret = av_read_frame(in_format_ctx, pkt);
		if (ret < 0)
		{
			fprintf(stderr, "error or end of file: %d\n", ret);
			continue;
		}
		fprintf(stdout, "video stream, packet size: %d\n", pkt->size);
		//index of av 
		if (pkt->stream_index == video_stream_index)
		{
			/*pkt->size should not be zero, if it is, it should be break*/
			if (ret < 0 && !(pkt->size))
			{
				printf("avcodec_decode_video2 error\n");
				return;
			}
			/*If one picture comes it should be storage.*/
			if (FLAG)
			{
				index++;
#if MJPG
				/*avcodec send packet*/
				if (avcodec_send_packet(pCodecCtx,pkt) < 0)
					continue;
				/*receive packet*/
				while(avcodec_receive_frame(pCodecCtx,per_frame) == 0)
				{
					generate_file_name(index,filename);
					ret = save_jpeg(per_frame,filename);//save image as jpeg	
					if (ret == -1)
						return;
				}
#endif
				/*receive the rtmp stream stored in local file:mp4 format*/

			}

		}
 
		if (pkt->stream_index == audio_stream_index)
			fprintf(stdout, "audio stream, packet size: %d\n", pkt->size);
 
		av_packet_unref(pkt);
	}

	//free all 
	avformat_free_context(in_format_ctx);
	avcodec_close(pCodecCtx);
	avformat_close_input(&in_format_ctx);
}
int main(int argc, char **argv)
{	
	test_ffmpeg_rtmp_client();
	return 0;
}
