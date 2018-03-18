/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Please checkout LICENSE file.
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gui.h"
#include "h264.h"
#include "discover.h"
#include "socket.h"

int
main(int argc, char **argv) {
	struct timeval start,stop;
	const char *cast_server = argv[1];
	int cast_port = 9000;
	int go;

	// Initialize GUI stuff in separate thread
	gui_init(&go);

	// Discover a Promys device, unless specified in arguments
	if (argv[1] == NULL) {
	    showMessage("Searching for PROMYS device");
	    cast_server = promys_discover(&cast_port);
	}

	printf("Found at %s:%d\n", cast_server, cast_port);

	// Update UI and iconfy window
	showMessage("PROMYS device found");
	hideWindow();

	socket_connect(cast_server, cast_port);

	gettimeofday(&start, NULL);

	// Determine desktop size
	size_t width, height;

	Display *dpy;
	int screen;
	XImage *image;
	Window root;

	dpy = XOpenDisplay(getenv("DISPLAY"));
	if (dpy == NULL) {
	    printf("Can't open display.\n");
	    exit(1);
	}
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	width = DisplayWidth (dpy, screen);
	height = DisplayHeight (dpy, screen);

	// If multiple monitors, use Xrandr to figure out the central left moft one
	XRRMonitorInfo *monitors;

	int nbMonitors = 0;

	monitors = XRRGetMonitors(dpy, root, True, &nbMonitors);

	if (nbMonitors) {
	    width =  monitors[0].width;
	    height = monitors[0].height;
	}

	// Make a first capture to figure out stride
	image = XGetImage(dpy, root, 0, 0, width, height, AllPlanes, ZPixmap);

	// Init encoder with correct sizes
	h264_init(width, height, image->bytes_per_line);

	// Inform we're broadcasting thru UI
	showMessage("Broadcasting...");

	// Proceed til user ends it up
	while(go) {
	    unsigned char *packet;
	    size_t packet_size;

	    const unsigned char *data = (const unsigned char *)image->data;

	    packet = h264_encode(data, &packet_size);

	    if (packet_size) {
		if (socket_send(packet, packet_size) < 0) break; // Peer likely has disconnected
	    }

	    XDestroyImage(image);

	    // We're trying to maintain a 20i/s pace
	    // taking into account all possible delays (capture, encoding and network)

	    gettimeofday(&stop, NULL);

#define DELAY_US 50000

	    long delay;

	    delay = (stop.tv_sec - start.tv_sec)*1000000;
	    delay += (stop.tv_usec - start.tv_usec);
	    if (delay < DELAY_US) {
		usleep(DELAY_US - delay);
	    }

	    gettimeofday(&start, NULL);

	    // Capture a new image
	    image = XGetImage(dpy, root, 0, 0, width, height, AllPlanes, ZPixmap);
	}

	h264_close();

	socket_close();

	XCloseDisplay(dpy);
}
