#ifndef CBOR_OBJECT_H_
#define CBOR_OBJECT_H_

#include "ArduinoCbor.h"
#include "stdlib.h"

class CborObject {
 friend class CborVariant;

 public:
  CborObject(CborBuffer& buffer, cn_cbor* raw=0);
  ~CborObject();

  CborVariant get(const char* key);

  void set(const char* key, CborVariant value);
  void set(const char* key, const char* value);
  void set(const char* key, CBOR_INT_T value);
  void set(const char* key, CborObject value);
  void set(const char* key, CborArray value);

  size_t encode(uint8_t* data, size_t size);

 protected:
  CborBuffer& buffer;
  cn_cbor* raw;
};

#endif // CBOR_OBJECT_H_
