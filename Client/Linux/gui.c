#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
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
mainLoop(void *arg) {
    int *go = arg;

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
    *go = 0;
    exit(0);
}

int
gui_init(int *go) {
    *go = 1;

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

    XMapRaised(dpy, win);

    XStoreName(dpy, win, "Promys");

    pthread_create(&thread, NULL, mainLoop, 0);
}
