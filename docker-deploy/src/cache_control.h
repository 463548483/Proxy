#ifndef CACHE_CONTROL_H
#define CACHE_CONTROL_H

#include <ctime>
#include <cstdint>

typedef struct ReqCacheControl_type {
  ReqCacheControl_type() : no_store(false), no_cache(false) {}
  bool no_store; //default: false; if true, do not store in cache
  bool no_cache; //default: false; if true, need revalidate everytime
} ReqCacheControl;

typedef struct RspCacheControl_type {
  RspCacheControl_type() : no_store(false), no_cache(false), is_private(false), max_age(-1),
      has_expires(false), expires(-1), age(0), etag(""), last_modified("") {}
  // Things in Cache-Control
  bool no_store; //default: false; if true, do not store in cache
  bool no_cache; //default: false; if true, need revalidate everytime
  bool is_private; //default: false; if true, do not store in cache
  int64_t max_age; // If does not have max-age, max-age=negative; if can not parse set to zero

  // Other field related to Cache
  bool has_expires; //default: false
  time_t expires; // If does not have Expires field or the field is invalid, Expires=-1
  uint64_t age; // If does not have a valid Age field=>Age=0
  std::string etag; // default empty string
  std::string last_modified; // default empty string
} RspCacheControl;
#endif
