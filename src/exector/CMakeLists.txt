# 指定CMake最低版本 建议写
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# 指定项目名称 （版本 备注 语言...）
project(kv-database-server-exector)

# set(HOME /root/project/all2023/kv-database)
# set(EXECUTABLE_OUTPUT_PATH ${HOME}/test)

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)
SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") 
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

file(GLOB UTILS_LIB ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
# file(GLOB UTILS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
message(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${INTERFACE_PATH}/include)
message(${INTERFACE_PATH}/include)
include_directories(${COMMON_PATH}/include)
message(${COMMON_PATH}/include)
include_directories(${EXECTOR_PATH}/include)
message(${EXECTOR_PATH}/include)
include_directories(${DATAMODEL_PATH}/simple_relation/include)
message(${DATAMODEL_PATH}/include)
include_directories(${STORAGE_PATH}/include)
message(${STORAGE_PATH}/include)

# 设置动态库生成路径
set(LIBRARY_OUTPUT_PATH ${LIB_SO_PATH})

add_library(kvexector SHARED ${UTILS_LIB})