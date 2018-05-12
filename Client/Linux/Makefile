OBJS=main.o socket.o gui.o discover.o h264.o
DEBUG=-O3
CFLAGS=$(DEBUG) -Wall -Werror -I../Common/x264 -I../Common/ffmpeg -I../Common/Socket
LIBRARIES:= -L../Common/x264 -lx264 -L../Common/ffmpeg/libswscale -lswscale -L../Common/ffmpeg/libavutil -lavutil -lX11 -lxcb -lxcb-randr -lxcb-xfixes -lpthread

screencast: $(OBJS)
	cp ../Common/x264/libx264.so.* .
	cp ../Common/ffmpeg/libswscale/libswscale.so.* .
	cp ../Common/ffmpeg/libavutil/libavutil.so.* .
	$(CC) $(DEBUG) -o $@ $(OBJS) $(FRAMEWORKS) -Wl,-rpath=./:/opt/promys $(LIBRARIES)

socket.o: ../Common/Socket/socket.c
	$(CC) -c -o $@ $(CFLAGS) $<

tgz: screencast
	strip screencast lib*.so.*
	tar czf promys.tgz screencast lib*.so.*

deb: dist
	mkdir -p dist/DEBIAN
	cp deb-control dist/DEBIAN/control
	echo Installed-Size: `du -sk dist | awk -- '{print $$1}'` >> dist/DEBIAN/control
	dpkg-deb -b dist promys.deb
	rm -rf dist

rpm: dist
	rpmbuild --quiet -bb --buildroot ${PWD}/dist promys.spec
	cp ${HOME}/rpmbuild/RPMS/`uname -m`/promy*.rpm promys.rpm

dist: screencast FRC
	mkdir -p dist/opt/promys
	cp ../Common/x264/libx264.so.* dist/opt/promys
	cp ../Common/ffmpeg/libswscale/libswscale.so.* dist/opt/promys
	cp ../Common/ffmpeg/libavutil/libavutil.so.* dist/opt/promys
	cp screencast dist/opt/promys
	strip dist/opt/promys/screencast dist/opt/promys/lib*
	cp promys.png dist/opt/promys
	mkdir -p dist/usr/share/applications
	cp promys.desktop dist/usr/share/applications

FRC:

clean:
	-rm -f *.o lib*.so.* screencast
	-rm -rf dist
