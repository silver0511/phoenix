cmake_minimum_required(VERSION 3.6)
project(gcserver)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../../Include)
include_directories(../../../Include/Package/common/hfbs)
include_directories(../../../public/server_common)
include_directories(../Src)

link_directories(../../../server_bin/lib)


aux_source_directory(../Src GCSERVER_SOURCES1)
aux_source_directory(../Src/ClientSide GCSERVER_SOURCES2)
aux_source_directory(../Src/OffciaToKafka GCSERVER_SOURCES3)


add_executable(offcialserver ${GCSERVER_SOURCES1} ${GCSERVER_SOURCES2} ${GCSERVER_SOURCES3})