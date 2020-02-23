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
#define OUT "helloworld.flv"
#define JPG "hello.jpg"
#define VERSION 1.01
#define FLAG 1

/*choose one of two methods*/
#define MJPG 0
#define RTMP 1/*always do it not*/

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

    /*output format as mjpeg*/
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
    parameters = pAVStream->codecpar;/*New stream sets parameters*/
    parameters->codec_id = pFormatCtx->oformat->video_codec;
    parameters->codec_type = AVMEDIA_TYPE_VIDEO;
    parameters->format = AV_PIX_FMT_YUVJ420P;/*Video format setting*/
    parameters->width = pFrame->width;
    parameters->height = pFrame->height;

	/*find encoder for this format*/
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
	
	/*copied parameters to context*/
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

    /*Encode*/
    AVPacket pkt;
    av_new_packet(&pkt, (pFrame->width)*(pFrame->height) * 3);
	
	/*
	* sending frame to the encoder
	* recving packet which encoded from encoder
	* pkt has this encoding data
	*/

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
#if RTMP
	av_dump_format(in_format_ctx,0,URL,0);
	AVFormatContext *out_format_ctx = NULL;

	/* set context from OUT to out_format_ctx*/
	avformat_alloc_output_context2(&out_format_ctx, NULL, NULL,OUT);
	AVOutputFormat* ofmt = NULL;
	ofmt = out_format_ctx->oformat;

	/*set out_stream with in_stream's codec*/
	for (i = 0; i < in_format_ctx->nb_streams; i++)
	{
		/*Add more stream channel*/
		AVStream *in_stream;
		AVStream *out_stream;

		in_stream = in_format_ctx->streams[i];
		out_stream = avformat_new_stream(out_format_ctx, in_stream->codec->codec);

		ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
		out_stream->codec->codec_tag = 0;

		if (out_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	av_dump_format(out_format_ctx, 0, OUT, 1);

	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		/*open OUT for out_format_ctx's pb just like New something*/
		ret = avio_open(&out_format_ctx->pb,OUT, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("Could not open output URL '%s'", OUT);
			return;
		}
	}
	/*Write head to file out_format_ctx as OUT*/
	avformat_write_header(out_format_ctx,NULL);
#endif

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
#if MJPG
	per_frame = av_frame_alloc();
#endif
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
		
		/*receive the rtmp stream stored in local file:mp4 format*/
#if RTMP
		if (pkt->size != 0)
		{
			AVStream* in_stream, * out_stream;

			in_stream = in_format_ctx->streams[pkt->stream_index];
			out_stream = out_format_ctx->streams[pkt->stream_index];

			/*
			*	PTS as presentation time stamp
			*	DTS as display time stamp
			*	When without B frame, PTS and DTS are the same
			*/

			pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, out_stream->time_base);
			pkt->pos = 1;
			
			/*write frame to the OUT*/
			ret = av_interleaved_write_frame(out_format_ctx,pkt);
			if (ret < 0)
			{
				if (ret == -22)
					continue;
				else
					break;

			}
		}//end of if
#endif
		/*Check whether the index is video stream*/
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
			}

		}
 
		if (pkt->stream_index == audio_stream_index)
			fprintf(stdout, "audio stream, packet size: %d\n", pkt->size);
 
		av_packet_unref(pkt);
	}
#if RTMP
	av_write_trailer(out_format_ctx);
#endif
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
