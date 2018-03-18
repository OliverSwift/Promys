#include <stdio.h>
#include "h264.h"
#include <libswscale/swscale.h>
#include <x264.h>

#undef FILE_DUMP

#ifdef FILE_DUMP
FILE *out = fopen("out.h264","wb");
#endif

static x264_param_t param;
static x264_picture_t pic;
static x264_picture_t pic_out;
static x264_t *h;
static x264_nal_t *nal;
static int i_nal;
static int i = 0;
static struct SwsContext *swsCtxt;
static size_t stride;
static int height;

void
h264_init(size_t input_width, size_t input_height, size_t input_stride) {

    if( x264_param_default_preset( &param, "ultrafast", "zerolatency" ) < 0 )
	exit(1);

    param.i_csp = X264_CSP_I420;
    param.i_width  = OUTPUT_WIDTH;
    param.i_height = OUTPUT_HEIGHT;
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

    swsCtxt = sws_getContext(input_width, input_height, AV_PIX_FMT_BGRA,
			     param.i_width, param.i_height, AV_PIX_FMT_YUV420P,
			     SWS_FAST_BILINEAR, NULL, NULL, NULL);

    stride = input_stride;
    height = input_height;
}

unsigned char *
h264_encode(const unsigned char *image_data, size_t * packet_size) {
    int linesize = stride;

    sws_scale(swsCtxt,
	      &image_data, &linesize,
	      0, height,
	      pic.img.plane,  pic.img.i_stride);

    int i_frame_size;

    pic.i_pts = i++;
    i_frame_size = x264_encoder_encode( h, &nal, &i_nal, &pic, &pic_out );
    if( i_frame_size < 0 )
	return NULL;

    *packet_size = (size_t)i_frame_size;

#ifdef FILE_DUMP
    if(i_frame_size)
	fwrite(nal->p_payload, 1, i_frame_size, out);
    }
#endif

    return nal->p_payload;
}

void h264_close() {
    int i_frame_size;

    /* Flush delayed frames */
    while( x264_encoder_delayed_frames( h ) )
    {
	i_frame_size = x264_encoder_encode( h, &nal, &i_nal, NULL, &pic_out );
	if (i_frame_size < 0) {
	    break;
	} else if (i_frame_size) {
#ifdef FILE_DUMP
	    fwrite(nal->p_payload, 1, i_frame_size, out);
#endif
	}
    }

    x264_encoder_close( h );
    x264_picture_clean( &pic );

    sws_freeContext(swsCtxt);
}
