/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Checkout LICENSE file
 */
#include <stdio.h>
#include <stdlib.h>
#include "jpeglib.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/fb.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static int fb;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static unsigned char *framebuffer;

static FT_Library    library;
static FT_Face       face;

int
fb_init() {
    int ret;
    int console = open("/dev/console", O_RDWR);
    ioctl(console, KDSETMODE, KD_GRAPHICS);
    close(console);

    fb = open("/dev/fb0", O_RDWR);

    // Force 1920x1080
    ret = ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    if (ret < 0) fprintf(stderr, "ioctl(%d): %d. Errno = %d\n", __LINE__, ret, errno);
    vinfo.xres = 1920;
    vinfo.yres = 1080;
    vinfo.xres_virtual = vinfo.xres;
    vinfo.yres_virtual = vinfo.yres;
    ret = ioctl(fb, FBIOPUT_VSCREENINFO, &vinfo);
    if (ret < 0) fprintf(stderr, "ioctl(%d): %d. Errno = %d\n", __LINE__, ret, errno);

    // Get actual settings
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
    framebuffer = mmap(NULL, finfo.smem_len, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);
    fprintf(stderr, "FB MEM=%p %dx%d\n", framebuffer, vinfo.xres, vinfo.yres);

    // Init Freetype
	FT_Error      error;

	error = FT_Init_FreeType(&library);

    if (error) {
        printf("Failed to init Freetype lib\n");
    } else {
        error = FT_New_Face(library, getenv("PROMYS_TTF_FONT"), 0, &face);

        if (error) {
            printf("Failed to load font\n");
        } else {
            FT_Set_Pixel_Sizes( face, 0, 22);
        }
    }

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
 
  if (row_stride > finfo.line_length) {
	  row_stride = finfo.line_length;
  }

  if (cinfo.output_height > vinfo.yres_virtual) {
	  cinfo.output_height = vinfo.yres_virtual;
  }

  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, &ptr, 1);
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

static unsigned char
blend(unsigned char selector, unsigned char color, unsigned char source) {
    unsigned short value;

    value = selector * color + source * (~selector & 0xff);

    return (unsigned char)(value >> 8);
}

static void
my_draw_bitmap(FT_Bitmap *bitmap, int x, int y) {
    int r,c;

    unsigned char *ptr = framebuffer;

    ptr += finfo.line_length * y + x * 4;

    for(r = 0; r < bitmap->rows; r++) {
        for(c = 0; c < bitmap->width; c++) {
            unsigned char byte;

            byte = bitmap->buffer[r*bitmap->pitch + c];
            ptr[0 + c*4] = blend(byte, 0x40, ptr[0 + c*4]); // B
            ptr[1 + c*4] = blend(byte, 0xc2, ptr[1 + c*4]); // G;
            ptr[2 + c*4] = blend(byte, 0xf7, ptr[2 + c*4]); // R;
        }

        ptr += finfo.line_length;
    }
}

void
fb_print(int x, int y, const char *format, ...) {
	FT_Error      error;
	int           pen_x, pen_y, n;

	pen_x = x;
	pen_y = y;

	char text[256];

	va_list ap;

	va_start(ap, format);
	vsnprintf(text, sizeof(text), format, ap);
	va_end(ap);

	int num_chars = strlen(text);

	for ( n = 0; n < num_chars; n++ )
	{
	  /* load glyph image into the slot (erase previous one) */
	  error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
	  if ( error )
		continue;  /* ignore errors */

	  /* now, draw to our target surface */
	  my_draw_bitmap( &face->glyph->bitmap,
					  pen_x + face->glyph->bitmap_left,
					  pen_y - face->glyph->bitmap_top );

	  /* increment pen position */
	  pen_x += face->glyph->advance.x >> 6;
	}

}

void
fb_info() {
	const char *infoPath;
	FILE *info;
	int y;

	infoPath = getenv("PROMYS_INFO_FILE");
	if (infoPath == NULL) return;

	info = fopen(infoPath, "r");
	if (info == NULL) return;

	y = 940;
	while(1) {
		char line[256];

		if (fgets(line, sizeof(line), info) == NULL) break;
		line[strlen(line)-1] = 0; // remove eol
		fb_print(64, y, "%s", line);
		y += 24;
	}
}
