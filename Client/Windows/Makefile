OBJS=main.o socket.o gui.o discover.o h264.o screencast.res
CFLAGS=-O -I../Common/x264 -I../Common/ffmpeg -I../Common/Socket

screencast.exe: $(OBJS)
	$(CC) -o $@ $(OBJS) -mwindows -L../Common/x264 -lx264 -L../Common/ffmpeg/libswscale -lswscale -lWtsapi32

socket.o: ../Common/Socket/socket.c
	$(CC) -c -o $@ $(CFLAGS) $<

screencast.res: ScreenCast.rc
	windres ScreenCast.rc -o screencast.res -O coff

ScreenCast.rc: promys.ico

install: screencast.exe promys.SED
	cp ../Common/ffmpeg/libavutil/cygavutil-56.dll .
	cp ../Common/ffmpeg/libswscale/cygswscale-5.dll .
	cp ../Common/x264/libx264-155.dll .
	cp /usr/bin/cygwin1.dll .
	strip screencast.exe *.dll
	iexpress /N /Q promys.SED

clean:
	-rm *.o screencast.exe *.res promys.exe
