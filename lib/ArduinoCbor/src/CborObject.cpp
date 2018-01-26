#include "CborObject.h"

CborObject::CborObject(CborBuffer& buffer, cn_cbor* raw) : buffer(buffer) {
  this->raw = raw;

  if (raw == 0) {
    cn_cbor_errback err;

    this->raw = cn_cbor_map_create(&buffer.context, &err);
  }
}

CborObject::~CborObject() {
  if (!raw->parent) {
    cn_cbor_free(raw, &(buffer.context));
  }
}

CborVariant CborObject::get(const char* key) {
  return CborVariant(buffer, cn_cbor_mapget_string(raw, key));
}

void CborObject::set(const char* key, CborVariant value) {
  cn_cbor_errback err;

  cn_cbor_mapput_string(raw, key, value.raw, &buffer.context, &err);
}

void CborObject::set(const char* key, const char* value) {
  set(key, CborVariant(buffer, value));
}

void CborObject::set(const char* key, CBOR_INT_T value) {
  set(key, CborVariant(buffer, value));
}

void CborObject::set(const char* key, CborObject value) {
  set(key, CborVariant(buffer, value));
}

void CborObject::set(const char* key, CborArray value) {
  set(key, CborVariant(buffer, value));
}

size_t CborObject::encode(uint8_t* data, size_t size) {
  return cn_cbor_encoder_write(data, 0, size, raw);
}
