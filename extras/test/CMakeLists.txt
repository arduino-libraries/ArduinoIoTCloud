##########################################################################

cmake_minimum_required(VERSION 2.8)

##########################################################################

project(testArduinoIoTCloud)

##########################################################################

include_directories(include)
include_directories(../../src)
include_directories(../../src/cbor)
include_directories(../../src/property)
include_directories(../../src/utility/time)
include_directories(external/catch/v2.12.1/include)
include_directories(external/fakeit/v2.0.5/include)

##########################################################################

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##########################################################################

set(TEST_TARGET ${CMAKE_PROJECT_NAME})

##########################################################################

set(TEST_SRCS
  src/test_addPropertyReal.cpp
  src/test_callback.cpp
  src/test_CloudColor.cpp
  src/test_CloudLocation.cpp
  src/test_CloudSchedule.cpp
  src/test_decode.cpp
  src/test_encode.cpp
  src/test_publishEvery.cpp
  src/test_publishOnChange.cpp
  src/test_publishOnChangeRateLimit.cpp
  src/test_readOnly.cpp
  src/test_writeOnly.cpp
)

set(TEST_UTIL_SRCS
  src/util/CBORTestUtil.cpp
  src/util/PropertyTestUtil.cpp
)

set(TEST_DUT_SRCS
  ../../src/property/Property.cpp
  ../../src/property/PropertyContainer.cpp
  ../../src/cbor/CBORDecoder.cpp
  ../../src/cbor/CBOREncoder.cpp
  ../../src/cbor/lib/tinycbor/src/cborencoder.c
  ../../src/cbor/lib/tinycbor/src/cborencoder_close_container_checked.c
  ../../src/cbor/lib/tinycbor/src/cborerrorstrings.c
  ../../src/cbor/lib/tinycbor/src/cborparser.c
  ../../src/cbor/lib/tinycbor/src/cborparser_dup_string.c
  ../../src/cbor/lib/tinycbor/src/cborpretty.c
  ../../src/cbor/lib/tinycbor/src/cborpretty_stdio.c
  ../../src/cbor/lib/tinycbor/src/cbortojson.c
  ../../src/cbor/lib/tinycbor/src/cborvalidation.c
  ../../src/cbor/lib/tinycbor/src/open_memstream.c
)

##########################################################################

set(TEST_TARGET_SRCS
  src/Arduino.cpp
  src/test_main.cpp
  ${TEST_SRCS}
  ${TEST_UTIL_SRCS}
  ${TEST_DUT_SRCS}
)

##########################################################################

add_compile_definitions(HOST)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)
add_compile_options(-Wno-cast-function-type)

set(CMAKE_C_FLAGS   ${CMAKE_C_FLAGS}   "--coverage")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--coverage -Wno-deprecated-copy")

##########################################################################

add_executable(
  ${TEST_TARGET}
  ${TEST_TARGET_SRCS}
)

##########################################################################

