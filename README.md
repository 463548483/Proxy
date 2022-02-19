# HTTP_caching_proxy

## Cache Rule
1. If cache-control include private or no store, not stored in Cache
2. Response must have max-age or expire time
3. Get directly from cache
    a) method=GET
    b) without "no cache" label both in request and response
    c) fresh(not expired)
4. How to decide fresh/not expire
    Suppose client,proxy,server have the same time, ignore the time cost in the process of send/receive/parse
    a) If without max-age, expire time > system time (expire time must be TMT/UTC time)
    b) If only with max-age, current age<max-age
    c) If has both max-age and Age, current age+ Age < max-age   
5. not fresh, revalide:
    a) provide Etag and last modify(if any) and send revalid request to server, log "in cache, requires validation"
    c) if none of Etag/last modify, log "in cache, but expired at EXPIREDTIME"
6. received revalide:
    a) get 304: replace header file
    b) get 5xx: send response to client, no update cache
    c) get 200: replace whole response
7. Cache evication:
    remove the first cache record in queue
