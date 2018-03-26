OBJS=main.o video.o socket.o splash.o discover.o
BIN=promys
LDFLAGS+=-lilclient -ljpeg
CFLAGS+=-O3
INCLUDES+=$(shell pkg-config --cflags freetype2)
LDFLAGS+=$(shell pkg-config --libs freetype2)

include Makefile.include

promys: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

install: promys
	strip promys
	mkdir -p /opt/promys
	rm -f /opt/promys/promys
	cp promys /opt/promys
