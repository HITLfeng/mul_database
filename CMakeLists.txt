# 指定CMake最低版本 建议写
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# 指定项目名称 （版本 备注 语言...）
project(kv-database)

# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# src的路径
set(SERVER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 生成的动态库目录
set(LIB_SO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/output/lib)
# 生成的bin目录
set(BIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/output/bin)

add_subdirectory(src)
add_subdirectory(test)

# 生成动态库
# add_library(kvdatabase SHARED)