#include "proxy.cpp"

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
                break;
            }
        }
        //client or server close
        if (rv==0 ){
            delete[] message;
            break;
        }
        client_socket.send_buffer(fds[1-i],message);
    }
    LOG << "Tunnel closed\n";
    close(webserver_fd); 
    close(connfd);

}