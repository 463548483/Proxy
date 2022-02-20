#include "gtest/gtest.h"
#include "http_request.h"
#include "http_response.h"
#include "http_parser.h"
#include <cstring>
#include <string> 
#include <fstream>

TEST(http_request_test, POST) {
  const char * input = "POST  http://w3schools.com:a8/test/demo_form.php  HTTP/1.1\r\nHost : abc.com\r\nCache-Control  :no-cache, no-store \r\n\r\nname1=value1&name2=value2"; 
  const char * request_line = "POST  http://w3schools.com:a8/test/demo_form.php  HTTP/1.1";
  const char * header1 = "Host: w3schools.com:80";
  const char * header2 = "Cache-Control:no-cache, no-store ";
  const char * request_body = "name1=value1&name2=value2";
  size_t size = strlen(input) + 1;
  HttpParser parser;
  HttpRequest req = parser.parse_request(input, size);
  std::vector<char> start_line = req.get_start_line();
  start_line.push_back('\0');
  EXPECT_STREQ(start_line.data(), request_line);
  EXPECT_EQ(req.get_start_line().size(), strlen(request_line));
  EXPECT_EQ(req.get_header().size(), 2);
  std::vector<char> header_0 = req.get_header()[0];
  header_0.push_back('\0');
  EXPECT_STREQ(header_0.data(), header1);
  EXPECT_EQ(req.get_header()[0].size(), strlen(header1));
  std::vector<char> header_1 = req.get_header()[1];
  header_1.push_back('\0');
  EXPECT_STREQ(header_1.data(), header2);
  EXPECT_EQ(req.get_header()[1].size(), strlen(header2));
  EXPECT_STREQ(req.get_message_body().data(), request_body);
  EXPECT_EQ(req.get_message_body().size(), strlen(request_body) + 1);
  EXPECT_EQ(req.get_host(), std::string("w3schools.com"));
  EXPECT_EQ(req.get_port(), std::string("80"));
  EXPECT_EQ(req.get_URI(), std::string("http://w3schools.com:a8/test/demo_form.php"));
  EXPECT_EQ(req.get_method(), std::string("POST"));
}

TEST(http_request_test, CONNECT) {
  const char * input = "CONNECT w3schools.com:82/  HTTP/1.1\r\nHost : abc.com\r\nCache-Control  :no-cache, no-store \r\n\r\nname1=value1&name2=value2"; 
  const char * request_line = "CONNECT w3schools.com:82/  HTTP/1.1";
  const char * header1 = "Host: abc.com";
  const char * header2 = "Cache-Control:no-cache, no-store ";
  const char * request_body = "name1=value1&name2=value2";
  size_t size = strlen(input) + 1;
  HttpParser parser;
  HttpRequest req = parser.parse_request(input, size);
  std::vector<char> start_line = req.get_start_line();
  start_line.push_back('\0');
  EXPECT_STREQ(start_line.data(), request_line);
  EXPECT_EQ(req.get_start_line().size(), strlen(request_line));
  EXPECT_EQ(req.get_header().size(), 2);
  std::vector<char> header_0 = req.get_header()[0];
  header_0.push_back('\0');
  EXPECT_STREQ(header_0.data(), header1);
  EXPECT_EQ(req.get_header()[0].size(), strlen(header1));
  std::vector<char> header_1 = req.get_header()[1];
  header_1.push_back('\0');
  EXPECT_STREQ(header_1.data(), header2);
  EXPECT_EQ(req.get_header()[1].size(), strlen(header2));
  EXPECT_STREQ(req.get_message_body().data(), request_body);
  EXPECT_EQ(req.get_message_body().size(), strlen(request_body) + 1);
  EXPECT_EQ(req.get_host(), std::string("w3schools.com"));
  EXPECT_EQ(req.get_port(), std::string("82"));
  EXPECT_EQ(req.get_URI(), std::string("w3schools.com:82/"));
  EXPECT_EQ(req.get_method(), std::string("CONNECT"));
}

TEST(http_response_test, test0) {
  const char * input = "HTTP/1.1   200   OK\r\nHost : abc.com\r\nCache-Control  :no-cache, no-store \r\ncontent-length: 026e \r\n\r\nname1=value1&name2=value2"; 
  const char * rsp_line = "HTTP/1.1   200   OK";
  const char * header1 = "Host: abc.com";
  const char * header2 = "Cache-Control:no-cache, no-store ";
  const char * header3 = "content-length: 26";
  const char * rsp_body = "name1=value1&name2=value2";
  size_t size = strlen(input) + 1;
  HttpParser parser;
  HttpResponse rsp = parser.parse_response(input, size);
  std::vector<char> start_line = rsp.get_start_line();
  start_line.push_back('\0');
  EXPECT_STREQ(start_line.data(), rsp_line);
  EXPECT_EQ(rsp.get_start_line().size(), strlen(rsp_line));
  EXPECT_EQ(rsp.get_header().size(), 3);
  std::vector<char> header_0 = rsp.get_header()[0];
  header_0.push_back('\0');
  EXPECT_STREQ(header_0.data(), header1);
  EXPECT_EQ(rsp.get_header()[0].size(), strlen(header1));
  std::vector<char> header_1 = rsp.get_header()[1];
  header_1.push_back('\0');
  EXPECT_STREQ(header_1.data(), header2);
  EXPECT_EQ(rsp.get_header()[1].size(), strlen(header2));
  std::vector<char> header_2 = rsp.get_header()[2];
  header_2.push_back('\0');
  EXPECT_STREQ(header_2.data(), header3);
  EXPECT_EQ(rsp.get_header()[2].size(), strlen(header3));
  EXPECT_STREQ(rsp.get_message_body().data(), rsp_body);
  EXPECT_EQ(rsp.get_message_body().size(), strlen(rsp_body) + 1);
  EXPECT_EQ(rsp.get_code(), std::string("200"));
}

TEST(http_response_test, real_data) {
  std::ifstream is ("Http_parse_data.txt", std::ifstream::binary);
  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);
  char * input = new char [length];
  is.read (input,length);
  is.close();
  const char * rsp_line = "HTTP/1.1 200 OK";
  size_t size = length;
  HttpParser parser;
  HttpResponse rsp = parser.parse_response(input, size);
  std::vector<char> start_line = rsp.get_start_line();
  start_line.push_back('\0');
  EXPECT_STREQ(start_line.data(), rsp_line);
  EXPECT_EQ(rsp.get_start_line().size(), strlen(rsp_line));
  EXPECT_EQ(rsp.get_header().size(), 7);
  EXPECT_EQ(rsp.get_code(), std::string("200"));
}

TEST(http_base_test, reconstruct_and_add_line) {
  const char * input = "POST  http://w3schools.com:a8/test/demo_form.php  HTTP/1.1\r\nHost : abc.com\r\nCache-Control  :no-cache, no-store \r\n\r\nname1=value1&name2=value2"; 
  size_t size = strlen(input);
  const char * output = "POST  http://w3schools.com:a8/test/demo_form.php  HTTP/1.1\r\nHost: w3schools.com:80\r\nCache-Control:no-cache, no-store \r\nNewField: add new\r\n\r\nname1=value1&name2=value2"; 
  HttpParser parser;
  HttpRequest req = parser.parse_request(input, size);
  std::string field = "NewField: add new";
  req.add_header_field(field);
  std::vector<char> rec_req = req.reconstruct();
  size_t rec_size = rec_req.size();
  rec_req.push_back('\0');
  EXPECT_STREQ(rec_req.data(), output);
  EXPECT_EQ(rec_size, strlen(output));
}

TEST(http_request_test, get_revalidate_req) {
  const char * output = "GET http://w3schools.com:a8/test/demo_form.php HTTP/1.1\r\nHost: w3schools.com:a8\r\nIf-None-Match: abc\r\nIf-Modified-Since: def\r\n\r\n"; 
  std::string URI = "http://w3schools.com:a8/test/demo_form.php";
  std::string etag = "abc";
  std::string last_modified = "def";
  HttpRequest req;
  std::vector<char> output_vec = req.get_revalidate_req(URI, etag, last_modified);
  size_t size_vec = output_vec.size();
  output_vec.push_back('\0');
  EXPECT_STREQ(output_vec.data(), output);
  EXPECT_EQ(size_vec, strlen(output));
}

TEST(http_response_test, replace_header_fields) {
  const char * input1 = "HTTP/1.1   200   OK\r\nHost : abc.com\r\nCache-Control  :no-cache, no-store \r\ncontent-length: 025e \r\n\r\nname1=value1&name2=value2"; 
  const char * input2 = "HTTP/1.1   305   OK\r\nHost : def.com\r\nCache  :no-cache, no-store \r\n\r\nname1=value1";
  const char * output = "HTTP/1.1   200   OK\r\nHost: def.com\r\nCache:no-cache, no-store \r\n\r\nname1=value1&name2=value2";
  HttpParser parser;
  HttpResponse rsp1 = parser.parse_response(input1, strlen(input1));
  HttpResponse rsp2 = parser.parse_response(input2, strlen(input2));
  rsp1.replace_header_fields(&rsp2);
  std::vector<char> out_vec = rsp1.reconstruct();
  size_t size_vec = out_vec.size();
  out_vec.push_back('\0');
  EXPECT_STREQ(out_vec.data(), output);
  EXPECT_EQ(size_vec, strlen(output));
}

TEST(http_request_test, parse_cache_ctl) {
  const char * input1 = "GET http://w3schools.com:a8/test/demo_form.php HTTP/1.1\r\nHost : abc.com\r\nCache-Control  :no-cache, no-store \r\n\r\nname1=value1&name2=value2"; 
  const char * input2 = "GET http://w3schools.com:a8/test/demo_form.php HTTP/1.1\r\nHost : abc.com\r\nCache-Control  : \r\ncontent-length: 025e \r\n\r\nname1=value1&name2=value2"; 
  const char * input3 = "GET http://w3schools.com:a8/test/demo_form.php HTTP/1.1\r\nHost : abc.com\r\nCache-Control  :no-cache \r\ncache-control: no-store \r\ncontent-length: 025e \r\n\r\nname1=value1&name2=value2"; 
  const char * input4 = "GET http://w3schools.com:a8/test/demo_form.php HTTP/1.1\r\nHost : abc.com\r\nCache-Control  :no-cache \r\ncontent-length: 025e \r\ncache-control: no-store\r\n\r\nname1=value1&name2=value2"; 
  HttpParser parser;
  HttpRequest req1 = parser.parse_request(input1, strlen(input1));
  HttpRequest req2 = parser.parse_request(input2, strlen(input2));
  HttpRequest req3 = parser.parse_request(input3, strlen(input3));
  HttpRequest req4 = parser.parse_request(input4, strlen(input4));
  ReqCacheControl_type ctl1 = req1.get_cache();
  ReqCacheControl_type ctl2 = req2.get_cache();
  ReqCacheControl_type ctl3 = req3.get_cache();
  ReqCacheControl_type ctl4 = req4.get_cache();
  EXPECT_EQ(ctl1.no_store, true);
  EXPECT_EQ(ctl1.no_cache, true);
  EXPECT_EQ(ctl2.no_store, false);
  EXPECT_EQ(ctl2.no_cache, false);
  EXPECT_EQ(ctl3.no_store, true);
  EXPECT_EQ(ctl3.no_cache, true);
  EXPECT_EQ(ctl4.no_store, true);
  EXPECT_EQ(ctl4.no_cache, true);
}


TEST(http_response_test, parse_cache_ctl) {
  const char * input1 = "HTTP/1.1   200   OK\r\nHost : abc.com\r\nCache-Control  : no-store \r\ncontent-length: 025 \r\nCache-control: private, max-age=1230\r\n\r\nname1=value1&name2=value2"; 
  const char * input2 = "HTTP/1.1   200   OK\r\nHost : abc.com\r\nCache-Control  :no-cache \r\ncontent-length: 025 \r\nCache-control:  max-age=\"1230\"  \r\n\r\nname1=value1&name2=value2"; 
  const char * input3 = "HTTP/1.1   200   OK\r\nHost : abc.com\r\nExpires:  Sun, 06 Nov 1994 08:49:37 GMT\r\nCache-Control  :no-cache \r\ncontent-length: 025 \r\nCache-control:  max-age=\"zx1230\"  \r\n\r\nname1=value1&name2=value2"; 
  const char * input4 = "HTTP/1.1   200   OK\r\nHost : abc.com\r\nExpires:  Sun, 06 Nov 1994 8:49:37 GMT\r\nCache-Control  :no-cache \r\nAge: 432\a\r\ncontent-length: 025 \r\nCache-control:   \r\n\r\nname1=value1&name2=value2"; 
  HttpParser parser;
  HttpResponse req1 = parser.parse_response(input1, strlen(input1));
  HttpResponse req2 = parser.parse_response(input2, strlen(input2));
  HttpResponse req3 = parser.parse_response(input3, strlen(input3));
  HttpResponse req4 = parser.parse_response(input4, strlen(input4));
  RspCacheControl_type ctl1 = req1.get_cache();
  RspCacheControl_type ctl2 = req2.get_cache();
  RspCacheControl_type ctl3 = req3.get_cache();
  RspCacheControl_type ctl4 = req4.get_cache();
  EXPECT_EQ(ctl1.no_store, true);
  EXPECT_EQ(ctl1.no_cache, false);
  EXPECT_EQ(ctl1.is_private, true);
  EXPECT_EQ(ctl1.max_age, 1230);
  EXPECT_EQ(ctl1.has_expires, false);
  EXPECT_EQ(ctl1.expires, -1);
  EXPECT_EQ(ctl1.age, 0);
  EXPECT_EQ(ctl2.no_store, false);
  EXPECT_EQ(ctl2.no_cache, true);
  EXPECT_EQ(ctl2.is_private, false);
  EXPECT_EQ(ctl2.max_age, 1230);
  EXPECT_EQ(ctl2.has_expires, false);
  EXPECT_EQ(ctl2.expires, -1);
  EXPECT_EQ(ctl2.age, 0);
  EXPECT_EQ(ctl3.no_store, false);
  EXPECT_EQ(ctl3.no_cache, true);
  EXPECT_EQ(ctl3.is_private, false);
  EXPECT_EQ(ctl3.max_age, 0);
  EXPECT_EQ(ctl3.has_expires, true);
  EXPECT_STREQ(ctime(&(ctl3.expires)), "Sun Nov  6 08:49:37 1994\n");
  EXPECT_EQ(ctl3.age, 0);
  EXPECT_EQ(ctl4.no_store, false);
  EXPECT_EQ(ctl4.no_cache, true);
  EXPECT_EQ(ctl4.is_private, false);
  EXPECT_EQ(ctl4.max_age, -1);
  EXPECT_EQ(ctl4.has_expires, true);
  EXPECT_EQ(ctl4.expires, -1);
  EXPECT_EQ(ctl4.age, 432);


}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv); 
  return RUN_ALL_TESTS();
}
