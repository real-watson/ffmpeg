#This is Makefile for both pull.cpp and push.cpp
#Modified by watson.
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
	g++ -std=c++11 pull.cpp -o pull $(LDFLAGS) $(CFLAGS)	
clean:
	rm pull
