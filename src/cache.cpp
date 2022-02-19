#include "cache.h"
using namespace std;


//search
//expire
//revalid
//send back

Record::Record(string uri,HttpResponse response){
    URI=uri;
    http_response=response;
    
}

HttpResponse Record::get_response(){
    return http_response;
}

//return 0 if could not find, else check the status of record
string Cache::search_record(string uri){
    map<string,Record>::iterator it;
    it=record_lib.find(uri);
    if (it!=record_lib.end()){
        return "";//check(record_lib.at(uri));
    }
    else{
        return "not in cache";
    }
}

HttpResponse Cache::send_response(string uri){
    return record_lib.at(uri).get_response();
}

void Cache::remove_record(string uri){
    map<string,Record>::iterator it;
    it=record_lib.find(uri);
    if (it!=record_lib.end()){
        record_lib.erase(it);
    }
}

void Cache::store_record(string uri,HttpResponse response){
    Record new_record(uri,response);
    record_lib.insert(pair<string,Record>(uri,new_record));
}



//to do
/**
 * 1.cache-control in different type
 * 3.revalid
 * /