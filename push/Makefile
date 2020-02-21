
CFLAGS = -Wall -O -g
LDFLAGS = -I /usr/local/ffmpeg/include \
	-L /usr/local/ffmpeg/lib \
	-lavdevice \
	-lavformat \
	-lavcodec \
	-lavutil \
	-lavfilter \
	-lswresample \
	-lswscale
target:
	gcc push.c -o test $(LDFLAGS) $(CFLAGS)	
clean:
	rm test
