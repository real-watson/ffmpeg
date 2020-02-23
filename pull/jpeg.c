#include "common.h"
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


