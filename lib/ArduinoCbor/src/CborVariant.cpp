#include "CborVariant.h"

CborVariant::CborVariant(CborBuffer& buffer, cn_cbor* raw) : buffer(buffer) {
  this->raw = raw;
}

CborVariant::CborVariant(CborBuffer& buffer, const char* value) : buffer(buffer) {
  cn_cbor_errback err;

  raw = cn_cbor_string_create(value, &buffer.context, &err);
}

CborVariant::CborVariant(CborBuffer& buffer, CBOR_INT_T value) : buffer(buffer) {
  cn_cbor_errback err;

  raw = cn_cbor_int_create(value, &buffer.context, &err);
}

CborVariant::CborVariant(CborBuffer& buffer, CborObject& value) : buffer(buffer) {
  this->raw = value.raw;
}

CborVariant::CborVariant(CborBuffer& buffer, CborArray& value) : buffer(buffer) {
  this->raw = value.raw;
}

int CborVariant::length() {
  if (raw == 0) {
    return -1;
  }

  return raw->length;
}

bool CborVariant::isValid() {
  return raw != 0;
}

bool CborVariant::isString() {
  return isValid() && raw->type == CN_CBOR_BYTES || raw->type == CN_CBOR_TEXT;
}

bool CborVariant::isInteger() {
  return isValid() && raw->type == CN_CBOR_UINT || raw->type == CN_CBOR_INT;
}

bool CborVariant::isObject() {
  return isValid() && raw->type == CN_CBOR_MAP;
}

bool CborVariant::isArray() {
  return isValid() && raw->type == CN_CBOR_ARRAY;
}

const char* CborVariant::asString() {
  if (!isValid()) {
    return 0;
  }

  if (raw->type != CN_CBOR_BYTES && raw->type != CN_CBOR_TEXT) {
    return 0;
  }

  if (raw->v.str[raw->length] != 0) {
    char* tmp = (char*)buffer.alloc(raw->length + 1);
    
    for (int i = 0; i < raw->length; i++) {
      tmp[i] = raw->v.str[i];
    }

    return tmp;
  }

  return raw->v.str;
}

CBOR_INT_T CborVariant::asInteger() {
  if (!isValid()) {
    return 0;
  }

  if (raw->type == CN_CBOR_UINT) {
    return raw->v.uint;
  }

  if (raw->type == CN_CBOR_INT) {
    return raw->v.sint;
  }

  return 0;
}

CborObject CborVariant::asObject() {
  if (isObject()) {
    return CborObject(buffer, raw);
  }

  return CborObject(buffer);
}

CborArray CborVariant::asArray() {
  if (isArray()) {
    return CborArray(buffer, raw);
  }

  return CborArray(buffer);
}

size_t CborVariant::encode(uint8_t* data, size_t size) {
  return cn_cbor_encoder_write(data, 0, size, raw);
}
