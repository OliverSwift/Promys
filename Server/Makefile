OBJS=main.o video.o socket.o splash.o broadcast.o
BIN=promys
LDFLAGS+=-lilclient

include Makefile.include

promys: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)
