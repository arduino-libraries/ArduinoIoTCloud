#ifndef CBOR_BUFFER_H_
#define CBOR_BUFFER_H_

#include "ArduinoCbor.h"

class CborBuffer {
 public:
  cn_cbor_context context;
 
	CborBuffer(size_t size);
  ~CborBuffer();
  
  CborVariant decode(uint8_t* data, size_t size);

  uint8_t* alloc(size_t size);  

 protected:
  uint8_t* start;
  uint8_t* end;
  uint8_t* offset;
  cn_cbor* raw = NULL;

  static uint8_t* calloc(size_t count, size_t size, CborBuffer* context);
  static void free(uint8_t* ptr, CborBuffer* context);
};

#endif // CBOR_BUFFER_H_
