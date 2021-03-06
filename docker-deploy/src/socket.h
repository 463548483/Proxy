#ifndef SOCKET_H
#define SOCKET_H
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>

using namespace std;


class Socket{
    public:
    int connfd;//connfd to send/recv
    public:
    Socket(int connfd):connfd(connfd){}
    pair<vector<char>, size_t> recv_response(int connfd);
    pair<vector<char>, size_t> recv_request(int connfd);
    void send_buffer(int connfd,const char * buffer,int length=65536);
    ~Socket();
};

class Serversocket:public Socket{
    public:
    int listenfd;
    const char *server_host;//the host of web server
    const char *server_port;//the port of web server
    string client_ip;
    public:
    Serversocket():Socket(0),listenfd(0),server_host(NULL),server_port(NULL){}
    Serversocket(int connfd):Socket(connfd),listenfd(0),server_host(NULL),server_port(NULL){}
    int init_server(const char * self_port);
    int server_accept();
    string get_client_ip();
    //~Serversocket();
};

class Clientsocket:public Socket{
    public:
    Clientsocket():Socket(0){}
    int init_client(const char * hostname, const char * port);
    //~Clientsocket();
};

#endif
