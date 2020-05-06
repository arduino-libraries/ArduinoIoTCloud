##########################################################################

cmake_minimum_required(VERSION 2.8)

##########################################################################

project(testArduinoIoTCloud)

##########################################################################

include_directories(include)
include_directories(../../src/utility/ota)
include_directories(external/catch/v2.12.1/include)
include_directories(external/fakeit/v2.0.5/include)

##########################################################################

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##########################################################################

set(TEST_TARGET testArduinoIoTCloud)

set(TEST_SRCS
  src/test_main.cpp
  src/test_OTALogic.cpp
  src/OTATestData.cpp
  ../../src/utility/ota/crc.cpp
  ../../src/utility/ota/OTALogic.cpp
)

##########################################################################

add_compile_definitions(HOST)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

set(CMAKE_C_FLAGS   ${CMAKE_C_FLAGS}   "--coverage")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--coverage")

##########################################################################

add_executable(
  ${TEST_TARGET}
  ${TEST_SRCS}
)

##########################################################################

