#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

using namespace std;
int MAXLINE=1024;

class Socket{
    public:
    int request_fd;//connfd with web client
    const char *server_host;//the host of web server
    const char *server_port;//the port of web server
    int response_fd;//connfd with web server
    public:
    Socket():request_fd(0),server_host(NULL),server_port(NULL),response_fd(0){}
    int init_server(const char * self_port);
    int server_accept(char * client_hostname);
    char * recv_buffer(int connfd);
    void send_buffer(int connfd,const char * buffer);
    int init_client(const char * hostname, const char * port);

    ~Socket();
};