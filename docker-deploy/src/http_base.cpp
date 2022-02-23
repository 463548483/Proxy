#include "http_base.h"

std::vector<char> HttpBase::reconstruct() const {
  std::vector<char> res;
  res.insert(res.end(), start_line.begin(), start_line.end());
  res.push_back('\r');
  res.push_back('\n');
  for (size_t i = 0; i < header_fields.size(); ++i) {
    res.insert(res.end(), header_fields[i].begin(), header_fields[i].end());
    res.push_back('\r');
    res.push_back('\n');
  }
  res.push_back('\r');
  res.push_back('\n');
  if (message_body.size() != 0) {
    res.insert(res.end(), message_body.begin(), message_body.end());
  }
  //std::string temp_res(res.begin(), res.end());
  //std::cout << "Reconstructed request: \n" << temp_res << std::endl;
  return res;
}

void HttpBase::add_header_field(const std::string & field) {
  header_fields.push_back(std::vector<char>(field.begin(), field.end()));
}

void HttpBase::resize_message_body(long long size) {
  if (size < 0 || size > (long long )message_body.size()) {
    return;
  }
  message_body.resize(size);
}
