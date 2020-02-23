
CFLAGS = -Wall
LDFLAGS = -I /usr/local/ffmpeg/include \
	-L /usr/local/ffmpeg/lib \
	-lavdevice \
	-lavformat \
	-lavcodec \
	-lavutil \
	-lavfilter \
	-lswresample \
	-lswscale
all:
	gcc pull.c file.c jpeg.c avinit.c video.c -o test $(LDFLAGS) $(CFLAGS)	
clean:
	rm test
