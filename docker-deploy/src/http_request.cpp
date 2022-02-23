#include "http_request.h"

std::vector<char> HttpRequest::get_revalidate_req(const std::string& URI, 
    const std::string& etag, const std::string& last_modified) const {
  std::vector<char> res;
  std::string start_line = "GET " + URI + " HTTP/1.1";
  res.insert(res.end(), start_line.begin(), start_line.end());
  res.push_back('\r');
  res.push_back('\n');
  std::string host;
  // should be absolute form
  // but can also parse authority form
  size_t loc1 = URI.find("//");
  if (loc1 != std::string::npos) {
    size_t loc2 = URI.find("/", loc1 + 2);
    host = URI.substr(loc1 + 2, loc2 - loc1 - 2);
  } else {
    host = URI;
  }
  std::string field_host = "Host: " + host;
  res.insert(res.end(), field_host.begin(), field_host.end());
  res.push_back('\r');
  res.push_back('\n');
  if (!etag.empty()) {
    std::string field_etag = "If-None-Match: " + etag;
    res.insert(res.end(), field_etag.begin(), field_etag.end());
    res.push_back('\r');
    res.push_back('\n');
  }
  if (!last_modified.empty()) {
    std::string field_last_modified = "If-Modified-Since: " + last_modified;
    res.insert(res.end(), field_last_modified.begin(), field_last_modified.end());
    res.push_back('\r');
    res.push_back('\n');
  }
  res.push_back('\r');
  res.push_back('\n');
  return res;
}
