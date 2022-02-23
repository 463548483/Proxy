#include "../src/cache.h"
#include "../src/http_parser.h"
#include <cstring>
#include <string> 
#include <fstream>
#include <cassert>
using namespace std;


int main(){

    const char * input1 = "HTTP/1.1   200   OK\r\nHost : abc.com\r\nCache-Control  :no-store \r\ncontent-length: 026e \r\n\r\nname1=value1&name2=value2"; 
    const char * input2 = "HTTP/1.1   200   OK\r\nHost : abc.com\r\nCache-Control  :max-age=100 \r\ncontent-length: 026e \r\n\r\nname1=value1&name2=value2"; 
    size_t size1 = strlen(input1) + 1;
    HttpParser parser;
    HttpResponse rsp1 = parser.parse_response(input1, size1);
    string uri="test";
    Cache  c;
    bool store=c.store_record(uri,rsp1);
    assert(store==false);
    size_t size2 = strlen(input2) + 1;
    HttpResponse rsp2 = parser.parse_response(input2, size2);
    assert(store==true);
    HttpResponse * rtn_rsp=c.search_record(uri);
    assert(rtn_rsp==&rsp2);
    assert(c.check_time_valid(uri)==true);
    HttpResponse new_rsp;
    c.revalidate(uri,new_rsp);
    assert(c.search_record(uri)==&new_rsp);
    return 0;
}