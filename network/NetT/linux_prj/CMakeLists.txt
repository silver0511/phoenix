cmake_minimum_required(VERSION 3.6)
project(NetT)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../common_tcp)
include_directories(../prj)

set(NETT_SOURCES
        ../../common_tcp/BaseTCPMgr.cpp
        ../../common_tcp/EpollMgr.cpp
        ../../common_tcp/SelectMgr.cpp
        ../../common_tcp/TCPPacket.cpp
        ../../common_tcp/ClientPack.cpp
        ../prj/DLLExport.cpp
        ../src/TCPMgr.cpp)

add_library(NetT SHARED ${NETT_SOURCES})