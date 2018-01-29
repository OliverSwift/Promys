OBJS=main.o video.o socket.o splash.o discover.o
BIN=promys
LDFLAGS+=-lilclient -ljpeg

include Makefile.include

promys: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

install: promys
	mkdir -p /opt/promys
	cp promys /opt/promys
	cp Target/opt/promys/splash.jpg /opt/promys
