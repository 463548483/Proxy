#include <iostream>
#include <ctime>
#include <map>
#include "cache.h"
using namespace std;

//search
//expire
//revalid
//send back

Record::Record(string url,string response,struct tm time){
    URL=url;
    HTTP_response=response;
    expire_time=time;
}

string Record::get_response(){
    return HTTP_response;
}

//return 0 if could not find, else check the status of record
string Cache::search(string url){
    map<string,Record>::iterator it;
    it=record_lib.find(url);
    if (it!=record_lib.end()){
        return check(record_lib.at(url));
    }
    else{
        return "not in cache";
    }
}

string Cache::response(string url){
    return record_lib.at(url).get_response();
}

void Cache::remove(string url){
    map<string,Record>::iterator it;
    it=record_lib.find(url);
    if (it!=record_lib.end()){
        record_lib.erase(it);
    }
}


int main(){

}

//to do
/**
 * 1.cache-control in different type
 * 2.receive/store type
 * 3.revalid
 * /