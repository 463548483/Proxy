#include "http_parser.h"

// size should exclude '\0', aka equals to strlen(input)
HttpRequest HttpParser::parse_request(const char * input, size_t size) {
  HttpRequest req;
  try {
    parse_raw(input, size, &req);
    parse_request_line(&req);
    parse_request_headers(&req);
  }
  catch (HttpParserExc &e) {
    throw HttpRequestExc(); 
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
    throw HttpResponseExc();
  }
  return rsp;
}



void HttpParser::parse_raw(const char * input, size_t size, HttpBase* msg) {
  std::vector<char> pattern1{'\r', '\n'};
  std::vector<char> pattern2{'\r', '\n', '\r', '\n'};
  std::vector<char> msg_raw = std::vector<char>(input, input + size);
  auto iter1 = std::search(msg_raw.begin(), msg_raw.end(), pattern1.begin(), pattern1.end());
  if (iter1 == msg_raw.end()) {
    throw HttpParserExc(); 
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
    throw HttpRequestExc();
  }
  std::string method = std::string(request->start_line.begin(), iter1);
  // can only handle GET POST and CONNECT
  if (method != "GET" && method != "POST" && method != "CONNECT") {
    throw HttpRequestExc();
  }
  request->method = method;
  auto iter2 = std::find(iter1 + 1, request->start_line.end(), ' ');
  while(iter2 != request->start_line.end() && iter2 == iter1 + 1) {
    iter1 = iter2;
    iter2 = std::find(iter1 + 1, request->start_line.end(), ' ');
  }
  if (iter2 == request->start_line.end()) {
    throw HttpRequestExc();
  }
  request->URI = std::string(iter1 + 1, iter2);
  // parse URI
  // absolute-form
  std::string URI = request->URI;
  if (request->method == "GET" || request->method == "POST") {
    size_t loc1 = URI.find("//");
    if (loc1 == std::string::npos) {
      throw HttpRequestExc();
    }
    size_t loc2 = URI.find("/", loc1 + 2);
    if (loc2 == std::string::npos) {
      throw HttpRequestExc();
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
    throw HttpResponseExc();
  }
  auto iter2 = std::find(iter1 + 1, response->start_line.end(), ' ');
  while (iter2 != response->start_line.end() && iter2 == iter1 + 1) {
    iter1 = iter2;
    iter2 = std::find(iter1 + 1, response->start_line.end(), ' ');
  }
  if (iter2 == response->start_line.end()) {
    throw HttpResponseExc();
  }
  std::string code = std::string(iter1 + 1, iter2);
  // status code should be three digits
  if (code.size() != 3 || 
      code[0] < '0' || code[0] > '9' ||
      code[1] < '0' || code[1] > '9' ||
      code[2] < '0' || code[2] > '9') {
    throw HttpResponseExc();
  } else {
    response->code = code;
  }
}

void HttpParser::parse_request_headers(HttpRequest* request) {
  return;
}

void HttpParser::parse_response_headers(HttpResponse* response) {
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
    throw HttpParserExc();
  }
  // start with ' 'or '\t' => obs-fold => invalid format
  if (header_field->size() > 0 && (header_field->at(0) == ' ' || header_field->at(0) == '\t')) {
    throw HttpParserExc();
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

bool HttpParser::get_header_field(const std::vector<char>& field_name,
    std::vector<std::vector<char>>* header_fields,
    std::vector<char>* field) {
  for (int i = 0; i < header_fields->size(); ++i) {
    auto iter = std::search(header_fields->at(i).begin(), 
                            header_fields->at(i).end(),
                            field_name.begin(),
                            field_name.end());
    if (iter != header_fields->at(i).end()) {
      field = &(header_fields->at(i));
      return true;
    }
  }
  return false;
}

void HttpParser::parse_req_cache(const std::vector<char>* field, ReqCacheControl* cache) {
}

void HttpParser::parse_rsp_cache(const std::vector<char>* field, RspCacheControl* cache) {
}
