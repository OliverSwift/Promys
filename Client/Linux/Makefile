OBJS=main.o socket.o gui.o discover.o h264.o
DEBUG=-O
CFLAGS=$(DEBUG) -Wall -Werror -I../Common/x264 -I../Common/ffmpeg -I../Common/Socket
LIBRARIES:= -L../Common/x264 -lx264 -L../Common/ffmpeg/libswscale -lswscale -L../Common/ffmpeg/libavutil -lavutil -lX11 -lxcb -lxcb-randr -lxcb-xfixes -lpthread

screencast: $(OBJS)
	$(CC) $(DEBUG) -o $@ $(OBJS) $(FRAMEWORKS) -Wl,-rpath=./:/opt/promys $(LIBRARIES)

socket.o: ../Common/Socket/socket.c
	$(CC) -c -o $@ $(CFLAGS) $<

install: screencast
	cp ../Common/x264/libx264.so.* .
	cp ../Common/ffmpeg/libswscale/libswscale.so.* .
	cp ../Common/ffmpeg/libavutil/libavutil.so.* .
	strip screencast lib*.so.*
	tar czf promys.tgz screencast lib*.so.*

package: screencast
	mkdir -p dist/opt/promys
	cp ../Common/x264/libx264.so.* dist/opt/promys
	cp ../Common/ffmpeg/libswscale/libswscale.so.* dist/opt/promys
	cp ../Common/ffmpeg/libavutil/libavutil.so.* dist/opt/promys
	strip screencast
	cp screencast dist/opt/promys
	cp promys.png dist/opt/promys
	mkdir -p dist/DEBIAN
	cp deb-control dist/DEBIAN/control
	echo Installed-Size: `du -sk dist | awk -- '{print $$1}'` >> dist/DEBIAN/control
	mkdir -p dist/usr/share/applications
	cp promys.desktop dist/usr/share/applications
	dpkg-deb -b dist promys.deb
	rm -rf dist

clean:
	-rm -f *.o lib*.so.* screencast
