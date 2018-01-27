OBJS=main.o video.o socket.o splash.o discover.o
BIN=promys
LDFLAGS+=-lilclient -ljpeg

include Makefile.include

promys: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)
