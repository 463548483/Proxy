#include "gtest/gtest.h"
#include "http_request.h"
#include "http_response.h"
#include "http_parser.h"
#include <cstring>
#include <string> 

TEST(http_request_test, POST) {
  const char * input = "POST  http://w3schools.com:a8/test/demo_form.php  HTTP/1.1\r\nHost : abc.com\r\nCache-Control  :no-cache, no-store \r\n\r\nname1=value1&name2=value2"; 
  const char * request_line = "POST  http://w3schools.com:a8/test/demo_form.php  HTTP/1.1";
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
  const char * input = "HTTP/1.1   200   OK\r\nHost : abc.com\r\nCache-Control  :no-cache, no-store \r\n\r\nname1=value1&name2=value2"; 
  const char * rsp_line = "HTTP/1.1   200   OK";
  const char * header1 = "Host: abc.com";
  const char * header2 = "Cache-Control:no-cache, no-store ";
  const char * rsp_body = "name1=value1&name2=value2";
  size_t size = strlen(input) + 1;
  HttpParser parser;
  HttpResponse rsp = parser.parse_response(input, size);
  std::vector<char> start_line = rsp.get_start_line();
  start_line.push_back('\0');
  EXPECT_STREQ(start_line.data(), rsp_line);
  EXPECT_EQ(rsp.get_start_line().size(), strlen(rsp_line));
  EXPECT_EQ(rsp.get_header().size(), 2);
  std::vector<char> header_0 = rsp.get_header()[0];
  header_0.push_back('\0');
  EXPECT_STREQ(header_0.data(), header1);
  EXPECT_EQ(rsp.get_header()[0].size(), strlen(header1));
  std::vector<char> header_1 = rsp.get_header()[1];
  header_1.push_back('\0');
  EXPECT_STREQ(header_1.data(), header2);
  EXPECT_EQ(rsp.get_header()[1].size(), strlen(header2));
  EXPECT_STREQ(rsp.get_message_body().data(), rsp_body);
  EXPECT_EQ(rsp.get_message_body().size(), strlen(rsp_body) + 1);
  EXPECT_EQ(rsp.get_code(), std::string("200"));
}



int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv); 
  return RUN_ALL_TESTS();
}
