cmake_minimum_required(VERSION 3.6)
project(redisclient)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../..//Include/common)

set(REDIS_SOURCES
        ../src/RedisClient.cpp
        ../src/RedisUtil.cpp
        ../src/RedisUtil_Login.cpp
        ../src/RedisUtil_Message.cpp
        ../src/RedisUtil_UserInfo.cpp
        ../src/RedisUtil_Friend.cpp
        ../src/RedisUtil_Trade.cpp
        ../src/RedisUtil_OffcialMessage.cpp
        ../src/RedisUtil_Reg.cpp
        ../src/LuaShaManager.cpp)

add_library(redisclient ${REDIS_SOURCES})