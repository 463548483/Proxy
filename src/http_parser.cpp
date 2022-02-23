#include "http_parser.h"

#include <iostream>

// size should exclude '\0', aka equals to strlen(input)
HttpRequest HttpParser::parse_request(const char * input, size_t size) {
  HttpRequest req;
  try {
    parse_raw(input, size, &req);
    parse_request_line(&req);
    parse_request_headers(&req);
  }
  catch (HttpParserExc &e) {
    std::cout << e.what() << std::endl;
    throw HttpRequestExc(e.what()); 
  }
  catch (HttpRequestExc &e) {
    std::cout << e.what() << std::endl;
    throw;
  }
  catch (std::exception &e) {
    std::cout << "Unexpected exception when parsing request!\n";
    throw HttpRequestExc("Unecpected exception when parsing request!!"); 
  }
  return req;
}


HttpResponse HttpParser::parse_response(const char * input, size_t size) {
  HttpResponse rsp;
  try {
    parse_raw(input, size, &rsp);
    parse_response_line(&rsp);
    parse_response_headers(&rsp);
  }
  catch (HttpParserExc &e) {
    std::cout << e.what() << std::endl;
    throw HttpResponseExc(e.what());
  }
  catch (HttpResponseExc &e) {
    std::cout << e.what() << std::endl;
    throw;
  }
  catch (std::exception &e) {
    std::cout << "Unexpected exception when parsing response!\n";
    throw HttpResponseExc("Unexpected exception when parsing response!!\n"); 
  }
  return rsp;
}



void HttpParser::parse_raw(const char * input, size_t size, HttpBase* msg) {
  std::vector<char> pattern1{'\r', '\n'};
  std::vector<char> pattern2{'\r', '\n', '\r', '\n'};
  std::vector<char> msg_raw = std::vector<char>(input, input + size);
  auto iter1 = std::search(msg_raw.begin(), msg_raw.end(), pattern1.begin(), pattern1.end());
  if (iter1 == msg_raw.end()) {
    throw HttpParserExc("HttpParser::parse_raw: can not get start line"); 
  }
  msg->start_line = {msg_raw.begin(), iter1};
  auto iter2 = std::search(msg_raw.begin(), msg_raw.end(), pattern2.begin(), pattern2.end());
  std::vector<char> header = {iter1 + 2, iter2};
  parse_header_fields(&header, &(msg->header_fields));
  if (iter2 != msg_raw.end()) {
    msg->message_body = {iter2 + 4, msg_raw.end()};
  }
}

void HttpParser::parse_request_line(HttpRequest* request) {
  auto iter1 = std::find(request->start_line.begin(), request->start_line.end(), ' ');
  if (iter1 == request->start_line.end()) {
    throw HttpRequestExc("HttpParser::parse_request_line: Do not have the first blankspace in start line.");
  }
  std::string method = std::string(request->start_line.begin(), iter1);
  // can only handle GET POST and CONNECT
  if (method != "GET" && method != "POST" && method != "CONNECT") {
    std::string msg = "HttpParser::parse_request_line: Wrong methods: " + method;
    throw HttpRequestExc(msg);
  }
  request->method = method;
  auto iter2 = std::find(iter1 + 1, request->start_line.end(), ' ');
  while(iter2 != request->start_line.end() && iter2 == iter1 + 1) {
    iter1 = iter2;
    iter2 = std::find(iter1 + 1, request->start_line.end(), ' ');
  }
  if (iter2 == request->start_line.end()) {
    throw HttpRequestExc("HttpParser::parse_request_line: Do not have the second blankspace in start line.");
  }
  request->URI = std::string(iter1 + 1, iter2);
  // parse URI
  // absolute-form
  std::string URI = request->URI;
  if (request->method == "GET" || request->method == "POST") {
    size_t loc1 = URI.find("//");
    if (loc1 == std::string::npos) {
      throw HttpRequestExc("HttpParser::parse_request_line: do not have // for absolute-form.");
    }
    size_t loc2 = URI.find("/", loc1 + 2);
    if (loc2 == std::string::npos) {
      throw HttpRequestExc("HttpParser::parse_request_line: do not have / after url for absolute-form.");
    }
    std::string URL = URI.substr(loc1 + 2, loc2 - loc1 - 2);
    size_t loc3;
    if ((loc3 = URL.find(":")) != std::string::npos) {
      request->host = URL.substr(0, loc3);
      // port has to be a valid number
      std::string port = URL.substr(loc3 + 1);
      try {
        unsigned long port_num = std::stoul(port, nullptr);
        request->port = std::to_string(port_num);
      }
      catch (std::exception &e) {
        request->port = "80";
      }
    } else{
      request->host = URL;
      request->port = "80";
    }
  }
  // authority-form
  if (request->method == "CONNECT") {
    size_t loc = URI.find(":");
    if (loc == std::string::npos) {
      request->host = URI;
      request->port = "80";
    } else {
      request->host = URI.substr(0, loc);
      // port has to be valid
      std::string port = URI.substr(loc + 1);
      try {
        unsigned long port_num = std::stoul(port, nullptr);
        request->port = std::to_string(port_num);
      }
      catch (std::exception &e) {
        request->port = "80";
      }
    }
  }
}

void HttpParser::parse_response_line(HttpResponse* response) {
  auto iter1 = std::find(response->start_line.begin(), response->start_line.end(), ' ');
  if (iter1 == response->start_line.end()) {
    throw HttpResponseExc("HttpParser::parse_response_line: Do not have the first blankspace in start line.");
  }
  auto iter2 = std::find(iter1 + 1, response->start_line.end(), ' ');
  while (iter2 != response->start_line.end() && iter2 == iter1 + 1) {
    iter1 = iter2;
    iter2 = std::find(iter1 + 1, response->start_line.end(), ' ');
  }
  if (iter2 == response->start_line.end()) {
    throw HttpResponseExc("HttpParser::parse_response_line: Do not have the second blankspace in start line.");
  }
  std::string code = std::string(iter1 + 1, iter2);
  // status code should be three digits
  if (code.size() != 3 || 
      code[0] < '0' || code[0] > '9' ||
      code[1] < '0' || code[1] > '9' ||
      code[2] < '0' || code[2] > '9') {
    throw HttpResponseExc("HttpParser::parse_response_line: code is not a three digits number.");
  } else {
    response->code = code;
  }
}

// check has "Host", edit "Host" field, parse cache field
void HttpParser::parse_request_headers(HttpRequest* request) {
  size_t field_id = request->header_fields.size();
  if (get_header_field(std::string("Host"), &(request->header_fields), &field_id) == 0 || 
      field_id >= request->header_fields.size()) {
    // no "Host" field
    throw HttpRequestExc("HttpParser::parse_request_headers: no Host field.");
  }
  std::vector<char> * field = &(request->header_fields.at(field_id));
  // absolute-form URI, need to reconstruct "Host" field 
  if (request->method == "POST" || request->method == "GET") {
    field->resize(5);
    field->push_back(' ');
    field->reserve(7 + request->host.size() + request->port.size());
    for (size_t i = 0; i < request->host.size(); ++i) {
      field->push_back(request->host[i]);
    }
    field->push_back(':');
    for (size_t i = 0; i < request->port.size(); ++i) {
      field->push_back(request->port[i]);
    }
  }
  if (request->method == "GET") {
    parse_req_cache(request);
  }
}

// check Content_Length, parse cache field
void HttpParser::parse_response_headers(HttpResponse* response) {
  // code != 304
  // 1. if no transfer-encoding && if has content_length
  // 2. multi content_length => throw
  // 3. single content_length but not valid 1.not  >=0 2.a string of number =>throw
  // 4. valid number but not match => throw
  // 5. replace the valid number(in case the whole string is invalid)
  
  // if code=304, Content-Length might equal to the size of the original message body
  if (response->code != std::string("304")) {
    size_t field_id = response->header_fields.size();
    if (get_header_field(std::string("Transfer-Encoding"), &(response->header_fields), &field_id) == 0 ||
        field_id >= response->header_fields.size()) {
      field_id = response->header_fields.size();
      size_t num_fields = 0;
      if ((num_fields = get_header_field(std::string("Content-Length"), &(response->header_fields), &field_id)) > 0 
          && field_id < response->header_fields.size()) {
        // multi Content-Length field
        if (num_fields > 1) {
          throw HttpResponseExc("HttpParser::parse_response_headers: has multipul Content-Length field.");
        }
        std::vector<char> * field = &(response->header_fields.at(field_id));
        auto iter = std::find(field->begin(), field->end(), ':');
        if (iter == field->end()) {
          throw HttpResponseExc("HttpParser::parse_response_headers: field has wrong format, do not have ':'.");
        }
        std::string content_size_str = std::string(iter + 1, field->end());
        long long content_size = 0;
        try {
          content_size = std::stoll(content_size_str, nullptr);
        }
        catch (std::exception &e) {
          throw HttpResponseExc("HttpParser::parse_response_headers: have error when trying to convert the content size.");
        }
        //if (content_size < 0 || (unsigned int)content_size != response->message_body.size()) {
        if (content_size < 0 || (unsigned int)content_size > response->message_body.size()) {
          std::string msg = "HttpParser::parse_response_headers: invalid content size or doesn't match message body size. content size: "
              + std::to_string(content_size) + " body real size: " + std::to_string(response->message_body.size());
          throw HttpResponseExc(msg);
        }
        response->resize_message_body(content_size);
        field->resize(15);
        field->push_back(' ');
        std::string content_size_new = std::to_string(content_size);
        for (size_t i = 0; i < content_size_new.size(); ++i) {
          field->push_back(content_size_new[i]);
        }
      }
    }
  }
  if (response->code == "200") {
    parse_rsp_cache(response);
  }
}

void HttpParser::parse_header_fields(const std::vector<char>* header, std::vector<std::vector<char>>* header_fields) {
  std::vector<char> pattern{'\r', '\n'};
  auto iter = header->begin();
  while (iter < header->end()) {
    auto iter_end = std::search(iter, header->end(), pattern.begin(), pattern.end());
    header_fields->push_back(std::vector<char>({iter, iter_end}));
    sanity_check_header_field(&(header_fields->at(header_fields->size() - 1)));
    iter = iter_end + 2;
  }
}

void HttpParser::sanity_check_header_field(std::vector<char> * header_field) {
  std::vector<char>::const_iterator iter;
  // no ':' => not a valid field
  if ((iter = std::find(header_field->begin(), header_field->end(), ':')) == header_field->end()) {
    throw HttpParserExc("HttpParser::sanity_check_header_field: field has wrong format, no ':'.");
  }
  // start with ' 'or '\t' => obs-fold => invalid format
  if (header_field->size() > 0 && (header_field->at(0) == ' ' || header_field->at(0) == '\t')) {
    throw HttpParserExc("HttpParser::sanity_check_header_field: field has wrong format, obs-fold.");
  }
  // handle extra whitespaces
  iter--;
  while (iter >= header_field->begin()) {
    if (*iter == ' ') {
      header_field->erase(iter);
      iter--;
    } else {
      break;
    }
  }
}

size_t HttpParser::get_header_field(const std::string& field_name,
    const std::vector<std::vector<char>>* header_fields, size_t* field_id) {
  *field_id = header_fields->size();
  size_t num = 0;
  for (size_t i = 0; i < header_fields->size(); ++i) {
    // field_name is case insensitive
    std::string field_name_lower = to_lower_case(field_name);
    auto iter = std::find(header_fields->at(i).begin(), header_fields->at(i).end(), ':');
    if (iter == header_fields->at(i).end()) {
      throw HttpParserExc("HttpParser::get_header_field: field has wrong format, no ':'.");
    }
    std::string name(header_fields->at(i).begin(), iter);
    std::string name_lower = to_lower_case(name);
    if (field_name_lower == name_lower) {
      if (num == 0) {
        *field_id = i;
      }
      ++num;
    }
  }
  return num;
}

std::string HttpParser::to_lower_case(const std::string& str) {
  std::string res = str;
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] >= 'A' && str[i] <= 'Z') {
      res[i] = str[i] -'A' + 'a';
    }
  }
  return res;
}

// can only parese one cache-control: the first one
void HttpParser::parse_req_cache(HttpRequest* request) {
  std::string s1 = "no-store";
  std::string s2 = "no-cache";
  std::vector<char> nostore(s1.begin(), s1.end());
  std::vector<char> nocache(s2.begin(), s2.end());
  // Do not need to init, ReqCacheControl has a constructor
  size_t field_id = request->header_fields.size();
  std::vector<std::vector<char>> temp_vec;
  std::vector<std::vector<char>> * fields = &(request->header_fields);
  while (get_header_field("Cache-Control", fields, &field_id) != 0
      && field_id < fields->size()) {
    std::vector<char>* field = &(fields->at(field_id));
    auto iter1 = std::search(field->begin(), field->end(), nostore.begin(), nostore.end());
    auto iter2 = std::search(field->begin(), field->end(), nocache.begin(), nocache.end());
    if (iter1 != field->end()) {
      request->cache.no_store = true;
    }
    if (iter2 != field->end()) {
      request->cache.no_cache = true;
    }
    // if already reach the last element in header_fields
    if (field_id >= fields->size() - 1) {
      break;
    }
    temp_vec = std::vector<std::vector<char>>(fields->begin() + field_id + 1, fields->end());
    fields = &temp_vec;
    field_id = fields->size();
  }
}

void HttpParser::parse_rsp_cache(HttpResponse* response) {
  std::string s1 = "no-store";
  std::string s2 = "no-cache";
  std::string s3 = "private";
  std::string s4 = "max-age=";
  std::vector<char> nostore(s1.begin(), s1.end());
  std::vector<char> nocache(s2.begin(), s2.end());
  std::vector<char> isprivate(s3.begin(), s3.end());
  std::vector<char> maxage(s4.begin(), s4.end());
  // Do not need to init, RspCacheControl has a constructor
  size_t field_id = response->header_fields.size();
  std::vector<std::vector<char>> temp_vec;
  std::vector<std::vector<char>> * fields = &(response->header_fields);
  // parse Cache-Control
  while (get_header_field("Cache-Control", fields, &field_id) != 0
      && field_id < fields->size()) {
    std::vector<char>* field = &(fields->at(field_id));
    auto iter1 = std::search(field->begin(), field->end(), nostore.begin(), nostore.end());
    auto iter2 = std::search(field->begin(), field->end(), nocache.begin(), nocache.end());
    auto iter3 = std::search(field->begin(), field->end(), isprivate.begin(), isprivate.end());
    auto iter4 = std::search(field->begin(), field->end(), maxage.begin(), maxage.end());
    if (iter1 != field->end()) {
      response->cache.no_store = true;
    }
    if (iter2 != field->end()) {
      response->cache.no_cache = true;
    }
    if (iter3 != field->end()) {
      response->cache.is_private = true;
    }
    if (iter4 != field->end()) {
      std::string max_age = std::string(iter4 + 8, field->end());
      if (!max_age.empty() && max_age[0] == '\"') {
        max_age = std::string(iter4 + 9, field->end());
      }
      try {
        int64_t max_age_num = std::stoll(max_age, nullptr);
        if (max_age_num < 0) {
          response->cache.max_age = 0;
        } else if (response->cache.max_age == -1 || 
            (response->cache.max_age != -1 && max_age_num < response->cache.max_age)){
          response->cache.max_age = max_age_num;
        }
      }
      catch (std::exception &e) {
        response->cache.max_age = 0;
      }
    }
    // if already reach the last element in header_fields
    if (field_id >= fields->size() - 1) {
      break;
    }
    temp_vec = std::vector<std::vector<char>>(fields->begin() + field_id + 1, fields->end());
    fields = &temp_vec;
    field_id = fields->size();
  }
  // parse fields related to cache 
  fields = &(response->header_fields);
  field_id = fields->size();
  while (get_header_field("Age", fields, &field_id) != 0
      && field_id < fields->size()) {
    std::vector<char>* field = &(fields->at(field_id));
    auto iter = std::find(field->begin(), field->end(), ':');
    if (iter == field->end()) {
      throw HttpResponseExc("HttpParser::parse_rsp_cache: Wrong format, do not have : for Age Field.");
    }
    std::string age_str = std::string(iter + 1, field->end());
    try {
      uint64_t age_num = std::stoull(age_str, nullptr);
      if (response->cache.age < age_num) {
        response->cache.age = age_num;
      }
    }
    catch (std::exception &e) {
    }
    // if already reach the last element in header_fields
    if (field_id >= fields->size() - 1) {
      break;
    }
    temp_vec = std::vector<std::vector<char>>(fields->begin() + field_id + 1, fields->end());
    fields = &temp_vec;
    field_id = fields->size();
  }

  fields = &(response->header_fields);
  field_id = fields->size();
  while (get_header_field("Expires", fields, &field_id) != 0
      && field_id < fields->size()) {
    std::vector<char>* field = &(fields->at(field_id));
    auto iter = std::find(field->begin(), field->end(), ':');
    if (iter == field->end()) {
      throw HttpResponseExc("HttpParser::parse_rsp_cache: Wrong format, do not have : for Expires Field.");
    }
    response->cache.has_expires = true;
    std::string expires_str = std::string(iter + 1, field->end());
    while (!expires_str.empty() && expires_str[0] == ' ') {
      expires_str.erase(0, 1);
    }
    // Can only parse certain format
    struct tm expires_time;
    const char * input_ptr = expires_str.c_str();
    char * ret_ptr = strptime(input_ptr, "%a, %d %b %Y %H:%M:%S" , &expires_time);
    // consider successfully parsed
    if (ret_ptr == input_ptr + 25) {
      time_t time = mktime(&expires_time);
      if (time != -1 && 
          (response->cache.expires == -1 ||
          (response->cache.expires != -1 && response->cache.expires > time))) {
        response->cache.expires = time;  
      }
    }
    // if already reach the last element in header_fields
    if (field_id >= fields->size() - 1) {
      break;
    }
    temp_vec = std::vector<std::vector<char>>(fields->begin() + field_id + 1, fields->end());
    fields = &temp_vec;
    field_id = fields->size();
  }
  
  fields = &(response->header_fields);
  field_id = fields->size();
  if (get_header_field("Etag", fields, &field_id) != 0
      && field_id < fields->size()) {
    std::vector<char>* field = &(fields->at(field_id));
    auto iter = std::find(field->begin(), field->end(), ':');
    if (iter == field->end()) {
      throw HttpResponseExc("HttpParser::parse_rsp_cache: Wrong format, do not have : for Etag Field.");
    }
    response->cache.etag = std::string(iter + 1, field->end());
  }

  fields = &(response->header_fields);
  field_id = fields->size();
  if (get_header_field("Last-Modified", fields, &field_id) != 0
      && field_id < fields->size()) {
    std::vector<char>* field = &(fields->at(field_id));
    auto iter = std::find(field->begin(), field->end(), ':');
    if (iter == field->end()) {
      throw HttpResponseExc("HttpParser::parse_rsp_cache: Wrong format, do not have : for Last-Modified Field.");
    }
    response->cache.last_modified = std::string(iter + 1, field->end());
  }
}
