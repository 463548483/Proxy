#include <iostream>
#include <ctime>
#include <map>
using namespace std;

//expire
class Record {
  private:
  string URL;
  string HTTP_response;
  struct tm expire_time;
  public:
  Record(string url,string response,struct tm time);
  string get_response();

};

class Cache{
  private:
  map<string,Record> record_lib;
  public:
  Cache();
  int search();
  int store();
  void remove();
  string response();

};
