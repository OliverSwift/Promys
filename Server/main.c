/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Checkout LICENSE file
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#include "socket.h"
#include "video.h"
#include "splash.h"
#include "discover.h"

int
main(int argc, char **argv) {
	int ret;
	int connection;
	int discover;
	int overscan = 0;

	connection = socket_listen(9000);

	printf("\033[2J\033[?25l\033[9;0]\n"); // Clear Screen, Cursor Off, Blanking Off

	fb_init();

	fb_splash();

    //fb_print(100, 800, "Message: %s", "Hello World!!!");

	discover = promys_listen();
	if (discover < 0) {
		fprintf(stderr, "promys_init: %s (%d)\n", strerror(errno), errno);
	}

	while (1) {
		fd_set rset;

		FD_ZERO(&rset);
		FD_SET(connection, &rset);
		FD_SET(discover, &rset);
		int client;

		ret = select(discover+1, &rset, NULL, NULL, NULL);

		if (ret < 0) break;

		if (FD_ISSET(discover, &rset)) {
			promys_reply();
		}

		if (FD_ISSET(connection, &rset)) {
			client = socket_accept();

			if (client > 0) {
				pid_t pid;

				pid = fork();
				switch (pid) {
					case 0: // Child
						video_decode(client, overscan);
						exit(0);
					case -1: // Error
						break;
					default: // Parent
						waitpid(pid, NULL, 0);
						close(client);
						fb_splash();
						break;
				}
			}
		}
	}
}
