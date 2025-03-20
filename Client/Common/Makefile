all: submodules
	(cd x264; ./configure --disable-cli --enable-shared; make -j; ln -sf libx264.so.155 libx264.so)
	(cd ffmpeg; ./configure --disable-all --enable-swscale --enable-shared; make -j)

submodules:
	git submodule init
	git submodule update
