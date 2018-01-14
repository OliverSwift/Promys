#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>

static int br; // Socket
static struct sockaddr_in br_addr;

static struct {
	char title[8];
	unsigned short port;
} announce = {
	.title = "Promys",
	.port  = 0
};

int
broadcast_init(int port) {
	announce.port = htons(port);

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
	int ret;
	struct ifaddrs *iface_list, *iface;

	getifaddrs(&iface_list);

	for(iface = iface_list; iface;  iface = iface->ifa_next) {
		if (iface->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *addr;

			addr = (struct sockaddr_in *)iface->ifa_broadaddr;
			br_addr.sin_addr = addr->sin_addr;
			ret = sendto(br, &announce, sizeof(announce), 0, (const struct sockaddr *)&br_addr, sizeof(br_addr));
		}
	}

	freeifaddrs(iface_list);

	return ret;
}
