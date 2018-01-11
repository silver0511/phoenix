cmake_minimum_required(VERSION 3.6)
project(httpcomet)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/Package/common/hfbs)
include_directories(../../../public/server_common)
include_directories(../Src)

link_directories(../../../server_bin/lib)

set(HTTPCOMET_SOURCES
        ../Src/ClientSide/ClientSide.cpp
        ../Src/ServerSide/ServerSide.cpp
        ../../../public/server_common/ClientConn/User.cpp
        ../../../public/server_common/ClientConn/UserMgr.cpp
        ../Src/Config.cpp
        ../Src/GlobalResource.cpp
        ../Src/Server.cpp
        ../Src/ServerMgr.cpp
        ../Src/ServerMgrTimeTask.cpp
        ../Src/stdafx.cpp)

add_executable(httpcomet ${HTTPCOMET_SOURCES})