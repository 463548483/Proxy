# HTTP_caching_proxy

## Thread pool
    - Pre-create threads to take the request from client
    - max 10 threads
    - max 100 tasks in queue for threads to taken

## Cache Rule
1. If cache-control include private or no store, not stored in Cache
2. If response does not have max-age or expire time, regarded as never expired!!
3. Get directly from cache
    - method=GET
    - without "no cache" label both in request and response
    - fresh(not expired)
4. How to decide fresh/not expired
    - Suppose client,proxy,server have the same time, ignore the time cost in the process of send/receive/parse
    - a) If without max-age, expire time > system time (expire time must be TMT/UTC time)
    - b) If only with max-age, current age< max-age
    - c) If has both max-age and Age, current age+ Age < max-age   
    - d) not max-age, no expire time
5. not fresh, revalide:
    - provide Etag and last modify(if any) and send revalid request to server, log "in cache, requires validation"
    - if none of Etag/last modify, log "in cache, but expired at EXPIREDTIME"
6. received revalide:
    - a) get 304: replace header file
    - b) get 5xx: send response to client, no update cache
    - c) get 200: replace whole response
7. Cache evication:
    - cache no more than 1000 responses
    - remove the first cache record in queue when the cache is full
