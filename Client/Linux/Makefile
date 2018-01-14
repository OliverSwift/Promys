OBJS=main.o socket.o
CXXFLAGS=-I../Common/x264 -I../Common/ffmpeg -I../Common/Socket
LIBRARIES:= -L../Common/x264 -lx264 -L../Common/ffmpeg/libswscale -lswscale -L../Common/ffmpeg/libavutil -lavutil -lstdc++ -lX11

screencast: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(FRAMEWORKS) $(LIBRARIES)

socket.o: ../Common/Socket/socket.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

install:
	cp ../Common/x264/libx264.so.* .
	cp ../Common/ffmpeg/libswscale/libswscale.so.* .
	cp ../Common/ffmpeg/libavutil/libavutil.so.* .
	strip screencast lib*.so.*
	tar czf promys.tgz lib*.so.* screencast promys.sh

clean:
	-rm -f *.o lib*.so.* screencast
