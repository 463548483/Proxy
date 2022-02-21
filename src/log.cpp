#include "log.h"

Logger& Logger::operator << (const std::string& msg) {
  if (fs.is_open()) {
    std::lock_guard<std::mutex> lck(mtx);
    fs << msg;
  }
  return *this;
}

Logger& Logger::operator << (const long long & msg) {
  if (fs.is_open()) {
    std::lock_guard<std::mutex> lck(mtx);
    fs << msg;
  }
  return *this;
}

Logger& Logger::operator << (const int & msg) {
  if (fs.is_open()) {
    std::lock_guard<std::mutex> lck(mtx);
    fs << msg;
  }
  return *this;
}

Logger& Logger::operator << (const unsigned long long & msg) {
  if (fs.is_open()) {
    std::lock_guard<std::mutex> lck(mtx);
    fs << msg;
  }
  return *this;
}

Logger& Logger::operator << (const unsigned int & msg) {
  if (fs.is_open()) {
    std::lock_guard<std::mutex> lck(mtx);
    fs << msg;
  }
  return *this;
}

Logger::~Logger() {
  if (fs.is_open()) {
    fs.close();
  }
}
