#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>

class HttpRequestExc : public std::exception {
  virtual const char * what() const throw() {
    return "Invalid http request, should reply with 400\n";
  }
};

class HttpResponseExc : public std::exception {
  virtual const char * what() const throw() {
    return "Invalid http response, should reply with 502\n";
  }
};

// This is for internal use in http_parse only
class HttpParserExc: public std::exception {
  virtual const char * what() const throw() {
    return "Encounter an error when parsing headers fields in http\n";
  }
};
#endif
