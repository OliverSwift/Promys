#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libswscale/swscale.h>
#include "gui.h"
#include "discover.h"
#include <x264.h>
#include "socket.h"

#undef FILE_DUMP

int
main(int argc, char **argv) {
	struct timeval start,stop;
#ifdef FILE_DUMP
	FILE *out = fopen("out.h264","wb");
#endif
	const char *cast_server = argv[1];
	int cast_port = 9000;
	int go;

	gui_init(&go);

	if (argv[1] == NULL) {
	    showMessage("Searching for PROMYS device");
	    cast_server = promys_discover(&cast_port);
	}

	printf("Found at %s:%d\n", cast_server, cast_port);

	showMessage("PROMYS device found");
	hideWindow();

	socket_connect(cast_server, cast_port);

	gettimeofday(&start, NULL);

	size_t width, height;
	size_t out_width, out_height;

	Display *dpy;
	int screen;
	XImage *image;
	Window root;

	dpy = XOpenDisplay(getenv("DISPLAY"));
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	width = DisplayWidth (dpy, screen);
	height = DisplayHeight (dpy, screen);

	image = XGetImage(dpy, root, 0, 0, width, height, AllPlanes, ZPixmap);

	out_width = 1920;
	out_height = 1080;

	int linesize = image->bytes_per_line;

	x264_param_t param;
	x264_picture_t pic;
	x264_picture_t pic_out;
	x264_t *h;
	int i_frame = 0;
	int i_frame_size;
	x264_nal_t *nal;
	int i_nal;

	if( x264_param_default_preset( &param, "ultrafast", "zerolatency" ) < 0 )
	    exit(1);

	param.i_csp = X264_CSP_I420;
	param.i_width  = out_width;
	param.i_height = out_height;
	param.b_vfr_input = 0;
	param.b_repeat_headers = 1;
	param.b_annexb = 1;

	x264_param_apply_fastfirstpass(&param);

	/* Apply profile restrictions. */
	if( x264_param_apply_profile( &param, "main" ) < 0 )
	    exit(2);

	if( x264_picture_alloc( &pic, param.i_csp, param.i_width, param.i_height ) < 0 )
	    exit(2);

	h = x264_encoder_open( &param );

	struct SwsContext *swsCtxt;

	swsCtxt = sws_getContext(width, height, AV_PIX_FMT_BGRA,
	                         param.i_width, param.i_height, AV_PIX_FMT_YUV420P,
				 SWS_FAST_BILINEAR, NULL, NULL, NULL);

	int i=0;

    showMessage("Broadcasting...");

	while(go) {
	    const unsigned char *data = (const unsigned char *)image->data;

	    sws_scale(swsCtxt,
	              &data, &linesize,
		      0, height,
		      pic.img.plane,  pic.img.i_stride);

	    XDestroyImage(image);

	    pic.i_pts = i++;
	    i_frame_size = x264_encoder_encode( h, &nal, &i_nal, &pic, &pic_out );
	    if( i_frame_size < 0 )
		break;
	    else if( i_frame_size )
	    {
#ifdef FILE_DUMP
		fwrite(nal->p_payload, 1, i_frame_size, out);
#else
		if (socket_send(nal->p_payload, i_frame_size) < 0) break;
#endif
	    }

	    gettimeofday(&stop, NULL);

	    long delay;

	    delay = (stop.tv_sec - start.tv_sec)*1000000;
	    delay += (stop.tv_usec - start.tv_usec);
	    if (delay < 40000) {
		usleep(40000 - delay);
	    }

	    gettimeofday(&start, NULL);
	    // Capture a new image
	    image = XGetImage(dpy, root, 0, 0, width, height, AllPlanes, ZPixmap);
	}

	/* Flush delayed frames */
	while( x264_encoder_delayed_frames( h ) )
	{
	    i_frame_size = x264_encoder_encode( h, &nal, &i_nal, NULL, &pic_out );
	    if( i_frame_size < 0 )
		break;
	    else if( i_frame_size )
	    {
#ifdef FILE_DUMP
		fwrite(nal->p_payload, 1, i_frame_size, out);
#else
		if (socket_send(nal->p_payload, i_frame_size) < 0) break;
#endif
	    }
	}

	x264_encoder_close( h );
	x264_picture_clean( &pic );

	sws_freeContext(swsCtxt);

	socket_close();

	XCloseDisplay(dpy);

}
