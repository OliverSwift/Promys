/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Please checkout LICENSE file.
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
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
static Atom wm_delete_window, wm_protocols, cm_hide_window;
static char *message = "";

void
hideWindow() {
    XEvent event;

    event.type = ClientMessage;
    event.xclient.window = win;
    event.xclient.message_type = cm_hide_window;
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
        if (event.type == ClientMessage) {
            if (event.xclient.message_type == wm_protocols) {
                if (event.xclient.data.l[0] == wm_delete_window) {
                    break;
                }
            }
            if (event.xclient.message_type == cm_hide_window) {
                XIconifyWindow(dpy, win, screen);
            }
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
    if (dpy == NULL) {
	return -1;
    }

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

    // WM stuff
    char *name = "Promys";

    XStoreName(dpy, win, name);

    XClassHint *classHint = XAllocClassHint();
    if (classHint) {
        classHint->res_name = name;
        classHint->res_class = name;
    }
    XSetClassHint(dpy, win, classHint);
    XFree(classHint);

    // Some WM client message atoms
    wm_protocols     = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

    XSetWMProtocols(dpy, win, &wm_delete_window, 1);

    // Create our client message
    cm_hide_window = XInternAtom(dpy, "PROMYS_HIDE_WINDOW", False);

    pthread_create(&thread, NULL, mainLoop, go);

    return 0;
}
