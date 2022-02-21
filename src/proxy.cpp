#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <csignal>
#include <ctime>
#include "socket.h"
#include "http_parser.h"
#include "threadpool.h"
#include "log.h"
#include "exceptions.h"
#include "cache.h"

// global logger
Logger LOG("log.txt");
// global cache
Cache cache;

void handle_get(const HttpRequest & req, int connfd, size_t rid) {
// 1. decide whether use cache
// 2. if true, send back cache
// 3. if need revalidate, construct revalidate_req
// 4. if false, send the original reques
// also need to LOG
  if (req.get_method() != "GET") {
    throw HttpRequestExc("A unexpected logical error happened at handle_get()");
  }
  HttpParser parser;
  const HttpResponse* rsp_ptr = cache.search_record(req.get_URI());
  HttpResponse rsp;
  if (rsp_ptr == nullptr) { 
    LOG << rid << ": not in cache\n";
    Clientsocket client_socket;
    int webserver_fd = client_socket.init_client(req.get_host().c_str(), req.get_port().c_str());
    std::vector<char> req_vec = req.reconstruct();
    client_socket.send_buffer(webserver_fd, req_vec.data(), req_vec.size());
    pair<vector<char>, int> response_buffer = client_socket.recv_response(webserver_fd);
    close(webserver_fd);
    rsp = parser.parse_response(response_buffer.first.data(), response_buffer.second);
    if (rsp.get_code() == "200") {
      cache.store_record(req.get_URI(), rsp,rid);
    }
  } else {
    if (rsp_ptr->get_code() != "200") {
      throw CacheExc("stored a response that is not 200"); 
    }
    if (req.get_cache().no_cache || rsp_ptr->get_cache().no_cache ||
        cache.check_time_valid(req.get_URI()) == false){
      if (cache.check_tag_valid(req.get_URI())) {
        LOG << rid << ": in cache, requires validation\n";
        std::string etag = rsp_ptr->get_cache().etag;
        std::string last_modified = rsp_ptr->get_cache().last_modified;
        std::vector<char> revalidate_req = req.get_revalidate_req(req.get_URI(), etag, last_modified);
        Clientsocket client_socket;
        int webserver_fd = client_socket.init_client(req.get_host().c_str(), req.get_port().c_str());
        client_socket.send_buffer(webserver_fd, revalidate_req.data(), revalidate_req.size());
        pair<vector<char>, int> response_buffer = client_socket.recv_response(webserver_fd);
        close(webserver_fd);
        rsp = *rsp_ptr;
        HttpResponse revalidate_rsp;
        revalidate_rsp = parser.parse_response(response_buffer.first.data(), response_buffer.second);
        // deal with revalidate response
        if (revalidate_rsp.get_code() == "304") { // replace header fields, store to cache
          rsp.replace_header_fields(&revalidate_rsp);
          cache.revalidate(req.get_URI(), rsp,rid);
        } else if (revalidate_rsp.get_code() == "200") { // replace all, store to cache     
          rsp = revalidate_rsp;
          cache.revalidate(req.get_URI(), rsp,rid);
        }
      } else {
        time_t expire_time = cache.get_expire_time(req.get_URI());
        std::string expire_time_str(ctime(&expire_time)); 
        LOG << rid << ": in cache, but expired at " << expire_time_str << "\n"; 
        Clientsocket client_socket;
        int webserver_fd = client_socket.init_client(req.get_host().c_str(), req.get_port().c_str());
        std::vector<char> req_vec = req.reconstruct();
        client_socket.send_buffer(webserver_fd, req_vec.data(), req_vec.size());
        pair<vector<char>, int> response_buffer = client_socket.recv_response(webserver_fd);
        close(webserver_fd);
        rsp = parser.parse_response(response_buffer.first.data(), response_buffer.second);
        if (rsp.get_code() == "200") {
          cache.revalidate(req.get_URI(), rsp,rid);
        }
      }    
    } else {
      LOG << rid << ": in cache, valid\n";
      rsp = *rsp_ptr;
      int64_t old_time = rsp.get_cache().age;
      time_t store_time = cache.get_store_time(req.get_URI());
      std::string new_time = std::to_string(old_time + store_time); 
      rsp.change_header_field("Age", new_time);
    }
  }
  // send rsp to client
  std::vector<char> rsp_vec = rsp.reconstruct();
  Socket socket(connfd);
  socket.send_buffer(connfd, rsp_vec.data(), rsp_vec.size());
  close(connfd);
}

void handle_post(const HttpRequest & req, int connfd,size_t rid) {
    // send request to server
    Clientsocket client_socket;
    int webserver_fd=client_socket.init_client(req.get_host().c_str(),req.get_port().c_str());
    client_socket.send_buffer(webserver_fd,req.reconstruct().data());
    // log ID: Requesting "REQUEST" from SERVER
    LOG << rid << ": Requesting "<<req.get_start_line().data()<< " from "<<req.get_host().data()<<"\n";
    // recv response from server
    pair<vector<char>,int> response_buffer=client_socket.recv_response(webserver_fd);
    HttpParser parse;
    HttpResponse rsp=parse.parse_response(response_buffer.first.data(),response_buffer.second);
    // log ID: Received "RESPONSE" from SERVER
    LOG << rid << ": Received "<<rsp.get_start_line().data()<<" from "<<req.get_host().data()<<"\n";
    //send back to web client
    client_socket.send_buffer(connfd,response_buffer.first.data(),response_buffer.second);
    //log responding
    LOG << rid << ": Responding "<<rsp.get_start_line().data()<<"\n";
    close(webserver_fd); 
    close(connfd);

}

void handle_connection(const HttpRequest & req, int connfd, size_t rid) {
    // send request to server
    Clientsocket client_socket;
    int webserver_fd=client_socket.init_client(req.get_host().c_str(),req.get_port().c_str());
    client_socket.send_buffer(webserver_fd,req.reconstruct().data());
    // log ID: Requesting "REQUEST" from SERVER
    LOG << rid << ": Requesting "<<req.get_start_line().data()<< " from "<<req.get_host().data()<<"\n";
    // response HTTP/1.1 200 OK\r\n\r\n
    // log ID: Received "RESPONSE" from SERVER
    client_socket.send_buffer(connfd,"HTTP/1.1 200 OK\r\n\r\n");
    LOG << rid << ": Responding "<<"HTTP/1.1 200 OK"<<"\n";
    
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
    LOG <<rid<< ": Tunnel closed\n";
    close(webserver_fd); 
    close(connfd);

}

void handle_request(int connfd, size_t rid) {
    //receive from client and parse
    Socket socket(connfd);
    pair<vector<char>,int> request_buffer=socket.recv_request(connfd); 
    //cout<<"request first"<<request_buffer.first.data()<<"second"<<request_buffer.second<<endl;
    HttpParser parser;
    HttpRequest req=parser.parse_request(request_buffer.first.data(),request_buffer.second);
    string method=req.get_method();
    if (method=="GET"){
      handle_get(req,connfd,rid);
    }
    else if (method=="POST"){
      handle_post(req,connfd,rid);
    }
    else if (method=="CONNECT"){
      handle_connection(req,connfd, rid);
    }
    else {
      LOG<<rid<<": ERROR HTTP Method\n";
    }
    return;
}

int main(int argc, char **argv) {
    size_t rid = 0;
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
        pool->assign_task(bind(handle_request, connfd, rid));
        ++rid;
    } 
    close(listenfd);    
    return 0;
}

