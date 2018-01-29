#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> /* inet_ntoa */
#include <netinet/in.h>
#include <signal.h>

struct socket_ {
    int server;
    int client;
    struct sockaddr_in desc; /* Description of either server or client */
    _Bool verbose;
};

socket_t
socket_create() {
    socket_t sock = malloc(sizeof(struct socket_));

    sock->server = -1;
    sock->client = -1;
    sock->verbose = 0; /* disabled */

    return sock;
}

void
socket_destroy(socket_t sock) {
    shutdown(sock->server, SHUT_RDWR);
    shutdown(sock->client, SHUT_RDWR);

    free(sock);
}

int
socket_listen(socket_t sock, in_port_t port) {
    int optval = 1;

    sock->server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock->server < 0) {
        if(sock->verbose) {
            perror("[socket] socket()");
        }

        return -1;
    }

    sock->desc.sin_family      = AF_INET; 
    sock->desc.sin_addr.s_addr = htonl(INADDR_ANY);
    sock->desc.sin_port        = htons(port);
    if (bind(sock->server, (struct sockaddr *)(&sock->desc), sizeof(sock->desc)) < 0) {
        if(sock->verbose) {
            perror("[socket] bind()");
        }
        shutdown(sock->server, SHUT_RDWR);
        sock->server = -1;

        return -2;
    }

    setsockopt(sock->server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    listen(sock->server, 1);

    if(sock->verbose) {
        printf("Listening on: %d\n", port);
    }

    return sock->server;
}

int
socket_accept(socket_t sock) {
    socklen_t length = sizeof(sock->desc);

    sock->client = accept(sock->server, (struct sockaddr *)(&sock->desc), &length);
    if (sock->client < 0) {
        if(sock->verbose) {
            perror("[socket] accept()");
        }

        return -1;
    }

    if(sock->verbose) {
        printf("Connection from: %s\n", inet_ntoa(sock->desc.sin_addr));
    }

    signal(SIGPIPE, SIG_IGN);

    return sock->client;
}

