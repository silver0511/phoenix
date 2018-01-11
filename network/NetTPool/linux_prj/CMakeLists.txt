cmake_minimum_required(VERSION 3.6)
project(NetTPool)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../prj)
include_directories(../src/)

set(NETPOOL_SOURCES
        ../prj/DLLExport.cpp
        ../src/TCPConnectPool.cpp
        ../src/TCPConnPoolMgr.cpp
       ../../common_tcp/ServerPack.cpp)

add_library(NetTPool SHARED ${NETPOOL_SOURCES})