# 指定CMake最低版本 建议写
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# 指定项目名称 （版本 备注 语言...）
project(kvsysview)

# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HOME ${BASE_PATH})
set(EXECUTABLE_OUTPUT_PATH ${BIN_PATH})

SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") 
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

include_directories(${HOME}/output/include)
link_directories(${HOME}/output/lib)

include_directories(${COMMON_PATH}/include)
include_directories(${CLIENT_PATH}/include)
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
# 设置要依赖的动态库

# 指定可执行程序的名称及依赖


find_library(JANSSON_LIBRARY NAMES jansson libjansson)
add_executable(kvsysview ${SRC_LIST} ${SOURCES})

target_link_libraries(kvsysview pthread kvclient kvcommon ${JANSSON_LIBRARY})