cmake_minimum_required(VERSION 3.6)
project(kafka)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/common)


set(KKLIB_SOURCES
        ../src/KafkaMgr.cpp
        ../src/KafkaProducer.cpp)

add_library(kafka ${KKLIB_SOURCES})