/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Please checkout LICENSE file.
 */
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <xcb/xfixes.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "gui.h"
#include "h264.h"
#include "discover.h"
#include "socket.h"

static void
desk_properties(xcb_connection_t *connection,
	int screen_number,
	xcb_window_t *root,
	int16_t *x,
	int16_t *y,
	uint16_t *width,
	uint16_t *height) {
	xcb_screen_iterator_t iterator
	    = xcb_setup_roots_iterator(xcb_get_setup(connection));

	// First, we find the root window
	while(iterator.rem != 0
	    && screen_number != 0) {
	    screen_number -= 1;
	    xcb_screen_next(&iterator);
	}

	if(screen_number != 0) {
	    fprintf(stderr, "Cannot determine root window\n");
	    exit(EXIT_FAILURE);
	}

	*root = iterator.data->root;

	// We find randr resources associated to root
	xcb_randr_get_screen_resources_reply_t *screen_resources
	    = xcb_randr_get_screen_resources_reply(connection,
		xcb_randr_get_screen_resources(connection,
		    *root),
		NULL);
        if(screen_resources == NULL) {
	    fprintf(stderr, "Unable to get screen resources\n");
	    exit(EXIT_FAILURE);
        }

	// Listing its crtcs, we take the first viewport for display
	xcb_randr_crtc_t *crtcs
	    = xcb_randr_get_screen_resources_crtcs(screen_resources);
	xcb_randr_get_crtc_info_reply_t *info
	    = xcb_randr_get_crtc_info_reply(connection,
		xcb_randr_get_crtc_info(connection,
		    crtcs[0], XCB_TIME_CURRENT_TIME),
		NULL);

	*x = info->x;
	*y = info->y;
	*width = info->width;
	*height = info->height;

	free(info);
	free(screen_resources);
}

static inline uint8_t
color_select(uint8_t dst, uint8_t src_pre, uint8_t alpha) {
	uint16_t val;

	val = (dst * (~alpha & 255)) + src_pre * 255;

	return val >> 8;
}

static void
draw_cursor(xcb_connection_t *connection,
	xcb_window_t root,
	uint8_t *data,
	uint16_t height,
	size_t stride) {
	xcb_query_pointer_reply_t *query_pointer
	    = xcb_query_pointer_reply(connection,
		xcb_query_pointer(connection, root),
		NULL);

	if(query_pointer != NULL
	    && query_pointer->same_screen == 1) {
	    xcb_xfixes_get_cursor_image_reply_t *cursor_image
		= xcb_xfixes_get_cursor_image_reply(connection,
		    xcb_xfixes_get_cursor_image(connection),
		    NULL);

	    if(cursor_image != NULL) {
		uint8_t *dst, *low, *high;
		uint32_t *src;

		dst = data
			+ ((cursor_image->y - cursor_image->yhot) * stride)
			+ (cursor_image->x - cursor_image->xhot) * 4;
		src = xcb_xfixes_get_cursor_image_cursor_image(cursor_image);

		low = data;
		high = data + (height - 1) * stride;

		int l,c;

		for(l=0; l < cursor_image->height; l++) {
		    for(c=0; c < cursor_image->width*4; c+=4) {
			if(dst >= low && dst < high) {
			    dst[0+c] = color_select(dst[0+c], *src, (*src)>>24);
			    dst[1+c] = color_select(dst[1+c], (*src)>>8, (*src)>>24);
			    dst[2+c] = color_select(dst[2+c], (*src)>>16, (*src)>>24);
			}
			src++;
		    }
		    dst += stride;
		}

		free(cursor_image);
	    }
	}

	free(query_pointer);
}

int
main(int argc, char **argv) {
	struct timeval start,stop;
	const char *cast_server = argv[1];
	int cast_port = 9000;
	volatile int go;

	// Initialize GUI stuff in separate thread
	gui_init((int *)&go);

	// Discover a Promys device, unless specified in arguments
	if (argv[1] == NULL) {
	    showMessage("Searching for PROMYS device");
	    cast_server = promys_discover(&cast_port);
	}

	printf("Found at %s:%d\n", cast_server, cast_port);

	// Update UI and iconify window
	showMessage("PROMYS device found");
	hideWindow();

	socket_connect(cast_server, cast_port);

	gettimeofday(&start, NULL);

	// Connection to X11
        xcb_connection_t *connection;
        int screen_number;

        connection = xcb_connect(NULL, &screen_number);
        if(xcb_connection_has_error(connection) > 0) {
	    fprintf(stderr, "Cannot connect to X11\n");
	    exit(EXIT_FAILURE);
        }

        // XFixes query version, required by protocol or "undefined behavior"
	xcb_xfixes_query_version_reply_t *version
	    = xcb_xfixes_query_version_reply(connection,
		xcb_xfixes_query_version(connection,
		    XCB_XFIXES_MAJOR_VERSION,
		    XCB_XFIXES_MINOR_VERSION),
		NULL);

        // Determine desktop geometry
        xcb_window_t root;
	int16_t x, y;
	uint16_t width, height;

	desk_properties(connection, screen_number,
	    &root, &x, &y, &width, &height);

	// Make a first capture to figure out stride
        xcb_get_image_cookie_t image_cookie
	    = xcb_get_image(connection, XCB_IMAGE_FORMAT_Z_PIXMAP,
		root, x, y, width, height, ~0);
        xcb_get_image_reply_t *image
	    = xcb_get_image_reply(connection, image_cookie, NULL);
        if(image == NULL) {
	    fprintf(stderr, "Unable to get root image\n");
	    exit(EXIT_FAILURE);
        }
	size_t stride = xcb_get_image_data_length(image) / height;

	// Init encoder with correct sizes
	h264_init(width, height, stride);

	// Inform we're broadcasting thru UI
	showMessage("Broadcasting...");

	// Proceed til user ends it up
	while(go) {
	    unsigned char *packet;
	    size_t packet_size;
	    uint8_t *data
		= xcb_get_image_data(image);
	    draw_cursor(connection, root, data, height, stride);

	    packet = h264_encode(data, &packet_size);

	    free(image);

	    if (packet) {
		if (socket_send(packet, packet_size) < 0) break; // Peer likely has disconnected
	    }

	    // Request next capture
	    image_cookie = xcb_get_image(connection, XCB_IMAGE_FORMAT_Z_PIXMAP,
		root, x, y, width, height, ~0);

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
	    image = xcb_get_image_reply(connection, image_cookie, NULL);
	}

	h264_close();

	socket_close();

	free(image);
	free(version);
	xcb_disconnect(connection);
}
