#include "log.h"

Logger& Logger::operator << (const std::string& msg) {
  if (fs.is_open()) {
    std::lock_guard<std::mutex> lck(mtx);
    fs << msg << "\n";
  }
  return *this;
}

Logger::~Logger() {
  if (fs.is_open()) {
    fs.close();
  }
}
