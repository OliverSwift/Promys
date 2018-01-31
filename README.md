# Promys
_Project My Screen_

A screencasting system project based on a Pi 3. Nothing very new for such project except that it aims at being very user friendly. Easy setup for users on Windows, Mac and Linux.

User connects to Wifi, visits an embedded web page, downloads and starts client application. Almost as easy as Click&Share from BARCO but at a reasonable price (~40€).

Already made image will be available soon on the web site, so it should be very straightforward to setup a Pi3 out of the box and have an up and running system.

For the moment it is very basic but it can be extended with plenty of nifty features. Check out TODO/Ideas section for that and feel free to contribute.

## Background

## Server
This section describes what is needed to build and setup the server.

You'll need a Pi3 running `Raspian stretch` to compile the server executable.

## Client
You'll need native platforms running (or VMs). I used Ubuntu 16.04, Windows 7, MacOS High Sierra.
Minimum environment is required for compiling client binaries. You'll need bash, git, gcc (or clang) and nasm 2.13.
On Windows `cygwin64` is required with these specific packages. On Linux you'll probably have to get recent nasm package. On MacOS you'll need XCode and nasm.

It depends on x264 and ffmpeg package. It only uses libswscale/libavutil from ffmpeg, x264 is used for video compression.
For each platform you'll have to compile them this way:

In `Client/Common` directory you'll have to clone both projects.
```
# cd Client/Common
# git clone http://git.videolan.org/git/x264.git x264
# git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
```
Current promys clients have been compiled against these exact versions:
  x264: b00bcafe53a166b63a179a2f41470cd13b59f927
  ffmpeg: bfe397e4313c640e2f05c90a2ff1541f50524094
For versions matching reason I recommend to git checkout these commits.

For x264 you'll need nasm version 2.13 or superior.
```
# ./configure --disable-cli --enable-shared
# make
```
No installation needed. Leave libs where they are.

For ffmpeg:
```
# ./configure --disable-all --enable-swscale --enable-shared
# make
```

Once you have x264, swscale and avtuil libraries compiled you can proceed to each 

### Windows

Get to `Client/Windows` and type `make install`. This will generate a `promys.exe`file which is
a autoextractable executable with all need stuff in it. User just has to run it, no installer.
Client will start to search for a Promys device (server).

### Linux

Get to `Client/Linux` and type `make package`. This will generate a `promys.deb` file which is
intended for debian based distros. User will have to install it prior to using the client.
With any launcher, search for promys and start the application. It will start searching
promys device. Once found your desktop will be cast to the device.

### MacOS

Get to `Client/MacOS` and type `make`. This will generate a `promys.dmg` file which is
a software drive. User will have to download it, open it and start the application. GateKeeper
will certainly ask permission an application downloaded from the Internet.
The already built one is signed with my developer account so user won't have to allow its execution.
I haven't pushed it on the Mac Store though.

