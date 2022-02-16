#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include "socket.h"
#include <csignal>

using namespace std;


int Socket::init_server(const char * port){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);;
    } 

    if (strcmp(port,"")==0){
        struct sockaddr_in * addrinfo=(struct sockaddr_in *)(host_info_list->ai_addr);
        addrinfo->sin_port=0;
    }

    socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);;
    } //if

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);;
    } //if

    status = listen(socket_fd, 100);
    if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl; 
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);;
    } 

    cout << "Waiting for connection on port " << port << endl;

    freeaddrinfo(host_info_list);
    request_fd=socket_fd;

    return socket_fd;
    }


int Socket::server_accept(char * client_hostname){
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */
    //char client_hostname[MAXLINE], client_port[MAXLINE];

    clientlen = sizeof(struct sockaddr_storage);
    int connfd = accept(request_fd, (struct sockaddr *)&clientaddr, &clientlen);
    if (connfd == -1) {
        cerr << "Error: cannot accept connection on socket" << endl;
        exit(EXIT_FAILURE);
    }

    char * host=new char[MAXLINE]{0};
    char * service=new char[MAXLINE]{0};

    getnameinfo((struct sockaddr *) &clientaddr, clientlen, host, MAXLINE,
    service, MAXLINE, 0);

    memcpy(client_hostname,host,MAXLINE);
    
    return connfd;
    
}

char * Socket::recv_buffer(int connfd){
    char * buffer=new char[MAXLINE]{0};
    int byte=recv(connfd,buffer,MAXLINE,0);
    cout<<byte<<endl;
    cout<<"socket receive: "<<endl;
    cout<<buffer<<endl;
    return buffer;  
}

void Socket::send_buffer(int connfd,const char * buffer){
    send(response_fd,buffer,MAXLINE,0);
    cout<<"socket send:"<<endl;
    cout<<buffer<<endl;
}


int Socket::init_client(const char * hostname, const char * port) {
    server_host=hostname;
    server_port=port;
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }  //if

    socket_fd = socket(host_info_list->ai_family,
                        host_info_list->ai_socktype,
                        host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);;
    }  //if

    cout << "Connecting to " << hostname << " on port " << port << "..." << endl;

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);;
    }  //if

    // const char * message = "hi there!";
    // send(socket_fd, message, strlen(message), 0);

    freeaddrinfo(host_info_list);
    // close(socket_fd);
    response_fd=socket_fd;
    return socket_fd;
    }

    Socket::~Socket(){

    }

    void sigchld_handler(int sig) {
        while (waitpid(-1, 0, WNOHANG) > 0) ;
        return;
}

int main(int argc, char **argv) {
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]); 
        exit(0);
    }
    Socket socket;
    const char * port=argv[1];
    char * client_hostname=new char[MAXLINE]{0};;
    int listenfd, connfd;  
    signal(SIGCHLD, sigchld_handler); 
    listenfd = socket.init_server(port);
    //int socket_server_fd;
    while (1) { 
        connfd = socket.server_accept(client_hostname); 
        if (fork() == 0) {
            close(listenfd); /* Child closes its listening socket */ 
            char * request_buffer=socket.recv_buffer(connfd); /* Child services client */
            int socket_server_fd=socket.init_client("www.baidu.com","80");
            socket.send_buffer(socket_server_fd,request_buffer);
            char * response_buffer=socket.recv_buffer(socket.response_fd);
            socket.send_buffer(socket.request_fd,response_buffer);
            close(socket_server_fd);
            close(connfd); /* Child closes connection with client */ 
            exit(0);/* Child exits */ 
        } 
    close(connfd); /* Parent closes connected socket (important!) */ 
    }
    return 0;
}




