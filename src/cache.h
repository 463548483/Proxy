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
  string Etag;
  string last_modified;
  public:
  Record(string uri,HttpResponse response,time_t expire_time,string Etag,string last_modify);
  HttpResponse get_response();
  void replace();
  time_t get_expire();
  string get_Etag();
  string get_last_modify();
};

class Cache{
  private:
  unordered_map<string,Record> record_lib;
  public:
  Cache();
  string search_record(string uri);
  void store_record(string uri,HttpResponse);
  void remove_record(string uri);
  HttpResponse send_response(string uri);
  bool check_store_valid(RspCacheControl);
  bool check_time_valid(Record r);
  bool check_tag_valid(Record r);
  time_t parse_time(RspCacheControl cache);
};


#endif
