OBJS=main.o socket.o gui.o screencast.res
CXXFLAGS=-I../Common/x264 -I../Common/ffmpeg -I../Common/Socket

screencast: $(OBJS)
	$(CXX) -o $@ $(OBJS) -mwindows -L../Common/x264 -lx264 -L../Common/ffmpeg/libswscale -lswscale

socket.o: ../Common/Socket/socket.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

screencast.res: ScreenCast.rc
	windres ScreenCast.rc -o screencast.res -O coff

install: screencast promys.SED
	cp ../Common/ffmpeg/libavutil/cygavutil-56.dll .
	cp ../Common/ffmpeg/libswscale/cygswscale-5.dll .
	cp ../Common/x264/libx264-155.dll .
	cp /usr/bin/cyggcc_s-seh-1.dll .
	cp /usr/bin/cygstdc++-6.dll .
	cp /usr/bin/cygwin1.dll .
	strip screencast.exe
	iexpress /N /Q promys.SED

clean:
	-rm *.o screencast.exe
