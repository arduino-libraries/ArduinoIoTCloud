#ifndef ARDUINO_CBOR_H_
#define ARDUINO_CBOR_H_

#define USE_CBOR_CONTEXT

#include "cn-cbor/cn-cbor.h"

#ifndef CBOR_INT_T
#define CBOR_INT_T long
#endif

class CborArray;
class CborBuffer;
class CborObject;
class CborVariant;

#include "CborArray.h"
#include "CborBuffer.h"
#include "CborObject.h"
#include "CborVariant.h"

#endif // ARDUINO_CBOR_H_
