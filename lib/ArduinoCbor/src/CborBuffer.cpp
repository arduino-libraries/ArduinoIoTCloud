#include "CborBuffer.h"
#include "Arduino.h"

uint8_t* CborBuffer::calloc(size_t count, size_t size, CborBuffer* context) {
  return context->alloc(count * size);
}

void CborBuffer::free(uint8_t* ptr, CborBuffer* context) {
}

CborBuffer::CborBuffer(size_t size) {
  this->start = new uint8_t[size];
  this->offset = this->start;
  this->end = this->start + size;
  this->context.calloc_func = (cn_calloc_func)CborBuffer::calloc;
  this->context.free_func = (cn_free_func)CborBuffer::free;
  this->context.context = this;
}

CborBuffer::~CborBuffer() {
  delete[] this->start;
}

uint8_t* CborBuffer::alloc(size_t size) {
  if (offset + size > end) {
    return 0;
  }

  uint8_t* data = offset;

  offset += size;

  for (uint8_t* ptr = data; ptr < offset; ptr++) {
    *ptr = 0;
  }

  return data;
}

CborVariant CborBuffer::decode(uint8_t* data, size_t size) {
  cn_cbor_errback err;

  raw = cn_cbor_decode(data, size, &context, &err);

  return CborVariant(*this, raw);
}
