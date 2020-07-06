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

#endif