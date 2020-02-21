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

void test_ffmpeg_rtmp_client()
{
	FILE *video = NULL;
	int i;
	int ret = -1;
	int video_stream_index = -1;
	int audio_stream_index = -1;

	//open FILE video
	video = fopen(OUT,"wb");
	if (NULL == video)
	{
		printf("Error in fopen\n");
		return;
	}

	//init register and network
	init_register_network();

	//alloc context
	format_ctx = avformat_alloc_context();

	//open video file -> rtmp path
	ret = avformat_open_input(&format_ctx, URL, NULL, NULL);
	if (ret < 0)
	{
		fprintf(stderr, "fail to open url: %s, return value: %d\n", URL, ret);
		return;
	}

	// Read packets of a media file to get stream information
	ret = avformat_find_stream_info(format_ctx, NULL);
	if (ret < 0) 
	{
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return;
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
	}
 
	if (video_stream_index == -1 || audio_stream_index == -1)
	{
		fprintf(stderr, "no video audio stream\n");
		return;
	}

	/*Fetch the context from decoder and coder from video*/
	pCodecCtx = format_ctx->streams[video_stream_index]->codec;
	
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

	/*
	*	read each frame using av_read_frame
	*	check the index of av
	*	write in file 
	*/

	/*avoid segmentation fault*/
	pkt = av_packet_alloc();
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

			/*pkt->size should not be zero, if it is, it should be break*/
			if (ret < 0 && !(pkt->size))
			{
				printf("decoder error......\n");
				return;
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
	avformat_close_input(&format_ctx);
}
int main(int argc, char **argv)
{	
	test_ffmpeg_rtmp_client();
	return 0;
}
