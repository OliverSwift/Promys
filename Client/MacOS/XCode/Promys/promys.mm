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
#include "h264.h"
#include "discover.h"
}
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

        width = CGImageGetWidth(image_ref);
        height = CGImageGetHeight(image_ref);

	size_t linesize = width *4;
    
        h264_init(width, height, linesize);
    
        [ self showMessage:@"Broadcasting..." ];
	[ self hideWindow ];
    
        change_priority();
    
	while(1) {
	    CFDataRef dataref = CGDataProviderCopyData(CGImageGetDataProvider(image_ref));
	    const unsigned char *data = CFDataGetBytePtr(dataref);
            
            unsigned char *packet;
            size_t packet_size;
            
            packet = h264_encode(data, &packet_size);

	    CFRelease(dataref);
	    CGImageRelease(image_ref);

            if (packet_size) {
                if (cast->send(packet, (int)packet_size) < 0) break;
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

        h264_close();
    
	delete cast;
}
@end

