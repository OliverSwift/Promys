#include <windows.h>
#include "gui.h"
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
extern "C" {
#include <libswscale/swscale.h>
}
#include <x264.h>
#include "socket.h"
#include <arpa/inet.h>

#define FILE_DUMP

char *
find_promys(int *port) {
    int ret;
    int s;
    struct sockaddr_in promys, from;

    s = socket(AF_INET, SOCK_DGRAM, 0);

    promys.sin_family = AF_INET;
    promys.sin_port   = htons(9999);
    promys.sin_addr.s_addr   = htonl(INADDR_ANY);

    ret = bind(s, (struct sockaddr *) &promys, sizeof(promys));

    memset(&from, 0, sizeof(from));

    struct {
	char  title[8];
	short port;
    } announce;

    int from_len = sizeof(from);

    memset(&announce, 0, sizeof(announce));

    ret = recvfrom(s, &announce, sizeof(announce), 0, (struct sockaddr *)&from, &from_len);
    if (ret < 0) {
	return NULL;
    }

    *port = ntohs(announce.port);

    return inet_ntoa(from.sin_addr);
}

DWORD promys(LPVOID);

int
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    CreateThread(NULL, 0, promys, NULL, 0, NULL);
    guiMain(hInstance, hPrevInst, lpCmdLine, nCmdShow);
}

DWORD promys(LPVOID) {
#ifdef FILE_DUMP
	FILE *out = fopen("out.h264","wb");
#endif
	unsigned int height, width;
	unsigned int out_width, out_height;

	Socket *cast;
	char *cast_server;
	int cast_port;

	Sleep(1000); // Give GUI thread some time to setup, kludgy

#ifndef FILE_DUMP
	showMessage("Searching for PROMYS device");

	cast_server = find_promys(&cast_port);

	if (cast_server == NULL) {
	    exit(1);
	} else {
	    printf("Found at %s:%d\n", cast_server, cast_port);
	    showMessage("PROMYS device found");
	    hideWindow();
	}

	cast = new Socket();

	if (cast->connect(cast_server, cast_port) < 0) {
	    printf("Unable to connect to Promys\n");
	    exit(1);
	}
#else
	showMessage("File dump mode");
	hideWindow();
#endif

	HDC hDCScreen = GetDC(NULL);

	HDC hDCMem = CreateCompatibleDC(hDCScreen);

	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);

	out_width = 1920;
	out_height = 1080;

	HBITMAP hBitmap = CreateCompatibleBitmap(hDCScreen, width, height);
	BITMAP bmpScreen;

	BITMAPFILEHEADER   bmfHeader;    
	BITMAPINFOHEADER   bi;
	 
	bi.biSize = sizeof(BITMAPINFOHEADER);    
	bi.biWidth = width;
	bi.biHeight = height;
	bi.biPlanes = 1;    
	bi.biBitCount = 32;    
	bi.biCompression = BI_RGB;    
	bi.biSizeImage = 0;  
	bi.biXPelsPerMeter = 0;    
	bi.biYPelsPerMeter = 0;    
	bi.biClrUsed = 0;    
	bi.biClrImportant = 0;

	DWORD dwBmpSize = ((bi.biWidth * bi.biBitCount + 31) / 32) * 4 * bi.biHeight;
	int linesize = bi.biWidth *4;

	unsigned char *lpbitmap = (unsigned char *)malloc(dwBmpSize);    

	GetObject(hBitmap,sizeof(BITMAP),&bmpScreen);

	SelectObject(hDCMem, hBitmap);

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

	SwsContext *swsCtxt;

	swsCtxt = sws_getContext(width, height, AV_PIX_FMT_BGRA,
	                         param.i_width, param.i_height, AV_PIX_FMT_YUV420P,
				 SWS_FAST_BILINEAR, NULL, NULL, NULL);

	int i=0;

	showMessage("Broadcasting...");

	while(1) {
	    SYSTEMTIME start,stop;

	    GetSystemTime(&start);
	    StretchBlt(hDCMem, 0, bi.biHeight, bi.biWidth, -bi.biHeight, hDCScreen, 0, 0, bi.biWidth, bi.biHeight, SRCCOPY);

#if 0
	    // Draw cursor
	    CURSORINFO ci;
	    ICONINFO ii;
	    BITMAP bm;
	    HDC cdc;

	    ci.cbSize = sizeof(ci);
	    GetCursorInfo(&ci);
	    GetIconInfo(ci.hCursor, &ii);

	    cdc = CreateCompatibleDC(hDCMem);

	    SelectObject(cdc, ii.hbmColor);
	    GetObject(ii.hbmColor, sizeof(bm), &bm);
	    MaskBlt(hDCMem, ci.ptScreenPos.x, bmpScreen.bmHeight - ci.ptScreenPos.y, bm.bmWidth, bm.bmHeight, cdc, 0, 0, ii.hbmMask, 0,0, MAKEROP4(SRCPAINT,SRCCOPY) );
	    DeleteDC(cdc);
#endif

	    // Gets the "bits" from the bitmap and copies them into a buffer 
	    // which is pointed to by lpbitmap.
	    GetDIBits(hDCMem, hBitmap, 0,
		(UINT)bmpScreen.bmHeight,
		lpbitmap, // pic.img.plane[0],
		(BITMAPINFO *)&bi, DIB_RGB_COLORS);

	    sws_scale(swsCtxt,
	              &lpbitmap, &linesize,
		      0, height,
		      pic.img.plane,  pic.img.i_stride);

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
	    GetSystemTime(&stop);
	    DWORD delta;

	    delta  = (stop.wSecond - start.wSecond)*1000;
	    if (delta < 0) delta += 60*1000;
	    delta += (stop.wMilliseconds - start.wMilliseconds);
	    if (delta < 40) {
		Sleep(40-delta);
	    }
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

	free(lpbitmap);

	delete cast;

	return 0;
}
