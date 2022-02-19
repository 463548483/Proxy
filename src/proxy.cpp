#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include "socket.h"
#include <csignal>
#include "http_parser.h"
#include "threadpool.h"

void handle_request(void * ptr) {
    int connfd = *((int *)ptr); 
    free(ptr);
    //receive from client and parse
    Socket socket(connfd);
    pair<const char *,int> request_buffer=socket.recv_buffer(connfd); /* Child services client */
    HttpParser parser;
    HttpRequest req=parser.parse_request(request_buffer.first,request_buffer.second);
    //init connect to web server
    Clientsocket client_socket;
    int webserver_fd=client_socket.init_client(req.get_host().c_str(),req.get_port().c_str());
    client_socket.send_buffer(webserver_fd,request_buffer.first);
    pair<const char *,int> response_buffer=client_socket.recv_buffer(webserver_fd);
    //send back to web client
    socket.send_buffer(connfd,response_buffer.first);
    close(webserver_fd); 
    close(connfd);
    return;
}

int main(int argc, char **argv) {
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]); 
        exit(0);
    }
    std::shared_ptr<Threadpool> pool(Threadpool::get_pool());
    Serversocket server_socket;
    const char * port=argv[1];
    int listenfd;   
    listenfd = server_socket.init_server(port);
    int * connfd_ptr;
    while (true) { 
        connfd_ptr=(int *)malloc(sizeof(int)); 
        *connfd_ptr= server_socket.server_accept(); 
        pool->init_pool(bind(handle_request, connfd_ptr));
    } 
    close(listenfd);    
    return 0;
}

