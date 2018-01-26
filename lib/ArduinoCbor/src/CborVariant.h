#ifndef CBOR_VARIANT_H_
#define CBOR_VARIANT_H_

#include "ArduinoCbor.h"

class CborVariant {
 friend class CborArray;
 friend class CborObject;

 public:
  CborVariant(CborBuffer& buffer, cn_cbor* raw);
  CborVariant(CborBuffer& buffer, const char* value);
  CborVariant(CborBuffer& buffer, CBOR_INT_T value);
  CborVariant(CborBuffer& buffer, CborObject& value);
  CborVariant(CborBuffer& buffer, CborArray& value);

  int length();

  bool isValid();
  bool isString();
  bool isInteger();
  bool isObject();
  bool isArray();

  const char* asString();
  CBOR_INT_T asInteger();
  CborObject asObject();
  CborArray asArray();

  size_t encode(uint8_t* data, size_t size);

 protected:
  CborBuffer& buffer;
  cn_cbor* raw;
};

#endif // CBOR_VARIANT_H_
