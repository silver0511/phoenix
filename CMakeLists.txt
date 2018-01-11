cmake_minimum_required(VERSION 3.8)
set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk")
project(phoenix)

set(CMAKE_CXX_STANDARD 11)

include_directories(/usr/local/include)
#common
add_subdirectory(common/CommonLib/linux_prj)
add_subdirectory(common/EncryptLib/linux_prj)
add_subdirectory(common/memcache/linux_prj)
add_subdirectory(common/mysqlpool/linux_prj)

#public
add_subdirectory(public/kafka/linux_prj)
add_subdirectory(public/redisclient/linux_prj)

#network
add_subdirectory(network/NetT/linux_prj)
add_subdirectory(network/NetTB/linux_prj)
add_subdirectory(network/NetWS/linux_prj)
add_subdirectory(network/NetHttp/linux_prj)
add_subdirectory(network/NetTPool/linux_prj)

#server
add_subdirectory(server_comet/tcpcomet/linux_prj)
add_subdirectory(server_comet/wscomet/linux_prj)
add_subdirectory(server_comet/httpcomet/linux_prj)
add_subdirectory(server_logic/regserver/linux_prj)
add_subdirectory(server_logic/userserver/linux_prj)
add_subdirectory(server_logic/scserver/linux_prj)
add_subdirectory(server_logic/gcserver/linux_prj)
add_subdirectory(server_logic/groupserver/linux_prj)
add_subdirectory(server_logic/fdserver/linux_prj)
add_subdirectory(server_logic/ecserver/linux_prj)
add_subdirectory(server_logic/offcialserver/linux_prj)
add_subdirectory(server_route/scroute/linux_prj)
add_subdirectory(server_route/gcroute/linux_prj)
add_subdirectory(server_route/pcroute/linux_prj)
add_subdirectory(server_logic/dbtest/linux_prj)