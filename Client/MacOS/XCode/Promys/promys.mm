/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 */
#include "promys.h"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CGImage.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
extern "C" {
#include <libswscale/swscale.h>
#include "discover.h"
}
#include <x264.h>
#include "socket.h"
#include "AppDelegate.h"

#undef FILE_DUMP

#include <pthread.h>

static void change_priority() {
    struct sched_param sp;
    
    memset(&sp, 0, sizeof(struct sched_param));
    sp.sched_priority= sched_get_priority_max(2);
    
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp)  == -1) {
        printf("Failed to change priority.\n");
    }
}

@implementation Promys

-(void)showMessage:(NSString*)text {
    AppDelegate *appDelegate;
    
    appDelegate = [[ NSApplication sharedApplication ] delegate ] ;
    
    [ appDelegate performSelectorOnMainThread:@selector(showMessage:) withObject:text waitUntilDone:false];
}

-(void)hideWindow {
    [[ NSApplication sharedApplication ] hide:nil ];
}

- (void)main {
	struct timeval start,stop;
#ifdef FILE_DUMP
	FILE *out = fopen("out.h264","wb");
#endif
	Socket *cast;
	char *cast_server;
        int cast_port;
         
        [ self showMessage:@"Searching for PROMYS device" ];
    
	cast_server = promys_discover(&cast_port);
	printf("Found at %s:%d\n", cast_server, cast_port);

	cast = new Socket();

	cast->connect(cast_server, cast_port);

	gettimeofday(&start, NULL);

        CGImageRef image_ref = CGDisplayCreateImage(CGMainDisplayID());

        size_t width, height;
        size_t out_width, out_height;

        width = CGImageGetWidth(image_ref);
        height = CGImageGetHeight(image_ref);

	out_width = 1920;
	out_height = 1080;

	int linesize = width *4;

	x264_param_t param;
	x264_picture_t pic;
	x264_picture_t pic_out;
	x264_t *h;
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

	SwsContext *swsCtxt;

	swsCtxt = sws_getContext(width, height, AV_PIX_FMT_BGRA,
	                         param.i_width, param.i_height, AV_PIX_FMT_YUV420P,
				 SWS_FAST_BILINEAR, NULL, NULL, NULL);

	int i=0;

        [ self showMessage:@"Broadcasting..." ];
	[ self hideWindow ];
    
        change_priority();
    
	while(1) {
	    CFDataRef dataref = CGDataProviderCopyData(CGImageGetDataProvider(image_ref));
	    const unsigned char *data = CFDataGetBytePtr(dataref);

	    sws_scale(swsCtxt,
	              &data, &linesize,
		      0, height,
		      pic.img.plane,  pic.img.i_stride);

	    CFRelease(dataref);
	    CGImageRelease(image_ref);

	    pic.i_pts = i++;
	    i_frame_size = x264_encoder_encode( h, &nal, &i_nal, &pic, &pic_out );
	    if( i_frame_size < 0 )
		break;
	    else if( i_frame_size )
	    {
#ifdef FILE_DUMP
		fwrite(nal->p_payload, 1, i_frame_size, out);
#else
		if (cast->send(nal->p_payload, i_frame_size) < 0) break;
#endif
	    }

	    gettimeofday(&stop, NULL);

	    long delay;

	    delay = (stop.tv_sec - start.tv_sec)*1000000;
	    delay += (stop.tv_usec - start.tv_usec);
#define DELAY_US 50000
	    if (delay < DELAY_US) {
		usleep(DELAY_US - (int)delay);
	    }

	    gettimeofday(&start, NULL);
	    // Capture a new image
	    image_ref = CGDisplayCreateImage(CGMainDisplayID());
#if 0
            NSPoint mouse = [ NSEvent mouseLocation ];
#endif
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
		if (cast->send(nal->p_payload, i_frame_size) < 0) break;
#endif
	    }
	}

	x264_encoder_close( h );
	x264_picture_clean( &pic );

	sws_freeContext(swsCtxt);

	delete cast;
}
@end

