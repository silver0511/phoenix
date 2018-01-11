cmake_minimum_required(VERSION 3.6)
project(gcserver)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/Package/common/hfbs)
include_directories(../../../public/server_common)
include_directories(../Src)

link_directories(../../../server_bin/lib)

set(GCSERVER_SOURCES
        ../Src/ClientSide/ClientSide.cpp
        ../Src/Config.cpp
        ../Src/GlobalResource.cpp
        ../Src/Server.cpp
        ../Src/ServerMgr.cpp
        ../Src/ServerMgrTimeTask.cpp
        ../../../public/server_common/ServerToKafka.cpp
        ../../../public/server_common/SensitiveMgr.cpp
        ../../../public/server_common/ServerConn/User.cpp
        ../../../public/server_common/ServerConn/UserConMgr.cpp
        ../Src/stdafx.cpp)

add_executable(gcserver ${GCSERVER_SOURCES})