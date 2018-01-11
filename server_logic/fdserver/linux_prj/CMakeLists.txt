cmake_minimum_required(VERSION 3.6)
project(fdserver)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../public/server_common)
include_directories(../../../Include/Package/common/hfbs)
include_directories(../Src)

link_directories(../../../server_bin/lib)


set(FDSERVER_SOURCES
        ../Src/ClientSide/ClientSide.cpp
        ../Src/Config.cpp
        ../Src/GlobalResource.cpp
        ../Src/Server.cpp
        ../Src/ServerMgr.cpp
        ../Src/ServerMgrTimeTask.cpp
        ../Src/stdafx.cpp
        ../Src/FriendListTask.cpp
        ../Src/FriendDelTask.cpp
        ../Src/FriendConfirmTask.cpp
        ../Src/FriendRemarkTask.cpp
        ../Src/FriendClientAddTask.cpp
        ../Src/FriendBlackListTask.cpp
        ../Src/FriendUpdateTask.cpp
        ../../../public/server_common/ThreadNotify.cpp
        ../../../public/server_common/TaskThreadPoll.cpp
        ../../../public/server_common/httpclient.cpp
        ../../../public/server_common/ServerToKafka.cpp
        ../../../public/server_common/ServerConn/User.cpp
        ../../../public/server_common/ServerConn/UserConMgr.cpp)

add_executable(fdserver ${FDSERVER_SOURCES})