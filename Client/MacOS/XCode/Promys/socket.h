#ifndef _SOCKET_H_
#define _SOCKET_H_

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
};

class Socket {
public:
    Socket();
    ~Socket();

    // Client
    int connect(const char *server, in_port_t port);

    // Server
    int listen(in_port_t port);
    int accept();

    // read/write helpers
    int receive(void *dst, unsigned int size);
    int send(void *dst, unsigned int size);

    int verbose;

private:
    int socket_server;
    int socket_client;
    struct sockaddr_in sock_desc; // Description of either server or client

    void error(const char *s);
};

#endif // _SOCKET_H_
