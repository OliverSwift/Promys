#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <netinet/in.h> /* in_port_t */

typedef struct socket_ *socket_t;

socket_t socket_create();
void socket_destroy(socket_t sock);

int socket_listen(socket_t sock, in_port_t port);

int socket_accept(socket_t sock);

/* _SOCKET_H_ */
#endif
