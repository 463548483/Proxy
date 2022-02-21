#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <vector>
#include <string>

#include "cache_control.h"
#include "http_parser.h"
#include "exceptions.h"
#include "http_base.h"

class HttpBase; 

class HttpResponse : public HttpBase {
  friend class HttpParser;
public:
  HttpResponse(): HttpBase() {}
  std::string get_code() const {return code;}
  RspCacheControl get_cache() const {return cache;}
  // also need to replace cache_control accordingly
  void replace_header_fields(const HttpResponse* rsp);
  // NOTICE: will not change other data(Cache-Control data...)
  bool change_header_field(const std::string & field_name, const std::string & field_value);
private:
  std::string code;
  // only use this if code="200"
  RspCacheControl cache;
};

#endif
