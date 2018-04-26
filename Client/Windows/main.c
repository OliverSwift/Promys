/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Checkout LICENSE file
 */
#include <windows.h>
#include <windows.h>
#include "gui.h"
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "discover.h"
#include "h264.h"
#include "socket.h"
#include <arpa/inet.h>

DWORD promys(LPVOID);

HANDLE thread;

int
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    thread = CreateThread(NULL, 0, promys, NULL, CREATE_SUSPENDED, NULL);
    guiMain(hInstance, hPrevInst, lpCmdLine, nCmdShow);
}

DWORD promys(LPVOID arg) {
	size_t height, width;

	char *cast_server;
	int cast_port;

	showMessage("Searching for PROMYS device");

	cast_server = promys_discover(&cast_port);

	if (cast_server == NULL) {
	    exit(1);
	} else {
	    printf("Found at %s:%d\n", cast_server, cast_port);
	    showMessage("PROMYS device found");
	    hideWindow();
	}

	if (socket_connect(cast_server, cast_port) < 0) {
	    printf("Unable to connect to Promys\n");
	    exit(1);
	}

	HDC hDCScreen = GetDC(NULL);

	HDC hDCMem = CreateCompatibleDC(hDCScreen);

	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);

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

	h264_init(width, height, linesize);

	showMessage("Broadcasting...");

	while(1) {
	    SYSTEMTIME start,stop;

	    GetSystemTime(&start);
	    StretchBlt(hDCMem, 0, bi.biHeight, bi.biWidth, -bi.biHeight, hDCScreen, 0, 0, bi.biWidth, bi.biHeight, SRCCOPY);

#if 1
	    // Draw cursor (poorly)
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
	    StretchBlt(hDCMem, ci.ptScreenPos.x, bmpScreen.bmHeight - ci.ptScreenPos.y, bm.bmWidth, -bm.bmHeight, cdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCPAINT);
	    DeleteObject(ii.hbmColor);
	    DeleteObject(ii.hbmMask);
	    DeleteDC(cdc);
#endif

	    // Gets the "bits" from the bitmap and copies them into a buffer 
	    // which is pointed to by lpbitmap.
	    GetDIBits(hDCMem, hBitmap, 0,
		(UINT)bmpScreen.bmHeight,
		lpbitmap, // pic.img.plane[0],
		(BITMAPINFO *)&bi, DIB_RGB_COLORS);

	    unsigned char *packet;
	    size_t packet_size;

	    packet = h264_encode(lpbitmap, &packet_size);
	    if (packet_size)
	    {
		if (socket_send(packet, packet_size) < 0) break;
	    }

#define DELAY_MS 50

	    GetSystemTime(&stop);
	    DWORD delta;

	    delta  = (stop.wSecond - start.wSecond)*1000;
	    if (delta < 0) delta += 60*1000;
	    delta += (stop.wMilliseconds - start.wMilliseconds);
	    if (delta < DELAY_MS) {
		Sleep(DELAY_MS-delta);
	    }
	}

	h264_close();

	free(lpbitmap);

	socket_close();

	return 0;
}
