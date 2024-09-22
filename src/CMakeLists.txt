# 指定CMake最低版本 建议写
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# 指定项目名称 （版本 备注 语言...）
project(kv-database-server)

# main的路径
set(MAIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/main)
# runtime模块的路径
set(RUNTIME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/runtime)
# storage模块的路径
set(STORAGE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/storage)
# common模块的路径
set(COMMON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common)
# interface 模块的路径
set(INTERFACE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/interface)

add_subdirectory(common)
# add_subdirectory(interface)
add_subdirectory(runtime)
# add_subdirectory(storage)
add_subdirectory(main)
add_subdirectory(client)