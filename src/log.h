#ifndef LOG_H
#define LOG_H

#include <string>
#include <fstream>
#include <mutex>
#include <ios>

// thread-save Logger class
class Logger {
public:
  Logger() {}
  Logger(const Logger &) = delete;
  void open(std::string f_name, std::ios_base::openmode mode = std::ios_base::out);
  Logger& operator = (const Logger &) = delete;
  Logger& operator << (const std::string & msg);
  Logger& operator << (const int & msg);
  Logger& operator << (const long long & msg);
  Logger& operator << (const unsigned int  & msg);
  Logger& operator << (const unsigned long long & msg);
  Logger& operator << (const size_t & msg);
  ~Logger();
  std::mutex mtx;
private:
  std::string file_name;
  std::ofstream fs;
};

#endif
