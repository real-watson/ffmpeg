
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
	mkdir source
	cp pull.cpp source -fa
	tar czf source.tar.gz source
	g++ -std=c++11 pull.cpp -o pull $(LDFLAGS) $(CFLAGS)	
clean:
	rm pull
