#include "log.h"
#include <iostream>

void Logger::open(std::string f_name, std::ios_base::openmode mode) {
   file_name= f_name;
   fs.open(file_name.c_str(), mode);
   if (!fs.is_open()) {
    std::cerr << "Can not open log file!\n";
   }
}

Logger& Logger::operator << (const std::string& msg) {
  if (fs.is_open()) {
    fs << msg;
    fs.flush();
  }
  return *this;
}

Logger& Logger::operator << (const long long & msg) {
  if (fs.is_open()) {
    fs << msg;
    fs.flush();
  }
  return *this;
}

Logger& Logger::operator << (const int & msg) {
  if (fs.is_open()) {
    fs << msg;
    fs.flush();
  }
  return *this;
}

Logger& Logger::operator << (const unsigned long long & msg) {
  if (fs.is_open()) {
    fs << msg;
    fs.flush();
  }
  return *this;
}

Logger& Logger::operator << (const unsigned int & msg) {
  if (fs.is_open()) {
    fs << msg;
    fs.flush();
  }
  return *this;
}

Logger& Logger::operator << (const size_t & msg) {
  if (fs.is_open()) {
    fs << msg;
    fs.flush();
  }
  return *this;
}

Logger::~Logger() {
  if (fs.is_open()) {
    fs.close();
    fs.flush();
  }
}
