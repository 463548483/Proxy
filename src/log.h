#ifndef LOG_H
#define LOG_H

#include <string>
#include <fstream>
#include <mutex>
// thread-save Logger class
class Logger {
public:
  Logger(const std::string& fname): file_name(fname), fs(fname, std::ofstream::out | std::ofstream::app) {}
  Logger(const Logger &) = delete;
  Logger& operator = (const Logger &) = delete;
  Logger& operator << (const std::string & msg);
  ~Logger();
private:
  std::string file_name;
  std::ofstream fs;
  std::mutex mtx;
};

#endif
