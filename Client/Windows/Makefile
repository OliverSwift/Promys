OBJS=main.o socket.o gui.o screencast.res
CXXFLAGS=-I../Common/x264 -I../Common/ffmpeg

screencast: $(OBJS)
	$(CXX) -o $@ $(OBJS) -mwindows -L../Common/x264 -lx264 -L../Common/ffmpeg/libswscale -lswscale

socket.o: ../Common/Socket/socket.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

screencast.res: ScreenCast.rc
	windres ScreenCast.rc -o screencast.res -O coff

install: screencast.exe promys.SED
	iexpress /N /Q promys.SED
