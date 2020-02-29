
CFLAGS = -Wall
LDFLAGS = -I /usr/local/ffmpeg/include \
	-L /usr/local/ffmpeg/lib \
	-lavdevice \
	-lavformat \
	-lavcodec \
	-lavutil \
	-lavfilter \
	-lswresample \
	-lswscale \
	-larcsoft_face \
	-larcsoft_face_engine 

all:
	gcc pull.c face.c file.c jpeg.c avinit.c video.c -o test $(LDFLAGS) $(CFLAGS)	
clean:
	rm test
