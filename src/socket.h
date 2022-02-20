#ifndef SOCKET_H
#define SOCKET_H
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

using namespace std;


class Socket{
    public:
    int connfd;//connfd to send/recv
    public:
    Socket(int connfd):connfd(connfd){}
    pair<const char *,size_t> recv_buffer(int connfd);
    void send_buffer(int connfd,const char * buffer,int length);
    ~Socket();
};

class Serversocket:public Socket{
    public:
    int listenfd;
    const char *server_host;//the host of web server
    const char *server_port;//the port of web server
    public:
    Serversocket():Socket(0),listenfd(0),server_host(NULL),server_port(NULL){}
    int init_server(const char * self_port);
    int server_accept();
    //~Serversocket();
};

class Clientsocket:public Socket{
    public:
    Clientsocket():Socket(0){}
    int init_client(const char * hostname, const char * port);
    //~Clientsocket();
};

#endif