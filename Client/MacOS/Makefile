OBJS=main.o socket.o
CFLAGS=-I../Common/x264 -I../Common/ffmpeg -I../Common/Socket
FRAMEWORKS:= -framework Foundation -framework CoreVideo -framework CoreGraphics
LIBRARIES:= -L../Common/x264 -lx264.152 -L../Common/ffmpeg/libswscale -lswscale.5 -lobjc -lstdc++

screencast: $(OBJS)
	$(CC) -o $@ $(OBJS) $(FRAMEWORKS) $(LIBRARIES)

main.o: main.mm
	$(CC) -c -o $@ $(CFLAGS) $<

socket.o: ../Common/Socket/socket.cpp
	$(CXX) -c -o $@ $(CFLAGS) $<

install:
	cp ../Common/x264/libx264.*.dylib .
	cp ../Common/ffmpeg/libswscale/libswscale.*.dylib .
	cp ../Common/ffmpeg/libavutil/libavutil.*.dylib .
	strip screencast
	tar czf promys.tgz lib*.dylib screencast promys.sh

clean:
	-rm -f *.o *.dylib screencast
