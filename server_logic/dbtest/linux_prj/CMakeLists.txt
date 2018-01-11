cmake_minimum_required(VERSION 3.6)
project(dbtest)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../public/server_common)
include_directories(../../../Include/Package/common/hfbs)
include_directories(../Src)

link_directories(../../../server_bin/lib)


set(DBTEST_SOURCES
        ../Src/ClientSide/ClientSide.cpp
        ../Src/DB/DatabaseMgr.cpp
        ../Src/DB/GDBCSStruct.cpp
        ../Src/MySQLDriver/MySQLDriver.cpp
        ../Src/MySQLDriver/MySQLDriverList.cpp
        ../Src/Config.cpp
        ../Src/GlobalResource.cpp
        ../Src/Server.cpp
        ../Src/ServerMgr.cpp
        ../Src/ServerMgrTimeTask.cpp
        ../Src/stdafx.cpp)

add_executable(dbtest ${DBTEST_SOURCES})