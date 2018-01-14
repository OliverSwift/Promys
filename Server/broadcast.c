#include <sys/socket.h>
#include <netinet/in.h>

static int tcp_port;

static int br; // Socket
static struct sockaddr_in br_addr;

int
broadcast_init(int port) {
	tcp_port = port;
	br = socket(AF_INET, SOCK_DGRAM, 0);

	int broadcastEnable = 1;
	int ret = setsockopt(br, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

	if (ret < 0) return ret;

	br_addr.sin_family = AF_INET;
	br_addr.sin_port   = htons(9999);  // Broadcast on port 9999
	br_addr.sin_addr.s_addr = INADDR_BROADCAST;

	return br;
}

int
broadcast_send() {
	struct {
		char title[8];
		unsigned short port;
	} announce = {
		.title = "Promys",
		.port  = 0
	};
	
	announce.port = htons(tcp_port);

	return sendto(br, &announce, sizeof(announce), 0, (const struct sockaddr *)&br_addr, sizeof(br_addr));
}
