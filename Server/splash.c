#include <stdio.h>
#include <stdlib.h>
#include "jpeglib.h"
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/fb.h>

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

    // Force 1920x1080
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    vinfo.xres = 1920;
    vinfo.yres = 1080;
    vinfo.xres_virtual = vinfo.xres;
    vinfo.yres_virtual = vinfo.yres;
    ioctl(fb, FBIOPUT_VSCREENINFO, &vinfo);

    // Get actual settings
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
    framebuffer = mmap(NULL, finfo.smem_len, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);

    return fb;
}

#include <setjmp.h>

struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

void
fb_splash() {
    // Clear screen
    memset(framebuffer, 0, finfo.smem_len);

    long offset;
    unsigned char *ptr;

  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  FILE *infile;                 /* source file */
  JSAMPARRAY buffer;            /* Output row buffer */
  int row_stride;               /* physical row width in output buffer */

  if ((infile = fopen(getenv("PROMYS_BACKGROUND"), "rb")) == NULL) {
    return;
  }

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return;
  }
  
  jpeg_create_decompress(&cinfo);

  jpeg_stdio_src(&cinfo, infile);

  (void) jpeg_read_header(&cinfo, TRUE);

  cinfo.out_color_components = 4;
  cinfo.out_color_space = JCS_EXT_BGRA;

  (void) jpeg_start_decompress(&cinfo);
  
  offset = (vinfo.xres_virtual - cinfo.output_width)/2*4;
  offset += (vinfo.yres_virtual - cinfo.output_height)/2*finfo.line_length;

  if (offset < 0) offset = 0;

  ptr = framebuffer+offset;

  row_stride = cinfo.output_width * cinfo.output_components;
 
  buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  if (row_stride > finfo.line_length) {
	  row_stride = finfo.line_length;
  }

  if (cinfo.output_height > vinfo.yres_virtual) {
	  cinfo.output_height = vinfo.yres_virtual;
  }

  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

    memcpy(ptr, buffer[0], row_stride);
    ptr += finfo.line_length;
  }

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  fclose(infile);
}

int
fb_close() {
    return close(fb);
}
