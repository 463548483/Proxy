#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <memory>
#include "socket.h"
#include <csignal>
#include "exceptions.h"
#include <arpa/inet.h>
#include <string.h>


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

    //cout << "Waiting for connection on port " << port << endl;

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
    //cout<<"Accept listenfd"<<endl;
    char * host=NULL;
    char * service=NULL;

    getnameinfo((struct sockaddr *) &clientaddr, clientlen, host, MAXLINE,
    service, MAXLINE, 0);

    //memcpy(client_hostname,host,MAXLINE);
    struct  sockaddr_in * addr = (struct sockaddr_in *)&clientaddr;
    client_ip = inet_ntoa(addr->sin_addr);

    this->connfd=connfd;
    return connfd;
    
}

string Serversocket::get_client_ip(){
    return client_ip;
}

pair<vector<char>, size_t> Socket::recv_response(int connfd){
    //char * recv_buffer=(char *)malloc(MAXLINE*sizeof(char));
    //cout << "socket starting to receive" << endl;
    vector<char> recv_buffer;
    size_t total_byte=0;
    while(true){
        //cout << "get a block, size: " ;
        std::unique_ptr<char>  buffer(new char[MAXLINE]{0});
        memset(buffer.get(),0,sizeof(char)*MAXLINE);
        int byte=recv(connfd,buffer.get(),MAXLINE,0);
        //cout << byte << endl;
        if (byte==-1){
            std::string err = std::to_string(errno);
            cout<<"Socket Error when receiving response, recieve -1 bytes, errno:  "<<err<<endl;            
            //std::string err = std::to_string(errno);
            //cout<<"error recv"<<err<<endl;
            break;
            //throw SocketExc("Error Receive, errno: " + err);
        }
        total_byte+=byte;
        recv_buffer.reserve(total_byte);
        recv_buffer.insert(recv_buffer.end(),buffer.get(),buffer.get()+byte);
        if (byte==0){ 
            break;
        }
        
        //recv_buffer=(char *)realloc(recv_buffer,(total_byte+1)*sizeof(char));
        //strcat(recv_buffer,buffer);
    }
    //cout<<"socket receive: "<<total_byte<<endl;
    //cout<<std::string(recv_buffer.begin(), recv_buffer.end())<<endl;
    return pair<vector<char>, size_t>(recv_buffer,total_byte);  
}

pair<vector<char>, size_t> Socket::recv_request(int connfd){
    //cout << "socket starting to receive." << endl;
    //char * recv_buffer=(char *)malloc(MAXLINE*sizeof(char));
    vector<char> recv_buffer;
    size_t total_byte=0;
    std::unique_ptr<char>  buffer(new char[MAXLINE]{0});
    memset(buffer.get(),0,sizeof(char)*MAXLINE);
    int byte=recv(connfd,buffer.get(),MAXLINE,0);
    if (byte==-1){
        std::string err = std::to_string(errno);
        cout<<"Socket Error when receiving request, recieve -1 bytes, errno:  "<<err<<endl;            
        throw SocketExc("Error Receiving Request, errno: " + err);
    }
    total_byte+=byte;
    recv_buffer.reserve(total_byte);
    recv_buffer.insert(recv_buffer.end(),buffer.get(),buffer.get()+byte);

    //cout<<"socket receive: "<<total_byte<<endl;
    //cout<<std::string(recv_buffer.begin(), recv_buffer.end())<<endl;
    return pair<vector<char>, size_t>(recv_buffer,total_byte);  
}

void Socket::send_buffer(int connfd,const char * buffer,int length){
    int byte=send(connfd,buffer,length,MSG_NOSIGNAL);
    if (byte==-1){
        throw SocketExc("Error Send");
    }
    //cout<<"socket send:"<<endl;
    //cout<<buffer<<endl;
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

    //cout << "Connecting to " << hostname << " on port " << port << "..." << endl;

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






