# Install script for directory: /Users/shiyunjie/Documents/work/phoenix/server/phoenix

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/common/CommonLib/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/common/EncryptLib/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/common/memcache/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/common/mysqlpool/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/public/kafka/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/public/redisclient/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/network/NetT/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/network/NetTB/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/network/NetWS/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/network/NetHttp/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/network/NetTPool/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_comet/tcpcomet/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_comet/wscomet/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_comet/httpcomet/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_logic/regserver/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_logic/userserver/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_logic/scserver/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_logic/gcserver/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_logic/groupserver/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_logic/fdserver/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_logic/ecserver/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_logic/offcialserver/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_route/scroute/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_route/gcroute/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_route/pcroute/linux_prj/cmake_install.cmake")
  include("/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/server_logic/dbtest/linux_prj/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/shiyunjie/Documents/work/phoenix/server/phoenix/cmake-build-debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
