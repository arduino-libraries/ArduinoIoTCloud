##########################################################################

cmake_minimum_required(VERSION 3.5)

##########################################################################

project(testArduinoIoTCloud)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_Declare(
  cloudutils
  GIT_REPOSITORY      https://github.com/arduino-libraries/Arduino_CloudUtils.git
  GIT_TAG             main
)

FetchContent_Declare(
  connectionhandler
  GIT_REPOSITORY      https://github.com/arduino-libraries/Arduino_ConnectionHandler.git
  GIT_TAG             master
)

FetchContent_MakeAvailable(Catch2)

FetchContent_MakeAvailable(cloudutils)

FetchContent_MakeAvailable(connectionhandler)
##########################################################################

include_directories(include)
include_directories(../../src)
include_directories(../../src/message)
include_directories(../../src/cbor)
include_directories(../../src/property)
include_directories(../../src/utility/time)

# add_library(cloudutils STATIC IMPORTED GLOBAL)
add_library(cloudutils INTERFACE)

target_include_directories(
  cloudutils INTERFACE
  ${cloudutils_SOURCE_DIR}/src/
)

target_include_directories(
  cloudutils INTERFACE
  ${cloudutils_SOURCE_DIR}/src/cbor
)

target_include_directories(
  cloudutils INTERFACE
  ${cloudutils_SOURCE_DIR}/src/cbor/utils
)

target_include_directories(
  cloudutils INTERFACE
  ${cloudutils_SOURCE_DIR}/src/interfaces
)

add_library(connectionhandler INTERFACE)

target_include_directories(
    connectionhandler INTERFACE
  ${connectionhandler_SOURCE_DIR}/src/
)

target_include_directories(
    connectionhandler INTERFACE
  ${connectionhandler_SOURCE_DIR}/src/connectionHandlerModels
)

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
  src/test_CloudFloat.cpp
  src/test_CloudWrapperFloat.cpp
  src/test_CloudLocation.cpp
  src/test_CloudSchedule.cpp
  src/test_decode.cpp
  src/test_encode.cpp
  src/test_command_decode.cpp
  src/test_command_encode.cpp
  src/test_publishEvery.cpp
  src/test_publishOnChange.cpp
  src/test_publishOnChangeRateLimit.cpp
  src/test_readOnly.cpp
  src/test_writeOnly.cpp
  src/test_writeOnDemand.cpp
  src/test_writeOnChange.cpp
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
  ../../src/cbor/IoTCloudMessageDecoder.cpp
  ../../src/cbor/IoTCloudMessageEncoder.cpp

  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/cborencoder.c
  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/cborencoder_close_container_checked.c
  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/cborerrorstrings.c
  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/cborparser.c
  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/cborparser_dup_string.c
  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/cborpretty.c
  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/cborpretty_stdio.c
  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/cbortojson.c
  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/cborvalidation.c
  ${cloudutils_SOURCE_DIR}/src/cbor/tinycbor/src/open_memstream.c
  ${cloudutils_SOURCE_DIR}/src/cbor/MessageDecoder.cpp
  ${cloudutils_SOURCE_DIR}/src/cbor/MessageEncoder.cpp
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

add_compile_definitions(BOARD_HAS_LORA BOARD_HAS_CATM1_NBIOT BOARD_HAS_WIFI BOARD_HAS_ETHERNET BOARD_HAS_CELLULAR BOARD_HAS_NB BOARD_HAS_GSM)
add_compile_definitions(HOST HAS_TCP)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)
add_compile_options(-Wno-cast-function-type)

set(CMAKE_C_FLAGS   ${CMAKE_C_FLAGS}   "--coverage")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--coverage -Wno-deprecated-copy -Wno-missing-field-initializers")

##########################################################################

add_executable(
  ${TEST_TARGET}
  ${TEST_TARGET_SRCS}
)

target_link_libraries( ${TEST_TARGET} connectionhandler)
target_link_libraries( ${TEST_TARGET} cloudutils)
target_link_libraries( ${TEST_TARGET} Catch2WithMain )

##########################################################################

