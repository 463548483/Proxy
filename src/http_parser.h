#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <vector>
#include <string>
#include <algorithm>

#include "http_request.h"
#include "http_response.h"
#include "exceptions.h"

class HttpBase;
class HttpRequest;
class HttpResponse;

class HttpParser {
public:
  // these functions will only throw HttpRequestExc/HttpResponseExc
  HttpRequest parse_request(const char * input, size_t size);
  HttpResponse parse_response(const char * input, size_t size);
private:
  // parse input in to HttpBase
  void parse_raw(const char * input, size_t size, HttpBase* message);
  void parse_request_line(HttpRequest* request);
  void parse_response_line(HttpResponse* reqponse);
  // check has "Host", edit "Host" field, parse cache field
  void parse_request_headers(HttpRequest* request);
  // check Content_Length, parse cache field
  void parse_response_headers(HttpResponse* response);
  // parse header into lines of header fields
  void parse_header_fields(const std::vector<char>* header, std::vector<std::vector<char>>* header_fields);
  // will correct whitespace error,  otherwise throw exp
  // must have ":", can not start with " " or "\t"
  void sanity_check_header_field(std::vector<char> * header_field);
  // return number of fields with the same field_name. field is the first field with the name
  size_t get_header_field(const std::string& field_name, 
      std::vector<std::vector<char>>* header_fields, std::vector<char>** field);
  std::string to_lower_case(const std::string& str);
  void parse_req_cache(HttpRequest* request);
  void parse_rsp_cache(HttpResponse* response);
};

#endif
