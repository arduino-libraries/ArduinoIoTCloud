#include <string.h>
#include <stdio.h>

#include "ArduinoCbor.h"

CborArray::CborArray(CborBuffer& buffer, cn_cbor* raw) : buffer(buffer) {
  this->raw = raw;

  if (raw == 0) {
    cn_cbor_errback err;

    this->raw = cn_cbor_array_create(&buffer.context, &err);  
  }
}

CborVariant CborArray::get(int index) {
  return CborVariant(buffer, cn_cbor_index(raw, index));
}

void CborArray::add(CborVariant value) {
  cn_cbor_errback err;

  cn_cbor_array_append(raw, value.raw, &err);
}

void CborArray::add(const char* value) {
  add(CborVariant(buffer, value));
}

void CborArray::add(CBOR_INT_T value) {
  add(CborVariant(buffer, value));
}
