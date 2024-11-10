# 指定CMake最低版本 建议写
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# 指定项目名称 （版本 备注 语言...）
project(kv-database-server-common)

# set(HOME /root/project/all2023/kv-database)
# set(EXECUTABLE_OUTPUT_PATH ${HOME}/test)

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)
SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") 
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

file(GLOB UTILS_LIB ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB UTILS_MEMCTX_LIB ${CMAKE_CURRENT_SOURCE_DIR}/memctx/*.c)
# file(GLOB UTILS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_library(JANSSON_LIBRARY NAMES jansson libjansson)

# 设置动态库生成路径
set(LIBRARY_OUTPUT_PATH ${LIB_SO_PATH})

# message("JANSSON_LIBRARY: ${JANSSON_LIBRARY}")
add_library(kvcommon SHARED ${UTILS_LIB} ${JANSSON_LIBRARY} ${UTILS_MEMCTX_LIB})