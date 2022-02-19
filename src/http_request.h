#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <vector>
#include <string>

#include "cache_control.h"
#include "http_parser.h"
#include "exceptions.h"
#include "http_base.h"

class HttpRequest : public HttpBase {
  friend class HttpParser;
public:
  // add default constructor
  HttpRequest(): HttpBase() {}
  // this is only use for regenerating revalidation request
  // need to insure that etag and last_modified is not both empty(this case should not revalidate)
  std::vector<char> get_revalidate_req(const std::string& URI, const std::string& etag, const std::string& last_modified);
  std::string get_host() {return host;}
  std::string get_port() {return port;}
  std::string get_URI() {return URI;}
  std::string get_method() {return method;}
  ReqCacheControl get_cache() {return cache;}
private:
  // for "GET" & "POST"ignore the 'Host' field in headers and replace it by info from request-target
  // Struct of the request_line:
  // method host:port
  std::string host;
  std::string port;
  std::string URI;
  std::string method;
  ReqCacheControl cache;
};

#endif
