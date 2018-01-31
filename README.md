# Promys
_Project My Screen_

A screencasting system project based on a Pi 3. Nothing very new for such project except that it aims at being very user friendly. Easy setup for users on Windows, Mac and Linux.

User connects to Wifi, visits an embedded web page, downloads and starts client application. Almost as easy as Click&Share from BARCO but at a reasonable price (~40€).

Already made image will be available soon on the web site, so it should be very straightforward to setup a Pi3 out of the box and have an up and running system.

For the moment it is very basic but it can be extended with plenty of nifty features. Check out TODO/Ideas section for that and feel free to contribute.

## Background

What this is all about...

_TBD_

## Server
This section describes what is needed to build and setup the server.

You'll need a Pi3 running `Raspian stretch` to compile the server executable.

_TBD_

## Client
The client part is the application to be run on a desktop so as to cast it to a Promys device connected to a TV set or projector.
Once launched it will start searching promys device over available networks. When a promys device is found your desktop will be cast to it.
A desktop PC or Mac can be directly connected to a Promys device using its Wifi access point or the same lan network they both are connected to. Usually, Wifi is used for guests and Lan for co-workers in a company. Lan connection is optional since it might be not allowed by company's security policy (default Promys device behaviour it to NAT Wifi trafic to Lan).

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
```
x264: b00bcafe53a166b63a179a2f41470cd13b59f927
ffmpeg: bfe397e4313c640e2f05c90a2ff1541f50524094
```
For versions matching reason I recommend to git checkout these commits.

For x264 you'll need nasm version 2.13 or superior.
```
# ./configure --disable-cli --enable-shared
# make
```

For ffmpeg:
```
# ./configure --disable-all --enable-swscale --enable-shared
# make
```

For both, no installation step needed. Leave libs where they are.

Once you have x264, swscale and avtuil libraries compiled you can proceed to each specific build stages below.

### Windows

Get to `Client/Windows` and type `make install`. This will generate a `promys.exe` file which is
a autoextractable executable with all needed stuff in it. User has just to run it, no installer, automatic clean-up.

### Linux

Get to `Client/Linux` and type `make package`. This will generate a `promys.deb` file which is
intended for debian based distros. User will have to install it prior to using the client.
With any launcher, search for promys and start the application.

### MacOS

Get to `Client/MacOS` and type `make`. This will generate a `promys.dmg` file which is
a software drive. User will have to download it, open it and start the application. GateKeeper
will certainly ask permission to run an application downloaded from the Internet.
The already built one is signed with my developer account so user won't have to allow its execution.
I haven't pushed it on the Mac Store though.

