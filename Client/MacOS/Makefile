OBJS=main.o socket.o
CFLAGS=-I../Common/x264 -I../Common/ffmpeg -I../Common/Socket
FRAMEWORKS:= -framework Foundation -framework CoreVideo -framework CoreGraphics
LIBRARIES:= -Wl,-rpath,./ -L../Common/x264 -lx264.152 -L../Common/ffmpeg/libswscale -lswscale.5 -lobjc -lstdc++

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
	install_name_tool -change /usr/local/lib/libavutil.56.dylib ./libavutil.56.dylib libswscale.5.dylib
	install_name_tool -change /usr/local/lib/libswscale.5.dylib ./libswscale.5.dylib screencast
	install_name_tool -change /usr/local/lib/libx264.152.dylib ./libx264.152.dylib screencast
	strip screencast
	tar czf promys.tgz lib*.dylib screencast promys.sh

clean:
	-rm -f *.o *.dylib screencast
