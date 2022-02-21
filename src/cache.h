#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <ctime>
#include <unordered_map>
using namespace std;
#include "http_response.h"

typedef struct tm tm;

//expire
class Record {
  private:
  string URI;
  HttpResponse http_response;
  //string ETag;
  time_t expire_time;

  public:
  Record(string uri,HttpResponse response,time_t expire_time);
  HttpResponse get_response();
  void replace();
  time_t get_expire();

};

class Cache{
  private:
  unordered_map<string,Record> record_lib;
  private:

  void remove_record(string uri);
  HttpResponse send_response(string uri);
  bool check_store_valid(RspCacheControl);
  time_t parse_time(RspCacheControl cache);
  public:
  Cache();
  HttpResponse * search_record(string uri);
  bool check_time_valid(string uri);//check if fresh/expire
  bool check_tag_valid(string uri);//check if has Etag/last_modify
  void store_record(string uri,HttpResponse);//store single record
  void replace_header(string uri,HttpResponse);//only replace header file
  void replace_response(string uri,HttpResponse);//replace the whole response
};


#endif
