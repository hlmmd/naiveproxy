# 检查CMake版本
cmake_minimum_required(VERSION 3.3)

# project名称
project (naiveproxy)

# 将include目录添加到include的搜索路径中,PROJECT_SOURCE_DIR为当前工程根目录
include_directories ("${PROJECT_SOURCE_DIR}/include")

# 生成debug/release版本
SET(CMAKE_BUILD_TYPE "release")
if (CMAKE_BUILD_TYPE STREQUAL debug)
    add_definitions(-D_DEBUG)
endif ()
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb ")
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall ")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -W ")

# 设置版本号
#set (LIBQEVENT_VERSION_MAJOR 1)
#set (LIBQEVENT_VERSION_MINOR 0)

# 配置一个头文件，从而将CMake的一些设置传递到源文件中。
# LibqeventConfig.h.LibqeventConfig.h
#configure_file (
#  "${PROJECT_SOURCE_DIR}/LibqeventConfig.h.in"
#  "${PROJECT_SOURCE_DIR}/include/LibqeventConfig.hpp"
#)

# 添加 src 目录
add_subdirectory (src) 

# 设置可执行文件
add_executable(naiveproxy main.c)
add_executable(client client.c)

# 为可执行文件添加链接库文件
target_link_libraries (naiveproxy nproxy)

#安装 incldue目录下所有的文件
#INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include/qevent) 

