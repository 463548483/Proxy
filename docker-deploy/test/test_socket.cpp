#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include "../src/socket.h"
#include <csignal>
#include "../src/http_parser.h"
#include <mutex>
#include <functional>
#include <thread>
using namespace std;

void handle_request(unique_ptr<int> ptr) {
    int connfd = *(ptr.get());
    ptr.release(); 
    //receive from client and parse
    Socket socket(connfd);
    pair<vector<char>,int> request_buffer=socket.recv_request(connfd); 
    //cout<<"request first"<<request_buffer.first.data()<<"second"<<request_buffer.second<<endl;
    HttpParser parser;
    HttpRequest req=parser.parse_request(request_buffer.first.data(),request_buffer.second);
    //init connect to web server
    Clientsocket client_socket;
    int webserver_fd=client_socket.init_client(req.get_host().c_str(),req.get_port().c_str());
    client_socket.send_buffer(webserver_fd,request_buffer.first.data(),request_buffer.second);
    pair<vector<char>,int> response_buffer=client_socket.recv_response(webserver_fd);
    //send back to web client
    socket.send_buffer(connfd,response_buffer.first.data(),response_buffer.second);
    close(webserver_fd); 
    close(connfd);
    return;
}

int main(int argc, char **argv) {
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]); 
        exit(0);
    }
    Serversocket server_socket;
    const char * port=argv[1];
    int listenfd;   
    listenfd = server_socket.init_server(port);
    //int * connfd_ptr=new int;
    // connfd_ptr=(int *)malloc(sizeof(int)); 
    unique_ptr<int> p(new int);
    *p= server_socket.server_accept(); 
    handle_request(p);
    close(listenfd);    
    return 0;
}