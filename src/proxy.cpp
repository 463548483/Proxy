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
#include "log.h"
#include "exceptions.h"
#include "cache.h"

// global logger
Logger LOG("log.txt");
// global cache
Cache cache;

void handle_get(const HttpRequest & req, int connfd) {
// 1. decide whether use cache
// 2. if true, send back cache
// 3. if need revalidate, construct revalidate_req
// 4. if false, send the original reques
// also need to LOG
  if (req.get_method() != "GET") {
    throw HttpRequestExc("A unexpected logical error happened at handle_get()");
  }
  const HttpResponse* rsp_ptr = cache.search_record(req.get_URI());
  if (rsp_ptr == nullptr) { 
    LOG << connfd << ": not in cache\n";
  } else {
    if (req.get_cache().no_cache || rsp_ptr->get_cache().no_cache){
    }
  }
}

void handle_request(int connfd) {
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
    Threadpool thread_pool;
    Threadpool* pool = thread_pool.get_pool();
    Serversocket server_socket;
    const char * port=argv[1];
    int listenfd;   
    listenfd = server_socket.init_server(port);
    while (true) { 
        int connfd;
        connfd = server_socket.server_accept(); 
        pool->assign_task(bind(handle_request, connfd));
    } 
    close(listenfd);    
    return 0;
}

