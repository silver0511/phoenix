cmake_minimum_required(VERSION 3.6)
project(NetHTTP)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DWEBSOCKETPP)

include_directories(../../../Include)
include_directories(../../common_tcp)
include_directories(../prj)

set(NetHTTP_SOURCES
        ../../common_tcp/BaseTCPMgr.cpp
        ../../common_tcp/HttpSocketMgr.cpp
        ../../common_tcp/SelectMgr.cpp
        ../../common_tcp/TCPPacket.cpp
        ../prj/DLLExport.cpp
        ../prj/stdafx.cpp
        ../src/HTTPMgr.cpp
        ../src/TCPMgr.cpp)

add_library(NetHTTP SHARED ${NetHTTP_SOURCES})