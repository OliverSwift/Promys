#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

static Display *dpy;
static Window win;
static int screen;
static GC gc;
static Font font;
static XFontStruct *font_info;
static pthread_t thread;

static char *message = "";

#define HIDE_WINDOW (0x0D)

void
hideWindow() {
    XEvent event;

    event.type = ClientMessage;
    event.xclient.window = win;
    event.xclient.message_type = HIDE_WINDOW;
    event.xclient.format = 8;
    XSendEvent(dpy, win, False, 0, &event);
    XFlush(dpy);
}

void
showMessage(char *msg) {
    message = msg;

    XEvent event;
    event.type = Expose;
    event.xexpose.count = 0;
    event.xexpose.window = win;
    XSendEvent(dpy, win, False, ExposureMask, &event);
    XFlush(dpy);
}

static void *
mainLoop() {
    while(1) {
        XEvent event;

        XNextEvent(dpy, &event);

        if (event.type == Expose && event.xexpose.count == 0) {
            int width;
            XClearWindow(dpy, win);
            width = XTextWidth(font_info, message, strlen(message));
            XDrawString(dpy, win, gc, (350-width)/2, 40, message, strlen(message));
            XFlush(dpy);
        }
        if (event.type == ClientMessage && event.xclient.message_type == HIDE_WINDOW) {
            XIconifyWindow(dpy, win, screen);
        }
    }

    XCloseDisplay(dpy);
}

int
gui_init() {
    XInitThreads();

    dpy = XOpenDisplay(NULL);
    screen = DefaultScreen(dpy);

    win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen),
                              10, 10, 350, 80, 1,
                              BlackPixel(dpy, screen), WhitePixel(dpy, screen));
    gc = XCreateGC(dpy, win, 0, 0);
    XSelectInput(dpy, win, ExposureMask);
    font = XLoadFont(dpy, "-*-fixed-*-*-*-*-18-*-*-*-*-*-*-*");
    font_info = XQueryFont(dpy, font);

    XSetFont(dpy, gc, font);

#if 0

	/* pointer to the WM hints structure. */
	XWMHints* win_hints;

	/* load the given bitmap data and create an X pixmap containing it. */
	Pixmap icon_pixmap = XCreateBitmapFromData(display,
											   win,
											   icon_bitmap_bits,
											   icon_bitmap_width,
											   icon_bitmap_height);
	if (!icon_pixmap) {
		fprintf(stderr, "XCreateBitmapFromData - error creating pixmap\n");
		exit(1);
	}

	/* allocate a WM hints structure. */
	win_hints = XAllocWMHints();
	if (!win_hints) {
		fprintf(stderr, "XAllocWMHints - out of memory\n");
		exit(1);
	}

	/* initialize the structure appropriately. */
	/* first, specify which size hints we want to fill in. */
	/* in our case - setting the icon's pixmap. */
	win_hints->flags = IconPixmapHint;
	/* next, specify the desired hints data.           */
	/* in our case - supply the icon's desired pixmap. */
	win_hints->icon_pixmap = icon_pixmap;

	/* pass the hints to the window manager. */
	XSetWMHints(display, win, win_hints);

	/* finally, we can free the WM hints structure. */
	XFree(win_hints);

#endif


    XMapRaised(dpy, win);

    XStoreName(dpy, win, "Promys");

    pthread_create(&thread, NULL, mainLoop, 0);
}
