cmake_minimum_required(VERSION 3.6)
project(gcroute)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/Package/common/hfbs)
include_directories(../Src)
include_directories(../../../public/server_common)

link_directories(../../../server_bin/lib)


set(GCROUTE_SOURCES
        ../Src/ClientSide/ClientSide.cpp
        ../Src/CKafkaConsumeThread.cpp
        ../Src/Config.cpp
        ../Src/GlobalResource.cpp
        ../Src/Server.cpp
        ../Src/ServerMgr.cpp
        ../Src/ServerMgrTimeTask.cpp
        ../Src/ConsumeDealMsgThread.cpp
        ../Src/ConsumeThreadNotify.cpp
        ../Src/stdafx.cpp
        ../../../public/server_common/ServerUtilMgr.cpp)

add_executable(gcroute ${GCROUTE_SOURCES} ../../../public/server_common/SensitiveMgr.cpp ../../../public/server_common/SensitiveMgr.h)