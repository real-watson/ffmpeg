
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
	g++ -std=c++11 push.cpp -o pushvideo $(LDFLAGS) $(CFLAGS)	
clean:
	rm pushvideo
