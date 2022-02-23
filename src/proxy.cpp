#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <csignal>
#include <ctime>
#include <memory>
#include "socket.h"
#include "http_parser.h"
#include "threadpool.h"
#include "log.h"
#include "exceptions.h"
#include "cache.h"
#include <signal.h>

// global logger
Logger LOG;
// global cache
Cache cache;

void handle_get(const HttpRequest & req, int connfd, size_t rid) {
  if (req.get_method() != "GET") {
    throw HttpRequestExc("A unexpected logical error happened at handle_get()");
  }
  HttpParser parser;
  const HttpResponse* rsp_ptr = cache.search_record(req.get_URI());
  HttpResponse rsp;
  std::vector<char> rsp_vec;
  try {
  if (rsp_ptr == nullptr) { 
    std::unique_lock<std::mutex> lck (LOG.mtx);
    LOG << rid << ": not in cache\n";
    lck.unlock();
    Clientsocket client_socket;
    int webserver_fd = client_socket.init_client(req.get_host().c_str(), req.get_port().c_str());
    std::vector<char> req_vec = req.reconstruct();
    std::vector<char> s_line = req.get_start_line();
    lck.lock();
    LOG << rid << ": Requesting \"" << std::string(s_line.begin(), s_line.end()) 
        << "\" from " << req.get_host() << "\n";
    lck.unlock();
    client_socket.send_buffer(webserver_fd, req_vec.data(), req_vec.size());
    pair<vector<char>, int> response_buffer = client_socket.recv_response(webserver_fd);
    close(webserver_fd);
    rsp = parser.parse_response(response_buffer.first.data(), response_buffer.second);
    s_line = rsp.get_start_line();
    lck.lock();
    LOG << rid << ": Received \"" << std::string(s_line.begin(), s_line.end()) 
        << "\" from " << req.get_host() << "\n";
    lck.unlock();
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
        std::unique_lock<std::mutex> lck (LOG.mtx);
        LOG << rid << ": in cache, requires validation\n";
        lck.unlock();
        std::string etag = rsp_ptr->get_cache().etag;
        std::string last_modified = rsp_ptr->get_cache().last_modified;
        std::vector<char> revalidate_req = req.get_revalidate_req(req.get_URI(), etag, last_modified);
        Clientsocket client_socket;
        int webserver_fd = client_socket.init_client(req.get_host().c_str(), req.get_port().c_str());
        std::vector<char> s_line = req.get_start_line();
        lck.lock();
        LOG << rid << ": Requesting \"" << std::string(s_line.begin(), s_line.end()) 
            << "\" from " << req.get_host() << "\n";
        lck.unlock();
        client_socket.send_buffer(webserver_fd, revalidate_req.data(), revalidate_req.size());
        pair<vector<char>, int> response_buffer = client_socket.recv_response(webserver_fd);
        close(webserver_fd);
        rsp = *rsp_ptr;
        HttpResponse revalidate_rsp;
        revalidate_rsp = parser.parse_response(response_buffer.first.data(), response_buffer.second);
        s_line = revalidate_rsp.get_start_line();
        lck.lock();
        LOG << rid << ": Received \"" << std::string(s_line.begin(), s_line.end()) 
            << "\" from " << req.get_host() << "\n";
        lck.unlock();
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
        std::unique_lock<std::mutex> lck (LOG.mtx);
        LOG << rid << ": in cache, but expired at " << expire_time_str << "\n"; 
        lck.unlock();
        Clientsocket client_socket;
        int webserver_fd = client_socket.init_client(req.get_host().c_str(), req.get_port().c_str());
        std::vector<char> req_vec = req.reconstruct();
        std::vector<char> s_line = req.get_start_line();
        lck.lock();
        LOG << rid << ": Requesting \"" << std::string(s_line.begin(), s_line.end()) 
            << "\" from " << req.get_host() << "\n";
        lck.unlock();
        client_socket.send_buffer(webserver_fd, req_vec.data(), req_vec.size());
        pair<vector<char>, int> response_buffer = client_socket.recv_response(webserver_fd);
        close(webserver_fd);
        rsp = parser.parse_response(response_buffer.first.data(), response_buffer.second);
        s_line = rsp.get_start_line();
        lck.lock();
        LOG << rid << ": Received \"" << std::string(s_line.begin(), s_line.end()) 
            << "\" from " << req.get_host() << "\n";
        lck.unlock();
        if (rsp.get_code() == "200") {
          cache.revalidate(req.get_URI(), rsp,rid);
        }
      }    
    } else {
      std::unique_lock<std::mutex> lck (LOG.mtx);
      LOG << rid << ": in cache, valid\n";
      lck.unlock();
      rsp = *rsp_ptr;
      int64_t old_time = rsp.get_cache().age;
      time_t store_time = cache.get_store_time(req.get_URI());
      std::string new_time = std::to_string(old_time + store_time); 
      rsp.change_header_field("Age", new_time);
    }
  }
  rsp_vec = rsp.reconstruct();
  }
  catch (HttpRequestExc &e) {
    std::string str = "HTTP/1.1 400 Bad Request\r\n\r\n";
    rsp_vec = std::vector<char>(str.begin(), str.end());
    std::unique_lock<std::mutex> lck (LOG.mtx);
    LOG << rid << ": NOTE recieved an invaild request from client\n";
    LOG << rid << ": Responding \"" << "HTTP/1.1 400 Bad Request" << "\"\n";
    lck.unlock();
  }
  catch (HttpResponseExc &e) {
    std::string str = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
    rsp_vec = std::vector<char>(str.begin(), str.end());
    std::unique_lock<std::mutex> lck (LOG.mtx);
    LOG << rid << ": NOTE recieved an invaild response from server\n";
    LOG << rid << ": Responding \"" << "HTTP/1.1 502 Bad Gateway" << "\"\n";
    lck.unlock();
  }
  // send rsp to client
  Socket socket(connfd);
  std::vector<char> s_line = rsp.get_start_line();
  if (s_line.size() != 0) {
    std::unique_lock<std::mutex> lck (LOG.mtx);
    LOG << rid << ": Responding \"" << std::string(s_line.begin(), s_line.end()) << "\"\n";
    lck.unlock();
  }
  socket.send_buffer(connfd, rsp_vec.data(), rsp_vec.size());
  close(connfd);
}

void handle_post(const HttpRequest & req, int connfd,size_t rid) {
    // send request to server
    Clientsocket client_socket;
    int webserver_fd=client_socket.init_client(req.get_host().c_str(),req.get_port().c_str());
    client_socket.send_buffer(webserver_fd,req.reconstruct().data());
    // log ID: Requesting "REQUEST" from SERVER
    std::unique_lock<std::mutex> lck (LOG.mtx);
    LOG << rid << ": Requesting \""<<req.get_start_line().data()<< "\" from "<<req.get_host().data()<<"\n";
    lck.unlock();
    // recv response from server
    pair<vector<char>,int> response_buffer=client_socket.recv_response(webserver_fd);
    HttpResponse rsp;
    try {
    HttpParser parse;
    rsp=parse.parse_response(response_buffer.first.data(),response_buffer.second);
    }
    catch (HttpRequestExc &e) {
      std::string str = "HTTP/1.1 400 Bad Request\r\n\r\n";
      vector<char> rsp_vec = std::vector<char>(str.begin(), str.end());
      response_buffer.first = rsp_vec;
      response_buffer.second = rsp_vec.size();
      lck.lock();
      LOG << rid << ": NOTE recieved an invaild request from client\n";
      LOG << rid << ": Responding \"" << "HTTP/1.1 400 Bad Request" << "\"\n";
      lck.unlock();
    }
    catch (HttpResponseExc &e) {
      std::string str = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
      vector<char>rsp_vec = std::vector<char>(str.begin(), str.end());
      response_buffer.first = rsp_vec;
      response_buffer.second = rsp_vec.size();
      lck.lock();
      LOG << rid << ": NOTE recieved an invaild response from server\n";
      LOG << rid << ": Responding \"" << "HTTP/1.1 502 Bad Gateway" << "\"\n";
      lck.unlock();
    }
    // log ID: Received "RESPONSE" from SERVER
    if (rsp.get_start_line().size() != 0 ) {
      lck.lock();
      LOG << rid << ": Received "<<rsp.get_start_line().data()<<" from "<<req.get_host().data()<<"\n";
      lck.unlock();
    }
    //send back to web client
    client_socket.send_buffer(connfd,response_buffer.first.data(),response_buffer.second);
    //log responding
    if (rsp.get_start_line().size() != 0 ) {
      lck.lock();
      LOG << rid << ": Responding "<<rsp.get_start_line().data()<<"\n";
      lck.unlock();
    }
    close(webserver_fd); 
    close(connfd);
}

void handle_connection(const HttpRequest & req, int connfd, size_t rid) {
    // send request to server
    Clientsocket client_socket;
    std::unique_lock<std::mutex> lck (LOG.mtx);
    LOG << rid << ": Requesting Connection to " <<req.get_host().data()<<"\n";
    lck.unlock();
    int webserver_fd=client_socket.init_client(req.get_host().c_str(), "443");
    // response HTTP/1.1 200 OK\r\n\r\n
    // log ID: Received "RESPONSE" from SERVER
    client_socket.send_buffer(connfd,"HTTP/1.1 200 OK\r\n\r\n",19);
    lck.lock();
    LOG << rid << ": Responding "<< "HTTP/1.1 200 OK\n";
    lck.unlock();
    vector<int> fds={connfd,webserver_fd};
    int numfds=connfd>webserver_fd?connfd:webserver_fd;
    fd_set readfds;
    //struct timeval tv;
    //tv.tv_sec = 2;
    //int MAXLINE=65536;
    //std::unique_ptr<char> message(new char[MAXLINE]{0});
    while (true) {
        FD_ZERO(&readfds);
        for (int i = 0; i < 2; i++) {
        FD_SET(fds[i], &readfds);
        }
        select(numfds + 1, &readfds, NULL, NULL, NULL);

        int rv;
        pair<vector<char>, size_t> received;
        for (int i = 0; i < 2; i++) {
            if (FD_ISSET(fds[i], &readfds)) {
                received=client_socket.recv_request(fds[i]);
                rv=received.second;
                if (rv>0){
                  if (send(fds[1-i],received.first.data(),rv,0)<=0){
                    lck.lock();
                    LOG <<rid<< ": Tunnel closed\n";
                    lck.unlock();
                    return;
                  }
                }
                else{
                  lck.lock();
                  LOG <<rid<< ": Tunnel closed\n";
                  lck.unlock();
                  return;
                }
                break;
            }
        }
        
        
    }
    lck.lock();
    LOG <<rid<< ": Tunnel closed\n";
    lck.unlock();
    close(webserver_fd); 
    close(connfd);
}

void handle_request(int connfd, size_t rid) {
    //receive from client and parse
    try {
      try {
        Socket socket(connfd);
        pair<vector<char>,int> request_buffer=socket.recv_request(connfd); 
        //cout<<"request first "<<request_buffer.first.size()<<"second "<<request_buffer.second<<endl;
        HttpParser parser;
        HttpRequest req=parser.parse_request(request_buffer.first.data(),request_buffer.second);
        time_t cur_time = time(0); 
        std::unique_lock<std::mutex> lck (LOG.mtx);
        std::string ipfrom = "1.2.3.4";
        std::vector<char> s_line = req.get_start_line();
        LOG << rid << ": \"" << std::string(s_line.begin(), s_line.end()) << "\" from " << ipfrom << " @ " << ctime(&cur_time) << "\n"; 
        lck.unlock();
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
          std::unique_lock<std::mutex> lck (LOG.mtx);
          LOG<<rid<<": ERROR Wrong HTTP Method\n";
          lck.unlock();
        }
      }
      catch (HttpRequestExc &e) {
        std::string str = "HTTP/1.1 400 Bad Request\r\n\r\n";
        vector<char> rsp_vec = std::vector<char>(str.begin(), str.end());
        Socket socket(connfd);
        std::unique_lock<std::mutex> lck (LOG.mtx);
        LOG << rid << ": Responding \"" << "HTTP/1.1 400 Bad Request" <<"\n";
        lck.unlock();
        socket.send_buffer(connfd, rsp_vec.data(), 29);
      }
      catch (HttpResponseExc &e) {
        std::string str = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
        vector<char>rsp_vec = std::vector<char>(str.begin(), str.end());
        Socket socket(connfd);
        std::unique_lock<std::mutex> lck (LOG.mtx);
        LOG << rid << ": Responding \"" << "HTTP/1.1 502 Bad Gateway" <<"\n";
        lck.unlock();
        socket.send_buffer(connfd, rsp_vec.data(), 29);
      }
    }
    catch (SocketExc &e) {
      std::cerr << "An error occurs in socket connection when handling request\n"; 
    }
    catch (std::exception &e) {
      std::cerr << "An unexpected error occurs when handling request\n"; 
      std::cerr << e.what() << std::endl;
    }
    return;
}

int main(int argc, char **argv) {
    LOG.open("log.txt", std::ofstream::out | std::ofstream::app);
    size_t rid = 0;
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]); 
        exit(0);
    }
    int listenfd;   
    try {
    Threadpool thread_pool;
    Threadpool* pool = thread_pool.get_pool();
    Serversocket server_socket;
    const char * port=argv[1];
    listenfd = server_socket.init_server(port);
    while (true) {
        int connfd;
        connfd = server_socket.server_accept(); 
        pool->assign_task(bind(handle_request, connfd, rid));
        ++rid;
    } 
    close(listenfd);
    }
    catch (SocketExc &e) {
      std::cerr << "An error occurs in socket connection when receiving request\n"; 
    }
    catch (std::exception &e) {
      std::cerr << "An unexpected error occurs when receiving request\n"; 
    }
    return 0;
}

