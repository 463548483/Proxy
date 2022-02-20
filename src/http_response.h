#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <vector>
#include <string>

#include "cache_control.h"
#include "http_parser.h"
#include "exceptions.h"
#include "http_base.h"

class HttpResponse : public HttpBase {
  friend class HttpParser;
public:
  std::string get_code() {return code;}
  RspCacheControl get_cache() {return cache;}
  // also need to replace cache_control accordingly
  void replace_header_fields(const HttpResponse* rsp);
private:
  std::string code;
  // only use this if code="200"
  RspCacheControl cache;
};

#endif
