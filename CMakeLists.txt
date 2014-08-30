# cd build/debug
# cmake ../.. -DCMAKE_BUILD_TYPE=Debug

cmake_minimum_required(VERSION 3.0.0)
project(Mip)

set(CMAKE_CXX_FLAGS "-Wall -g -std=c++11")

if (CMAKE_BUILD_TYPE STREQUAL debug)
  set(BUILD_POSTFIX "_d")
else (CMAKE_BUILD_TYPE STREQUAL debug)
  set(BUILD_POSTFIX "")
endif(CMAKE_BUILD_TYPE STREQUAL debug)

include_directories(${CMAKE_SOURCE_DIR}/lib/mpich/include)

add_subdirectory(src)
