#ifndef __COMMON_H__

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h> 
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


extern void generate_file_name(int index,char *filename);
extern int save_jpeg(AVFrame *pFrame, char *out_name);
extern void init_register_network();
extern void test_ffmpeg_rtmp_client();

#define __COMMON_H__
#endif
