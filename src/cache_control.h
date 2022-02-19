#ifndef CACHE_CONTROL_H
#define CACHE_CONTROL_H

#include <ctime>
#include <cstdint>

typedef struct ReqCacheControl_type {
  bool no_store; //default: false; if true, do not store in cache
  bool no_cache; //default: false; if true, need revalidate everytime
} ReqCacheControl;

typedef struct RspCacheControl_type {
  // Things in Cache-Control
  bool no_store; //default: false; if true, do not store in cache
  bool no_cache; //default: false; if true, need revalidate everytime
  bool is_private; //default: false; if true, do not store in cache
  int64_t max-age; // If does not have max-age, max-age=negative

  // Other field related to Cache
  struct tm Expires; // If does not have Expires field or the field is invalid, Expires=NULL
  uint64_t Age; // If does not have a valid Age field=>Age=0
  std::string Etag; // default empty string
  std::string Last-Modified; // default empty string
} RspCacheControl;
#endif
