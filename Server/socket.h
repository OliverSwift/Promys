/*
 * Copyright (c) 2018, Olivier DEBON
 * All rights reserved.
 * Checkout LICENSE file
 */
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Client
int socket_connect(const char *server, in_port_t port);

// Server
int socket_listen(in_port_t port);
int socket_accept();

// read/write helpers
int socket_receive(void *dst, unsigned int size);
int socket_send(void *dst, unsigned int size);

void socket_close();

#endif // _SOCKET_H_
