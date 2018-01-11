cmake_minimum_required(VERSION 3.6)
project(regserver)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/Package/common/hfbs)
include_directories(../../../public/server_common)
include_directories(../Src)

link_directories(../../../server_bin/lib)


set(REGSERVER_SOURCES
        ../Src/ClientSide/ClientSide.cpp
        ../Src/ServerSide/ServerSide.cpp
        ../Src/Config.cpp
        ../Src/GlobalResource.cpp
        ../Src/Server.cpp
        ../Src/ServerMgr.cpp
        ../Src/ServerMgrTimeTask.cpp
        ../Src/stdafx.cpp
        ../Src/SmsVerifyTask.cpp
        ../../../public/server_common/ServerConn/User.cpp
        ../../../public/server_common/ServerConn/UserConMgr.cpp
        ../../../public/server_common/ThreadNotify.cpp
        ../../../public/server_common/TaskThreadPoll.cpp
        ../../../public/server_common/httpclient.cpp)

add_executable(regserver ${REGSERVER_SOURCES})