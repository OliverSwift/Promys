/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Checkout LICENSE file
 */
#include <windows.h>
int guiMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);

void showMessage(const char *message);
void hideWindow();

extern BOOL stationIsLocked;
