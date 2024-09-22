# 指定CMake最低版本 建议写
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# 指定项目名称 （版本 备注 语言...）
project(kv-database-server-main)

# set(HOME /root/project/all2023/kv-database)
# set(EXECUTABLE_OUTPUT_PATH ${HOME}/test)

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)
SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") 
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

file(GLOB UTILS_LIB ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
# file(GLOB UTILS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${RUNTIME_PATH}/include)
include_directories(${INTERFACE_PATH}/include)
message(STATUS "include path: ${RUNTIME_PATH}/include")

# 设置动态库生成路径
# set(LIBRARY_OUTPUT_PATH ${LIB_SO_PATH})
# set(LIBRARY_OUTPUT_PATH ${LIB_SO_PATH})
set(EXECUTABLE_OUTPUT_PATH ${BIN_PATH})
# add_library(kvcommon SHARED ${UTILS_LIB})


add_executable(kvserver server_main.c)

target_link_libraries(kvserver pthread kvcommon kvruntime)
# target_link_libraries(kvserver pthread kvcommon kvinterface kvruntime kvstorage)