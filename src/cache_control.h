#ifndef CACHE_CONTROL_H
#define CACHE_CONTROL_H

typedef struct ReqCacheControl_type {
  bool no_store;
  bool no_cache;
} ReqCacheControl;

typedef struct RspCacheControl_type {
  bool no_store;
  bool no_cache;
} RspCacheControl;
#endif
