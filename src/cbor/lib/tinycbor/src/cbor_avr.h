#ifndef CBOR_AVR_H
#define CBOR_AVR_H

#define FP_INFINITE INFINITY
#define FP_NAN NAN
#define PRIu64 "llu"
#define PRIx64 "llx"

#define ssize_t int

extern int fpclassify(double x);

#endif /* CBOR_AVR_H */