cmake_minimum_required(VERSION 3.6)
project(EncryptLib)

set(CMAKE_CXX_STANDARD 11)
include_directories(../../../Include)
include_directories(../../../Include/common)

set(ENCRYPTLIB_SOURCES
        ../src/ASCIIEnrypt.cpp
        ../src/BlowFishEncrypt.cpp
        ../src/Encrypt.cpp
        ../src/TeaEncrypt.cpp)

add_library(EncryptLib ${ENCRYPTLIB_SOURCES})