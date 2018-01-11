cmake_minimum_required(VERSION 3.6)
project(pcroute)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/Package/common/hfbs)
include_directories(../Src)
include_directories(../../../public/server_common)

link_directories(../../../server_bin/lib)


set(PCROUTE_SOURCES
        ../Src/ClientSide/ClientSide.cpp
        ../Src/CKafkaConsumeThread.cpp
        ../Src/Config.cpp
        ../Src/GlobalResource.cpp
        ../Src/Server.cpp
        ../Src/ServerMgr.cpp
        ../Src/ServerMgrTimeTask.cpp
        ../Src/ConsumeDealMsgThread.cpp
        ../Src/ConsumeThreadNotify.cpp
        ../../../public/server_common/ServerToKafka.cpp
        ../Src/stdafx.cpp)

add_executable(pcroute ${PCROUTE_SOURCES})