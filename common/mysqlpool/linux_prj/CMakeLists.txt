cmake_minimum_required(VERSION 3.6)
project(mysqlpool)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/common)

set(MYSQL_SOURCES
        ../src/DBI.cpp
        ../src/MySQLDriver.cpp
        ../src/MySQLDriverPool.cpp
        ../src/MySQLRes.cpp)

add_library(mysqlpool ${MYSQL_SOURCES})