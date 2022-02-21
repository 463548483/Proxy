#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include "socket.h"
#include <csignal>
#include "exceptions.h"


using namespace std;
int MAXLINE=65536;

int Serversocket::init_server(const char * port){
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
        throw SocketExc("Error: cannot get address info for host");
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    } 

    if (strcmp(port,"")==0){
        struct sockaddr_in * addrinfo=(struct sockaddr_in *)(host_info_list->ai_addr);
        addrinfo->sin_port=0;
    }

    socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    if (socket_fd == -1) {
        throw SocketExc("Error: cannot create socket");
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    } 

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        throw SocketExc("Error: cannot bind socket");
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    } //if

    status = listen(socket_fd, 100);
    if (status == -1) {
        throw SocketExc("Error: cannot listen on socket"); 
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    } 

    cout << "Waiting for connection on port " << port << endl;

    freeaddrinfo(host_info_list);
    listenfd=socket_fd;
    return socket_fd;
    }


int Serversocket::server_accept(){
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */
    //char client_hostname[MAXLINE], client_port[MAXLINE];

    clientlen = sizeof(struct sockaddr_storage);
    int connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
    if (connfd == -1) {
        throw SocketExc("Error: cannot accept connection on socket");
        exit(EXIT_FAILURE);
    }
    cout<<"Accept listenfd"<<endl;
    char * host=NULL;
    char * service=NULL;

    getnameinfo((struct sockaddr *) &clientaddr, clientlen, host, MAXLINE,
    service, MAXLINE, 0);

    //memcpy(client_hostname,host,MAXLINE);
    this->connfd=connfd;
    return connfd;
    
}

pair<vector<char>, size_t> Socket::recv_response(int connfd){
    //char * recv_buffer=(char *)malloc(MAXLINE*sizeof(char));
    vector<char> recv_buffer;
    size_t total_byte=0;
    while(true){
        char * buffer=new char[MAXLINE]{0};
        memset(buffer,0,sizeof(char)*MAXLINE);
        int byte=recv(connfd,buffer,MAXLINE,0);
        if (byte==-1){
            throw SocketExc("Error Receive");
        }
        total_byte+=byte;
        recv_buffer.reserve(recv_buffer.size()+byte-1);
        recv_buffer.insert(recv_buffer.end(),buffer,buffer+byte);
        delete[] buffer;
        if (byte==0){ 
            break;
        }
        
        //recv_buffer=(char *)realloc(recv_buffer,(total_byte+1)*sizeof(char));
        //strcat(recv_buffer,buffer);
    }
    cout<<"socket receive: "<<endl;
    cout<<recv_buffer.data()<<endl;
    return pair<vector<char>, size_t>(recv_buffer,total_byte);  
}

pair<vector<char>, size_t> Socket::recv_request(int connfd){
    //char * recv_buffer=(char *)malloc(MAXLINE*sizeof(char));
    vector<char> recv_buffer;
    size_t total_byte=0;
    char * buffer=new char[MAXLINE]{0};
    memset(buffer,0,sizeof(char)*MAXLINE);
    int byte=recv(connfd,buffer,MAXLINE,0);
    if (byte==-1){
        throw SocketExc("Error Receive");
    }
    total_byte+=byte;
    recv_buffer.reserve(recv_buffer.size()+byte-1);
    recv_buffer.insert(recv_buffer.end(),buffer,buffer+byte);
    delete[] buffer;

    cout<<"socket receive: "<<endl;
    cout<<recv_buffer.data()<<endl;
    return pair<vector<char>, size_t>(recv_buffer,total_byte);  
}

void Socket::send_buffer(int connfd,const char * buffer,int length=MAXLINE){
    int byte=send(connfd,buffer,length,0);
    if (byte==-1){
        throw SocketExc("Error Send");
    }
    cout<<"socket send:"<<endl;
    cout<<buffer<<endl;
}


int Clientsocket::init_client(const char * hostname, const char * port) {
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        throw SocketExc("Error: cannot get address info for host");
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }  

    socket_fd = socket(host_info_list->ai_family,
                        host_info_list->ai_socktype,
                        host_info_list->ai_protocol);
    if (socket_fd == -1) {
        throw SocketExc("Error: cannot create socket");
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);;
    }  

    cout << "Connecting to " << hostname << " on port " << port << "..." << endl;

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        throw SocketExc("Error: cannot connect to socket");
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }  

    freeaddrinfo(host_info_list);
    this->connfd=socket_fd;
    return socket_fd;
    }

    Socket::~Socket(){

    }






