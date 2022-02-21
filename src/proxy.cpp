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

void handle_post(const HttpRequest & req, int connfd) {
    // send request to server
    Clientsocket client_socket;
    int webserver_fd=client_socket.init_client(req.get_host().c_str(),req.get_port().c_str());
    client_socket.send_buffer(webserver_fd,req.reconstruct().data());
    // log ID: Requesting "REQUEST" from SERVER
    LOG << "id" << "Requesting "<<req.get_start_line().data()<< " from "<<req.get_host().data()<<"\n";
    // recv response from server
    pair<vector<char>,int> response_buffer=client_socket.recv_response(webserver_fd);
    HttpParser parse;
    HttpResponse rsp=parse.parse_response(response_buffer.first.data(),response_buffer.second);
    // log ID: Received "RESPONSE" from SERVER
    LOG << "id" << "Received "<<rsp.get_start_line().data()<<" from "<<req.get_host().data()<<"\n";
    //send back to web client
    client_socket.send_buffer(connfd,response_buffer.first.data(),response_buffer.second);
    //log responding
    LOG << "id" << "Responding "<<rsp.get_start_line().data()<<"\n";
    close(webserver_fd); 
    close(connfd);

}

void handle_connection(const HttpRequest & req, int connfd) {
    // send request to server
    Clientsocket client_socket;
    int webserver_fd=client_socket.init_client(req.get_host().c_str(),req.get_port().c_str());
    client_socket.send_buffer(webserver_fd,req.reconstruct().data());
    // log ID: Requesting "REQUEST" from SERVER
    LOG << "id" << "Requesting "<<req.get_start_line().data()<< " from "<<req.get_host().data()<<"\n";
    // response HTTP/1.1 200 OK\r\n\r\n
    // log ID: Received "RESPONSE" from SERVER
    client_socket.send_buffer(connfd,"HTTP/1.1 200 OK\r\n\r\n");
    LOG << "id" << "Responding "<<"HTTP/1.1 200 OK"<<"\n";
    
    vector<int> fds={connfd,webserver_fd};
    int numfds=connfd>webserver_fd?connfd:webserver_fd;
    fd_set readfds;
    int MAXLINE=65536;
    char * message=new char[MAXLINE]{0};
    while (true) {
        FD_ZERO(&readfds);
        for (int i = 0; i < 2; i++) {
        FD_SET(fds[i], &readfds);
        }
        select(numfds + 1, &readfds, NULL, NULL, NULL);

        int rv;
        for (int i = 0; i < 2; i++) {
            if (FD_ISSET(fds[i], &readfds)) {
                rv=recv(fds[i], message, MAXLINE, MSG_WAITALL);
                if (rv!=0){
                  client_socket.send_buffer(fds[1-i],message);
                }
                break;
            }
        }
        //client or server close
        if (rv==0 ){
            delete[] message;
            break;
        }
        
    }
    LOG << "Tunnel closed\n";
    close(webserver_fd); 
    close(connfd);

}

void handle_request(int connfd) {
    //receive from client and parse
    Socket socket(connfd);
    pair<vector<char>,int> request_buffer=socket.recv_request(connfd); 
    //cout<<"request first"<<request_buffer.first.data()<<"second"<<request_buffer.second<<endl;
    HttpParser parser;
    HttpRequest req=parser.parse_request(request_buffer.first.data(),request_buffer.second);
    handle_connection(req,connfd);
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

