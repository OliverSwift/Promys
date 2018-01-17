OBJS=main.o socket.o gui.o
CXXFLAGS=-I../Common/x264 -I../Common/ffmpeg -I../Common/Socket
LIBRARIES:= -L../Common/x264 -lx264 -L../Common/ffmpeg/libswscale -lswscale -L../Common/ffmpeg/libavutil -lavutil -lstdc++ -lX11 -lpthread

screencast: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(FRAMEWORKS) -Wl,-rpath=./:/opt/promys $(LIBRARIES)

socket.o: ../Common/Socket/socket.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

install:
	cp ../Common/x264/libx264.so.* .
	cp ../Common/ffmpeg/libswscale/libswscale.so.* .
	cp ../Common/ffmpeg/libavutil/libavutil.so.* .
	strip screencast lib*.so.*
	tar czf promys.tgz screencast lib*.so.*

package:
	mkdir -p dist/opt/promys
	cp ../Common/x264/libx264.so.* dist/opt/promys
	cp ../Common/ffmpeg/libswscale/libswscale.so.* dist/opt/promys
	cp ../Common/ffmpeg/libavutil/libavutil.so.* dist/opt/promys
	strip screencast
	cp screencast dist/opt/promys
	cp promys.png dist/opt/promys
	mkdir -p dist/DEBIAN
	cp deb-control dist/DEBIAN/control
	mkdir -p dist/usr/share/applications
	cp promys.desktop dist/usr/share/applications
	dpkg-deb -b dist promys.deb

clean:
	-rm -f *.o lib*.so.* screencast
