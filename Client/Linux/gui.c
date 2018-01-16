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

        printf("Got event\n");

        if (event.type == Expose && event.xexpose.count == 0) {
            int width;
            XClearWindow(dpy, win);
            width = XTextWidth(font_info, message, strlen(message));
            XDrawString(dpy, win, gc, (350-width)/2, 40, message, strlen(message));
            XFlush(dpy);
        }
    }

    XCloseDisplay(dpy);
}

int
gui_init() {
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

    pthread_create(&thread, NULL, mainLoop, 0);
}
