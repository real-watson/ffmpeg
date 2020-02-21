#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h> 
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>

AVFormatContext *format_ctx;
AVPacket *pkt;
AVFrame *pFrame;
AVFrame *pFrameYUV;
AVStream *stream;
AVCodecContext *pCodecCtx;
AVCodec *pCodec;


#define URL "rtmp://120.77.214.213:1935/live_video/video"
#define OUT "helloworld.yuv"

void init_register_network()
{
		/*
		*	Init the network
		*	register all av
		*/
       	av_register_all();
       	avformat_network_init();
}

int test_ffmpeg_rtmp_client()
{
	FILE *video = NULL;
	int i;
	int ret = -1;
	int got_picture = -1;
	int frame_output = 0;// 帧数
	int video_stream_index = -1;
	int audio_stream_index = -1;

	//open FILE video
	video = fopen(OUT,"wb");
	if (NULL == video)
	{
		printf("Error in fopen\n");
		return -1;
	}

	//init register and network
	init_register_network();

	//alloc context
	format_ctx = avformat_alloc_context();

	//open video file -> rtmp path
	ret = avformat_open_input(&format_ctx, URL, NULL, NULL);
	if (!ret)
	{
		fprintf(stderr, "fail to open url: %s, return value: %d\n", URL, ret);
		return -1;
	}

	// Read packets of a media file to get stream information
	ret = avformat_find_stream_info(format_ctx, NULL);
	if (ret < 0) 
	{
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}
	
	/*Fetch index of audio and video from streams*/ 
	for (i = 0; i < format_ctx->nb_streams; i++) 
	{
		stream = format_ctx->streams[i];
		fprintf(stdout, "type of the encoded data: %d\n", stream->codecpar->codec_id);
		//video type
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) 
			video_stream_index = i;
		//audio type
		else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			audio_stream_index = i;
		else
			fprintf(stdout, "none sample format\n");
	}
 
	if (video_stream_index == -1 || audio_stream_index == -1)
	{
		fprintf(stderr, "no video audio stream\n");
		return -1;
	}

	/*Fetch the context from decoder and coder from video*/
	pCodecCtx = format_ctx->streams[video_stream_index]->codec;
	
	/*identity*/
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (NULL == pCodec)
	{
		printf("could not find decoder......\n");
		return -1;
	}

	//open decoder 
	if (avcodec_open2(pCodecCtx,pCodec,NULL)<0)
	{
		printf("avcodec open failed\n");
		return -1;
	}

	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	//The size of video
	struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,SWS_BICUBIC, NULL, NULL, NULL);

	/*
	*	read each frame using av_read_frame
	*	check the index of av
	*	write in file 
	*/
	while (1) 
	{
		//read each frame
		ret = av_read_frame(format_ctx, pkt);
		if (ret < 0)
		{
			fprintf(stderr, "error or end of file: %d\n", ret);
			continue;
		}
		//index of av 
		if (pkt->stream_index == video_stream_index)
		{
			printf("video_stream_index......\n");
			fprintf(stdout, "video stream, packet size: %d\n", pkt->size);

			//decodering video		
			ret = avcodec_decode_video2(pCodecCtx,pFrame,&got_picture,pkt);

			/*pkt->size should not be zero, if it is, it should be break*/
			if (ret < 0 && !(pkt->size))
			{
				printf("decoder error......\n");
				return -1;
			}

			//If the frame is not zero, it should be return got_picture.
		 	if (got_picture)
			{
				sws_scale(sws_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height,pFrameYUV->data, pFrameYUV->linesize);

				frame_output++;
				printf("The %d frame\n",frame_output);

				//save image for only one picture
				if (frame_output == 1)
					fwrite(pFrameYUV->data[0],(pCodecCtx->width)*(pCodecCtx->height)*3,1,video); 
			}

		}
 
		if (pkt->stream_index == audio_stream_index)
			fprintf(stdout, "audio stream, packet size: %d\n", pkt->size);
 
		av_packet_unref(pkt);
	}

	//free all 
	avformat_free_context(format_ctx);
	fclose(video); 
	avcodec_close(pCodecCtx);
	av_frame_free(&pFrame);
	return 0;
}
int main(int argc, char **argv)
{	
	if (-1 == test_ffmpeg_rtmp_client())
	{
		printf("test_ffmpeg_rtmp_client error\n");
	}
	return 0;
}
