#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <socket.h>
extern "C" {
#include <video.h>
#include <splash.h>
#include <broadcast.h>
}

int
main(int argc, char **argv) {
	int ret;
	int connection;

	Socket *cast;

	cast = new Socket();

	//cast->verbose = true;

	connection = cast->listen(9000);

	printf("\033[2J\033[?25l\033[9;0]\n"); // Clear Screen, Cursor Off, Blanking Off

	fb_init();

	fb_splash();

	ret = broadcast_init(9000);
	if (ret < 0) {
		printf("broadcast_init: %s (%d)\n", strerror(errno), errno);
	}

	while (1) {
		fd_set rset;
		struct timeval timeout = {
			.tv_sec = 2,
			.tv_usec = 0
		};

		FD_ZERO(&rset);
		FD_SET(connection, &rset);
		int client;

		ret = select(connection+1, &rset, NULL, NULL, &timeout);

		switch(ret) {
			case 0:
				if (broadcast_send() < 0) {
					printf("Error: %s (%d)\n", strerror(errno), errno);
				}
				break;
			case 1:
				client = cast->accept();

				if (client > 0) {
					pid_t pid;

					pid = fork();
					switch (pid) {
						case 0: // Child
							video_decode(client);
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
				break;
			default:
				break;
		} 

	}
}
