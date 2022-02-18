#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <ctime>
#include <map>
using namespace std;
#include "http_response.h"

//expire
class Record {
  private:
  string URI;
  HttpResponse http_response;
  //string ETag;
  struct tm expire_time;
  public:
  Record(string uri,HttpResponse response);
  HttpResponse get_response();

};

class Cache{
  private:
  map<string,Record> record_lib;
  public:
  Cache();
  string search_record(string uri);
  void store_record(string uri,HttpResponse);
  void remove_record(string uri);
  HttpResponse send_response(string uri);

};


#endif
