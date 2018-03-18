/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Please checkout LICENSE file.
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

static int br; // Socket
static struct sockaddr_in br_addr;
struct __attribute__((__packed__)) _promys {
	unsigned int version;
	char cmd[8];
	unsigned short port;
};

char *
promys_discover(int *port) {
	int ret;
	struct ifaddrs *iface_list, *iface;
	char *promys = NULL;
	struct _promys query = {
	    .version = 0x01000000,
	    .port    = 0,
	    .cmd     = "QUERY"
	};

	br = socket(AF_INET, SOCK_DGRAM, 0);

	int broadcastEnable = 1;
	ret = setsockopt(br, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

	if (ret < 0) return NULL;

	br_addr.sin_family = AF_INET;
	br_addr.sin_port   = htons(9999);  // Broadcast on port 9999
	br_addr.sin_addr.s_addr = INADDR_BROADCAST;

	ret = getifaddrs(&iface_list);

	if (ret) {
	    printf("Error: %d (%s)", errno, strerror(errno));
	    return NULL;
	}

	while(1) {
	    for(iface = iface_list; iface;  iface = iface->ifa_next) {
		if (iface->ifa_addr->sa_family == AF_INET) {
		    struct sockaddr_in *addr;

		    addr = (struct sockaddr_in *)iface->ifa_broadaddr;
		    if (addr) {
			br_addr.sin_addr = addr->sin_addr;
			sendto(br, &query, sizeof(query), 0, (const struct sockaddr *)&br_addr, sizeof(br_addr));
		    }
		}
	    }

	    // Wait for reply
	    struct sockaddr_in from;
	    unsigned int from_len = sizeof(from);
	    struct _promys reply;
	    fd_set fds;
	    struct timeval to;

	    FD_ZERO(&fds);
	    FD_SET(br, &fds);
	    to.tv_sec = 2;
	    to.tv_usec = 0;

	    ret = select(br+1, &fds, 0, 0, &to);

	    if (ret <= 0) continue;

	    ret = recvfrom(br, &reply, sizeof(reply), 0, (struct sockaddr *)&from, &from_len);
	    if (ret == sizeof(reply)) {
		promys = inet_ntoa(from.sin_addr);
		*port = ntohs(reply.port);
		break;
	    }
	}

	freeifaddrs(iface_list);

	return promys;
}
