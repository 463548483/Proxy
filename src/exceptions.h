#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

class HttpRequestExc : public std::exception {
public:
  HttpRequestExc(std::string msg) {
    err_msg = "Invalid http request, should reply with 400\n" + msg + "\n";
  }
  virtual const char * what() const throw() {
    return err_msg.c_str();
  }
private:
  std::string err_msg;
};

class HttpResponseExc : public std::exception {
public:
  HttpResponseExc(std::string msg) {
    err_msg = "Invalid http response, should reply with 502\n" + msg + "\n";
  }
  virtual const char * what() const throw() {
    return err_msg.c_str();
  }
private:
  std::string err_msg;
};

// This is for internal use in http_parse only
class HttpParserExc: public std::exception {
public:
  HttpParserExc(std::string msg) {
    err_msg = "Encounter an error when parsing headers fields in http\n" + msg + "\n";
  }
  virtual const char * what() const throw() {
    return err_msg.c_str();
  }
private:
  std::string err_msg;
};

class SocketExc : public std::exception {
  private:
    std::string error;
  public:
  SocketExc(std::string s):error(s){}
  virtual const char * what() const throw() {
    return error.c_str();
  }
};


#endif
