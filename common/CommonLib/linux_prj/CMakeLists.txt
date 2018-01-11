cmake_minimum_required(VERSION 3.6)
project(CommonLib)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/common)

set(COMMONLIB_SOURCES
        ../src/BaseConfig.cpp
        ../src/basefun.cpp
        ../src/BaseThread.cpp
        ../src/crc32.cpp
        ../src/DebugTrace.cpp
        ../src/DebugTraceThread.cpp
        ../src/Event.cpp
        ../src/FileSerialize.cpp
        ../src/IniEx.cpp
        ../src/md5.cpp
        ../src/MemorySerialize.cpp
        ../src/PacketQueue.cpp
        ../src/stdafx.cpp
        ../src/stdafx.h
        ../src/System.cpp
        ../src/U9Exception.cpp
        ../src/U9File.cpp
        ../src/U9Process.cpp)

add_library(CommonLib ${COMMONLIB_SOURCES})