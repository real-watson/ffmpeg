#include <stdio.h>
#include <string.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include "libavdevice/avdevice.h"

#define PUSH_PATH "rtmp://120.77.214.213:1935/live_video/video"
#define AVIN_PATH "hello.mp4"

static char *push_proto_set(char *path)
{
	char *format_name = NULL;

	if (strstr(path,"rtmp://") != NULL)
	{
		format_name = "flv";
	}
	else if (strstr(path,"udp://")  != NULL)
	{
		format_name = "mpegts";
	}
	else
	{
		format_name = NULL;
	}
	return (format_name);
}

int main(int argc, char **argv)
{
	double duration;
	int ret,i;
	int videoIndex=-1;
	int audioIndex=-1;
	int video_frame_count=0;
	int audio_frame_count = 0;
	int video_frame_size = 0;
	int audio_frame_size = 0;
	char fm_name[128] = "";
	int64_t start_time;

	AVFormatContext *pInFmtContext = NULL;
	AVStream *in_stream;
	AVCodec *pInCodec;
	AVPacket *in_packet;//AVPacket
	AVFormatContext * pOutFmtContext;
	AVRational frame_rate;
	AVStream * out_stream;

	/*init the device and register*/
	avdevice_register_all();
	av_register_all();

	/*call for network function*/
	avformat_network_init();

	//input_fmt = av_find_input_format (input_format_name);
	/*open a file about video*/
	if(avformat_open_input(&pInFmtContext,AVIN_PATH,NULL,NULL)<0)
	{
		printf("avformat_open_input failed\n");
		return -1;
	}

	if(avformat_find_stream_info(pInFmtContext,NULL)<0)
	{
		printf("avformat_stream_info failed\n");
		return -1;
	}

	av_dump_format(pInFmtContext,0,AVIN_PATH,0);

	/*check the protocol :rtmp:flv or udp:mpegts */
	strcpy(fm_name,push_proto_set(PUSH_PATH));

	/*out_file flv for rtmp format*/
	ret=avformat_alloc_output_context2(&pOutFmtContext,NULL,fm_name,PUSH_PATH);
	if(ret<0)
	{
		printf("avformat_output_context2 failed\n");
		return -1;
	}
/*search the index of video*/
	for(i=0;i<pInFmtContext->nb_streams;i++)
	{

		in_stream=pInFmtContext->streams[i];
	/*check the types of stream*/
		if(in_stream->codecpar->codec_type==AVMEDIA_TYPE_AUDIO)
		{
			audioIndex=i;
		}
		/*av_guess_frame_rate*/
		if(in_stream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
		{
			videoIndex=i;
			frame_rate=av_guess_frame_rate(pInFmtContext,in_stream,NULL);
			printf("video: frame_rate:%d/%d\n", frame_rate.num, frame_rate.den);

			printf("video: frame_rate:%d/%d\n", frame_rate.den, frame_rate.num);
			duration=av_q2d((AVRational){frame_rate.den,frame_rate.num});

		}

		//find decoder
		pInCodec=avcodec_find_decoder(in_stream->codecpar->codec_id);
		printf("%x, %d\n", pInCodec, in_stream->codecpar->codec_id);

		out_stream=avformat_new_stream(pOutFmtContext,pInCodec);
		if(out_stream==NULL)
		{
			printf("avformat_new_stream failed\n");
		}	

		ret=avcodec_parameters_copy(out_stream->codecpar,in_stream->codecpar);
			if(ret<0)
		{

			printf("avformat_parameters_copy failed\n");
		}

		out_stream->codecpar->codec_tag=0;
	
		if( pOutFmtContext->oformat->flags & AVFMT_GLOBALHEADER)
		{
					out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}

	av_dump_format(pOutFmtContext,0,PUSH_PATH,1);
/*
out_file -> rtmp address
AVIO_FLAG_WRITE -> WRITE
*/
	ret=avio_open(&pOutFmtContext->pb,PUSH_PATH,AVIO_FLAG_WRITE);
	if(ret<0)
	{
		printf("avio_open failed:%d\n", ret);
		return -1;
	}

	start_time=av_gettime();
	printf("The start time is %ld\n",start_time);
	ret=avformat_write_header(pOutFmtContext,NULL);
	in_packet=av_packet_alloc();
	 
	while (1) 
	{
		/*read each frame and set set pIn* to in_packet*/
		ret=av_read_frame(pInFmtContext,in_packet);
		/*each frame should be save as image*/

		printf("The av_read_frame is %d\n");
		if(ret<0)
		{
					printf("read frame end\n");
					break;
		}

		in_stream=pInFmtContext->streams[in_packet->stream_index];

		if(in_packet->stream_index==videoIndex)
		{
			video_frame_size+=in_packet->size;
			printf("recv %5d video frame %5d-%5d\n", ++video_frame_count, in_packet->size, video_frame_size);
		}

		if(in_packet->stream_index==audioIndex)
		{
			audio_frame_size += in_packet->size;
			printf("recv %5d audio frame %5d-%5d\n", ++audio_frame_count, in_packet->size, audio_frame_size);
		}

		int codec_type=in_stream->codecpar->codec_type;
		//add check for rtmp or udp
		/*check the types of stream*/
		if(codec_type==AVMEDIA_TYPE_VIDEO)
		{

			AVRational dst_time_base={1,AV_TIME_BASE};
			int64_t pts_time=av_rescale_q(in_packet->pts,in_stream->time_base,dst_time_base);
			int64_t now_time=av_gettime()-start_time;
			//delay for pushing video
			if(pts_time>now_time)
			{
				av_usleep(pts_time-now_time);
			}
		} 
		/*
		if(codec_type==AVMEDIA_TYPE_AUDIO)
		{
			printf("The stream is about audio\n");
			AVRational dst_time_base={1,AV_TIME_BASE};
			int64_t pts_time=av_rescale_q(in_packet->pts,in_stream->time_base,dst_time_base);
			int64_t now_time=av_gettime()-start_time;
			if(pts_time>now_time)
			{
				av_usleep(pts_time-now_time);
			}
		}
		*/
		out_stream=pOutFmtContext->streams[in_packet->stream_index];
		av_packet_rescale_ts(in_packet,in_stream->time_base, out_stream->time_base);
		in_packet->pos = -1;

		/*push to the server*/
		ret=av_interleaved_write_frame(pOutFmtContext,in_packet);
		//ret=av_write_frame(pOutFmtContext,in_packet);
		if(ret<0)
		{
			printf("av_interleaved_write_frame failed\n");
			break;
		}
		av_packet_unref(in_packet);//for next frame_read
	}
	/*close and free*/
	av_write_trailer(pOutFmtContext);
	av_packet_free(&in_packet);
	avformat_close_input(&pInFmtContext);
	avio_close( pOutFmtContext->pb);
	avformat_free_context(pOutFmtContext);
	return 0;
}








