/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Checkout LICENSE file
 */
#ifndef _SPLASH_H_
#define _SPLASH_H_
int fb_init();
void fb_splash();
int fb_close();
void fb_print(int x, int y, const char *, ...);
void fb_info();
#endif
