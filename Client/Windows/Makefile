OBJS=main.o socket.o gui.o discover.o screencast.res
CFLAGS=-I../Common/x264 -I../Common/ffmpeg -I../Common/Socket

screencast.exe: $(OBJS)
	$(CC) -o $@ $(OBJS) -mwindows -L../Common/x264 -lx264 -L../Common/ffmpeg/libswscale -lswscale

socket.o: ../Common/Socket/socket.c
	$(CC) -c -o $@ $(CFLAGS) $<

screencast.res: ScreenCast.rc
	windres ScreenCast.rc -o screencast.res -O coff

install: screencast.exe promys.SED
	cp ../Common/ffmpeg/libavutil/cygavutil-56.dll .
	cp ../Common/ffmpeg/libswscale/cygswscale-5.dll .
	cp ../Common/x264/libx264-155.dll .
	cp /usr/bin/cygwin1.dll .
	strip screencast.exe
	iexpress /N /Q promys.SED

clean:
	-rm *.o screencast.exe promys.exe
