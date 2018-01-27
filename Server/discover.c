#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct __attribute__((__packed__)) _promys {
	unsigned int version;
	char cmd[8];
	unsigned short port;
};

static int s;

int
promys_listen() {
    struct sockaddr_in promys;

    s = socket(AF_INET, SOCK_DGRAM, 0);

    promys.sin_family = AF_INET;
    promys.sin_port   = htons(9999);
    promys.sin_addr.s_addr   = htonl(INADDR_ANY);

    bind(s, (struct sockaddr *) &promys, sizeof(promys));

    return s;
}

void
promys_reply() {
    struct sockaddr_in from;
    int ret;

    memset(&from, 0, sizeof(from));

    struct _promys query;

    unsigned int from_len = sizeof(from);

    memset(&query, 0, sizeof(query));

    ret = recvfrom(s, &query, sizeof(query), 0, (struct sockaddr *)&from, &from_len);

    if (ret < 0) {
	printf("recvfrom: %d (%d)\n", ret, errno);
	return;
    }

    struct _promys reply;

    reply.version = 0x01000000;
    reply.port = htons(9000);
    strcpy(reply.cmd, "PROMYS");
    from_len = sizeof(from);

    ret = sendto(s, &reply, sizeof(reply), 0, (struct sockaddr *)&from, from_len);

    if (ret < 0) {
	printf("sendto: %d (%d)\n", ret, errno);
	return;
    }
}
