cmake_minimum_required(VERSION 3.6)
project(memcache)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/common)

set(MEMCACHE_SOURCES
        ../src/BaseMemCache.cpp)

add_library(memcache ${MEMCACHE_SOURCES})