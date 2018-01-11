cmake_minimum_required(VERSION 3.6)
project(NetWS)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DWEBSOCKETPP)

include_directories(../../../Include)
include_directories(../../common_tcp)
include_directories(../prj)

set(NETWS_SOURCES
        ../../common_tcp/BaseTCPMgr.cpp
        ../../common_tcp/WebSocketMgr.cpp
        ../../common_tcp/SelectMgr.cpp
        ../../common_tcp/TCPPacket.cpp
        ../prj/DLLExport.cpp
        ../prj/stdafx.cpp
        ../src/WSMgr.cpp
        ../src/TCPMgr.cpp)

add_library(NetWS SHARED ${NETWS_SOURCES})