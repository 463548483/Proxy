#include "cache.h"

using namespace std;

Record::Record(string uri, HttpResponse response, time_t expire_time,time_t store_time):URI(uri),http_response(response),expire_time(expire_time),store_time(store_time){}

time_t Record::get_expire() {
  return expire_time;
}

//return 0 if could not find, else check the status of record
const HttpResponse * Cache::search_record(string uri) {
  unordered_map<string, Record>::iterator it;
  shared_lock lck(mtx);
  it = record_lib.find(uri);
  if (it != record_lib.end()) {
    return &record_lib.at(uri).http_response;
  }
  else {
    HttpResponse * ptr = NULL;
    return ptr;
  }
}

HttpResponse Cache::send_response(string uri) {
  shared_lock lck(mtx);
  return record_lib.at(uri).http_response;
}

void Cache::remove_record(string uri) {
  unordered_map<string, Record>::iterator it;
  unique_lock lck(mtx);
  //cout<<"find this record"<<endl;
  it = record_lib.find(uri);
  if (it != record_lib.end()) {   
    //cout<<"remove"<<endl;
    record_lib.erase(it);
  }
}

string Cache::check_store_valid(RspCacheControl & cache) {
  if (cache.is_private == true){
    return "cache-control: private";
  }
  else if (cache.no_store==true){
    return "cache-control: no-store";
  }
  else if (cache.max_age == -1 and cache.has_expires == true and cache.expires == -1) {
    return "cache with expire time but expire time is not valid";
  }
  else {
    return "";
  }
}

time_t Cache::parse_time(RspCacheControl & cache) {
  if (cache.no_cache == true or cache.max_age == 0) {
    return time(0);
  }
  else if (cache.max_age < 0 and cache.has_expires == false) {
    return 2147483647;
  }
  else if (cache.max_age > 0) {
    return time(0) + ((uint64_t)cache.max_age - cache.age);
  }
  else {
    return cache.expires;
  }
}

bool Cache::store_record(string uri, const HttpResponse & response,size_t rid) {
  RspCacheControl cache = response.get_cache();
  //cout<<"check store valid"<<endl;
  if (check_store_valid(cache)!="") {
    std::unique_lock<std::mutex> lck (LOG.mtx);
    LOG<<rid<<": not cacheable because "<<check_store_valid(cache)<<"\n" ;
    lck.unlock();
    return false;
  }
  else {
    unique_lock lck(mtx);
    Record new_record(uri, response, parse_time(cache),time(0));
    if (record_lib.size() == 1000) {
      record_lib.erase(record_lib.begin());
    }
    //cout<<"insert"<<endl;
    record_lib.insert(pair<string, Record>(uri, new_record));
    lck.unlock();
    if (new_record.expire_time<=time(0)){
      std::unique_lock<std::mutex> lck (LOG.mtx);
      LOG<<rid<<": cached, but requires re-validation\n";
      lck.unlock();
    }
    else{
      std::unique_lock<std::mutex> lck (LOG.mtx);
      LOG<<rid<<": cached, expires at "<< asctime(gmtime(&new_record.expire_time));
      lck.unlock();
    }
    return true;
  }
}

bool Cache::check_time_valid(string uri) {
  shared_lock lck(mtx);
  Record r = record_lib.at(uri);
  time_t now = time(0);
  if ((difftime(r.get_expire(), now)) > 0) {
    return true;
  }
  else {
    return false;
  }
}

bool Cache::check_tag_valid(string uri) {
  shared_lock lck(mtx);
  Record r = record_lib.at(uri);
  RspCacheControl cache = (r.http_response).get_cache();
  if (cache.etag.empty() and cache.last_modified.empty()) {
    return false;
  }
  else {
    return true;
  }
}

void Cache::revalidate(string uri, const HttpResponse & rsp,size_t rid) {
  //cout<<"start remove"<<endl;
  remove_record(uri);
  //cout<<"start store"<<endl;
  store_record(uri, rsp,rid);
}

time_t Cache::get_store_time(string uri){
  shared_lock lck(mtx);
  Record r = record_lib.at(uri);
  time_t now = time(0);
  return now-r.store_time;
}

time_t Cache::get_expire_time(string uri){
  shared_lock lck(mtx);
  Record r = record_lib.at(uri);
  return r.expire_time;
}

