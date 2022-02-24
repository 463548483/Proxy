#include "http_response.h"

// need to change both header_fields and cache control
void HttpResponse::replace_header_fields(const HttpResponse* rsp) {
  header_fields = rsp->header_fields;
  cache = rsp->cache;
}

bool HttpResponse::change_header_field(const std::string & field_name, const std::string & field_value) {
  HttpParser parser;
  size_t field_id = header_fields.size();
  if (parser.get_header_field(field_name, &header_fields, &field_id) == 0 || field_id == header_fields.size()) {
    return false;
  }
  std::vector<char>* field = &(header_fields.at(field_id));
  auto iter = std::find(field->begin(), field->end(), ':');
  if (iter == field->end()) {
    throw HttpResponseExc("HttpBase::change_header_field: field has wrong format, do not have ':'.");
  }
  field->resize(iter - field->begin() + 1);
  field->push_back(' ');
  field->insert(field->end(), field_value.begin(), field_value.end());
  return true;
}
