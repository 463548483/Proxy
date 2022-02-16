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
private:
  std::string code;
  RspCacheControl cache;
};

#endif
