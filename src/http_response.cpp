#include "http_response.h"

// need to change both header_fields and cache control
void HttpResponse::replace_header_fields(const HttpResponse* rsp) {
  header_fields = rsp->header_fields;
  code = rsp->code;
}
