cmake_minimum_required(VERSION 3.6)
project(ecserver)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../public/server_common)
include_directories(../../../Include/Package/common/hfbs)
include_directories(../Src)

link_directories(../../../server_bin/lib)


set(ECSERVER_SOURCES
        ../Src/ClientSide/ClientSide.cpp
        ../Src/Config.cpp
        ../Src/GlobalResource.cpp
        ../Src/Server.cpp
        ../Src/ServerMgr.cpp
        ../Src/ServerMgrTimeTask.cpp
        ../Src/EcRecentMsgTask.cpp
        ../Src/stdafx.cpp
        ../Src/EcRecentOrdersTask.cpp
        ../Src/EcLastMsgTask.cpp
        ../../../public/server_common/ServerUtilMgr.cpp
        ../../../public/server_common/ServerConn/User.cpp
        ../../../public/server_common/ServerConn/UserConMgr.cpp
        ../../../public/server_common/ThreadNotify.cpp
        ../../../public/server_common/TaskThreadPoll.cpp
        ../../../public/server_common/httpclient.cpp)

add_executable(ecserver ${ECSERVER_SOURCES})