#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/fb.h>

#include "logo.c"

static int fb;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static unsigned char *framebuffer;

int
fb_init() {
    int console = open("/dev/console", O_RDWR);
    ioctl(console, KDSETMODE, KD_GRAPHICS);
    close(console);

    fb = open("/dev/fb0", O_RDWR);
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
    framebuffer = mmap(NULL, finfo.smem_len, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);

    return fb;
}

void
fb_splash() {
    // Clear screen
    memset(framebuffer, 0, finfo.smem_len);

    unsigned long offset;
    unsigned char *ptr;
    int x,y;

    offset = (vinfo.xres_virtual - logo_data.width)/2*4;
    offset += (vinfo.yres_virtual - logo_data.height)/2*finfo.line_length;

    ptr = framebuffer+offset;

    for(y=0; y < logo_data.height; y++) {
	    for(x=0; x < logo_data.width; x++) {
		    ptr[x*4  ] = logo_data.pixel_data[(x+y*logo_data.width)*4  ];
		    ptr[x*4+1] = logo_data.pixel_data[(x+y*logo_data.width)*4+1];
		    ptr[x*4+2] = logo_data.pixel_data[(x+y*logo_data.width)*4+2];
		    ptr[x*4+3] = logo_data.pixel_data[(x+y*logo_data.width)*4+3];
	    }
	    ptr += finfo.line_length;
    }
}

int
fb_close() {
    return close(fb);
}
