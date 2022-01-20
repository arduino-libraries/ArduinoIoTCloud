#ifndef CBOR_LIB_H
#define CBOR_LIB_H

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "src/cbor.h"

/******************************************************************************
 * DEFINE
 ******************************************************************************/

#ifndef CHECK_CBOR
  #define CHECK_CBOR(expr)           \
    do {                             \
      CborError error = CborNoError; \
      error = (expr);                \
      if (CborNoError != error)      \
        return error;                \
    } while(0);
#endif /* CHECK_CBOR */

#ifndef CHECK_CBOR_MULTI
  #define CHECK_CBOR_MULTI(expr)          \
    do {                                  \
      CborError error = CborNoError;      \
      error = (expr);                     \
      if (CborErrorOutOfMemory == error)  \
        return CborErrorSplitItems;     \
      if (CborNoError != error)           \
        return error;                     \
    } while(0);
#endif /* CHECK_CBOR_MULTI */

#endif