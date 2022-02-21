#include "cache.h"
using namespace std;


Record::Record(string uri,HttpResponse response,time_t expire_time){
    URI=uri;
    http_response=response;
    this->expire_time=expire_time;

}

HttpResponse Record::get_response(){
    return http_response;
}

time_t Record::get_expire(){
    return expire_time;
}



//return 0 if could not find, else check the status of record
HttpResponse * Cache::search_record(string uri){
    unordered_map<string,Record>::iterator it;
    it=record_lib.find(uri);
    if (it!=record_lib.end()){
        Record r=record_lib.at(uri);
        return &r.get_response();
    }
    else{
        return NULL;
    }
}

HttpResponse Cache::send_response(string uri){
    return record_lib.at(uri).get_response();
}

void Cache::remove_record(string uri){
    unordered_map<string,Record>::iterator it;
    it=record_lib.find(uri);
    if (it!=record_lib.end()){
        record_lib.erase(it);
    }
}

void Cache::store_record(string uri,HttpResponse response){
    RspCacheControl cache=response.get_cache();
    if (!check_store_valid(cache)){
        return;
    }
    else{
        Record new_record(uri,response,parse_time(cache));
        if (record_lib.size()==1000){
            record_lib.erase(record_lib.begin());
        }
        record_lib.insert(pair<string,Record>(uri,new_record));
    }
}


bool check_store_valid(RspCacheControl cache){
    if (cache.is_private==true or cache.no_store==true or (cache.max_age==-1 and cache.has_expires==true and cache.expires==-1)){
        return false;
    }
    else{
        return true;
    }
}

time_t parse_time(RspCacheControl cache){
    if (cache.no_cache==true or cache.max_age==0){
        return time(0);
    }
    else if (cache.max_age<0 and cache.has_expires==false){
        return NULL;
    }
    else if (cache.max_age>0){
        return time(0)+((uint64_t)cache.max_age-cache.age);
    }
    else{
        return cache.expires;
    }
}

bool Cache::check_time_valid(string uri){
    Record r=record_lib.at(uri);
    time_t now=time(0);
    if ((difftime(r.get_expire(),now))>0){
        return true;
    }
    else{
        return false;
    }
}

bool Cache::check_tag_valid(string uri){
    Record r=record_lib.at(uri);
    RspCacheControl cache=r.get_response().get_cache();
    if (cache.etag.empty() and cache.last_modified.empty()){
        return false;
    }
    else{
        return true;
    }
}


