#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <ctime>
#include <unordered_map>
#include <cassert>
#include <cstring>
#include <fstream>
#include <string>
#include <shared_mutex>
#include <mutex>
#include "http_response.h"
#include "log.h"
using namespace std;
typedef struct tm tm;

extern Logger LOG;

//expire
class Record {
  public:
  string URI;
  HttpResponse http_response;
  //string ETag;
  time_t expire_time;
  time_t store_time;

  public:
  Record(string uri,HttpResponse response,time_t expire_time,time_t store_time);
  time_t get_expire();

};

class Cache{
private:
  unordered_map<string,Record> record_lib;
  mutable shared_mutex mtx;
public:
  void remove_record(string uri);
  HttpResponse send_response(string uri);
  bool check_store_valid(RspCacheControl & cache);
  time_t parse_time(RspCacheControl & cache);

public:
  Cache(){}
  const HttpResponse * search_record(string uri);
  bool check_time_valid(string uri);//check if fresh/expire
  bool check_tag_valid(string uri);//check if has Etag/last_modify
  bool store_record(string uri, const HttpResponse & rsp,size_t rid);//store single record
  void revalidate(string uri,const HttpResponse & rsp,size_t rid);//replace httpresponse and re-calculate time
  time_t get_store_time(string uri);//return the time past since last stored  
  time_t get_expire_time(string uri);//return expire time of the record

};


#endif
