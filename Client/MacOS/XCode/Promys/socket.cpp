#include "socket.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

Socket::Socket() {
    socket_server = -1;
    socket_client = -1;
    verbose = 0;
}

Socket::~Socket() {
    close(socket_server);
    close(socket_client);
}

void
Socket::error(const char *s) {
    if (verbose)
	printf("%s failed (%d): %s\n", s, errno, strerror(errno));
}

int
Socket::connect(const char *server, unsigned short port) {
    struct hostent     *host_desc;

    if (server == NULL) return -2;

    host_desc = gethostbyname(server);
    if (host_desc == 0) {
	error("Hostname resolution");
	return -2;
    }

    socket_client = socket(AF_INET,SOCK_STREAM,0);
    if (socket_client < 0) {
	error("Socket creation");
	return -1;
    }

    sock_desc.sin_family = AF_INET;

    memcpy(&sock_desc.sin_addr,
	   host_desc->h_addr,
	   host_desc->h_length);
    sock_desc.sin_port = htons(port);

    if (::connect(socket_client,(struct sockaddr *)&sock_desc,sizeof(sock_desc)) < 0) {
	error("Connect");
	close(socket_client);
	socket_client = -1;
	return -3;
    }

    signal(SIGPIPE, SIG_IGN);

    return socket_client;
}

int
Socket::listen(unsigned short port) {
    struct sockaddr_in sock_desc;
    int v;

    socket_server = socket(AF_INET,SOCK_STREAM,0);
    if (socket_server < 0) {
	error("Socket creation");
	return -1;
    }
    sock_desc.sin_family      = AF_INET; 
    sock_desc.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_desc.sin_port        = htons(port);
    if (bind(socket_server,(struct sockaddr *)&sock_desc,sizeof(sock_desc)) < 0) {
	error("Bind");
	close(socket_server);
	socket_server = -1;
	return -2;
    }
    v = 1;
    setsockopt(socket_server,SOL_SOCKET, SO_REUSEADDR,&v, sizeof(v));
    ::listen(socket_server,1);

    return socket_server;
}

int
Socket::accept() {
    socklen_t length = sizeof(sock_desc);

    socket_client = ::accept(socket_server,(struct sockaddr *)&sock_desc,&length);
    if (socket_client < 0) {
	error("Accept");
	return -1;
    }

    if (verbose) {
	printf("Connection from: %s\n", inet_ntoa(sock_desc.sin_addr));
    }

    signal(SIGPIPE, SIG_IGN);

    return socket_client;
}

int
Socket::receive(void *data, unsigned int size) {
    int n;
    unsigned char *dst = (unsigned char *)data;

    while(size) {
	n = (int)read(socket_client, dst, size);
	if (n<0) {
	    error("Receive");
	    return n;
	}
	size -= n;
	dst += n;
    }
    return size;
}

int
Socket::send(void *data, unsigned int size) {
    int n;
    unsigned char *dst = (unsigned char *)data;

    while(size) {
	n = (int)write(socket_client, dst, size);
	if (n<0) {
	    error("Send");
	    return n;
	}
	size -= n;
	dst += n;
    }
    return size;
}
