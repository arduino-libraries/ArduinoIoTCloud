#ifndef CBOR_ARRAY_H_
#define CBOR_ARRAY_H_

#include "ArduinoCbor.h"

class CborArray {
 friend class CborVariant;

 public:
  CborArray(CborBuffer& buffer, cn_cbor* raw=0);
  ~CborArray();

  CborVariant get(int index);

  void add(CborVariant value);
  void add(const char* value);
  void add(CBOR_INT_T value);

  ssize_t encode(uint8_t* data, size_t size);

 protected:
  CborBuffer& buffer;
  cn_cbor* raw;
};

#endif // CBOR_ARRAY_H_
