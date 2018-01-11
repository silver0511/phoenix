cmake_minimum_required(VERSION 3.6)
project(websocketpp)

set(CMAKE_CXX_STANDARD 11)

set(WBSPPS_SOURCES)

add_library(websocketpp SHARED ${WBSPPS_SOURCES})
