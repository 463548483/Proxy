#ifndef HTTP_BASE_H
#define HTTP_BASE_H

#include <vector>
#include <string>

#include "cache_control.h"

class HttpParser;

class HttpBase {
  friend class HttpParser;
public:
  void resize_message_body(long long size);
  // request is the reconstructed request not raw request
  std::vector<char> reconstruct() const;
  std::vector<char> get_start_line() const {return start_line;}
  std::vector<std::vector<char>> get_header() const {return header_fields;}
  std::vector<char> get_message_body() const {return message_body;}
  // no sanity check, have to guarentee field have correct format
  // not change any field tags specific to request or response
  // Do not add \r\n in the end
  // Correct Eg. add_header_field(std::string("NewField: new"));
  void add_header_field(const std::string & field);
protected:
  // excludes '\0' in all vectors
  std::vector<char> start_line;
  std::vector<std::vector<char>> header_fields;
  std::vector<char> message_body;
};

#endif
