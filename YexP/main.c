/*
	yuv�ļ�ת��Ϊjpg���裺
	1.�����ʼ��
	2.��������ʼ��Ҫ�������Դ
	3.�������
	4.��������������ĳ�ʼ������ֵ
	5.�򿪱������
	6.����������
	7.������ר�����ݲ�����ͷд�����ý���ļ�
	8.Ϊpacket����ָ����С���ڴ�
	9.��ȡ�ļ�
	10.��Ƶ����
	11.�����ݰ�д�����ý���ļ�
	12.�ر���Դ
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
AVFormatContext *ifmt_ctx;
AVOutputFormat *ofmt;//��ʾ����ļ�������ʽ
AVCodecContext *mCodec_ctx;
AVCodec *mCodec;
AVFrame *frame;
AVPacket packet;
AVStream *video_st;
int in_w = 640, in_h = 360;
int got_picture = 0;
FILE *in_file;

int frame_count = 0;
char str[10];
int main() {
	av_register_all();
	in_file = fopen("E:/asd.yuv", "rb+");
	ifmt_ctx = avformat_alloc_context();
	// ��������ļ���ʽ 
	ofmt = av_guess_format("mjpeg", NULL, NULL);
	ifmt_ctx->oformat = ofmt;

	//��������ʼ����Դ��AvioContext��AVIOContext:��������������ݵĽṹ��
	

	/*if (avio_open(&ifmt_ctx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
		printf("Couldn't open output file.");
		return 0;
	}*/
	//����һ����stream
	video_st = avformat_new_stream(ifmt_ctx, 0);
	if (video_st == NULL) {
		return 0;
	}
	//���ø�stream����Ϣ
	mCodec_ctx = video_st->codec;
	mCodec_ctx->codec_id = ofmt->video_codec;
	mCodec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	mCodec_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
	mCodec_ctx->width = in_w;
	mCodec_ctx->height = in_h;
	mCodec_ctx->time_base.num = 1;
	mCodec_ctx->time_base.den = 25;

	printf("width:%d\n", mCodec_ctx->width);
	printf("height:%d\n", mCodec_ctx->height);
	printf("time_base.num:%d\n", mCodec_ctx->time_base.num);
	printf("time_base.den:%d\n", mCodec_ctx->time_base.den);
	
	// ���ҽ�����
	mCodec = avcodec_find_encoder(mCodec_ctx->codec_id);
	printf("name:%s\n", mCodec->name);
	if (!mCodec) {
		return 0;
	}
	// ����pCodecCtx�Ľ�����ΪpCodec
	if (avcodec_open2(mCodec_ctx, mCodec, NULL) < 0) {
		return 0;
	}
	frame = av_frame_alloc();
	//avpicture_get_size():����ͼ���С����
	
	while (1) {
		char out_file[] = "E:/pic/";
		itoa(frame_count, str, 10);
		strcat(out_file, str);
		strcat(out_file, ".jpg");
		printf("out_file:%s\n", out_file);
		if (avio_open(&ifmt_ctx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
			printf("Couldn't open output file.");
			return 0;
		}
		uint8_t *buffer = (uint8_t *)av_malloc(avpicture_get_size(mCodec_ctx->pix_fmt, mCodec_ctx->width, mCodec_ctx->height));
		//���ͼ��
		avpicture_fill((AVPicture *)frame, buffer, mCodec_ctx->pix_fmt, mCodec_ctx->width, mCodec_ctx->height);

		//Write Header
		avformat_write_header(ifmt_ctx, NULL);
		//����packet�ռ� AVPacket���洢ѹ���������������Ϣ�Ľṹ��
		av_new_packet(&packet, mCodec_ctx->width*mCodec_ctx->height * 3);

		if (fread(buffer, 1, mCodec_ctx->width*mCodec_ctx->height * 3 / 2, in_file) <= 0) {
			printf("Could not read input file.");
			if (video_st) {
				avcodec_close(video_st->codec);
				av_free(frame);
				av_free(buffer);
			}
			return 0;
		}
		frame->data[0] = buffer;
		frame->data[1] = buffer + mCodec_ctx->width*mCodec_ctx->height;
		frame->data[2] = buffer + mCodec_ctx->width*mCodec_ctx->height * 5 / 4;
		if (avcodec_encode_video2(mCodec_ctx, &packet, frame, &got_picture) < 0) {
			printf("Encode Failed!\n");
			return 0;
		}
		if (got_picture == 1) {
			packet.stream_index = video_st->index;
			//��packetд�����ý���ļ�
			av_write_frame(ifmt_ctx, &packet);
		}
		av_free_packet(&packet);
		av_write_trailer(ifmt_ctx);
		printf("Encode Successful.\n");
		frame_count++;
		printf("��%d֡\n",frame_count);
		
	}

	avio_close(ifmt_ctx->pb);
	avformat_free_context(ifmt_ctx);

	fclose(in_file);
	return 0;
}