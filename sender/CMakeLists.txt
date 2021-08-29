cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(sender)

#set(CMAKE_PREFIX_PATH E:/solomatin/boost/)
#find_package(Boost REQUIRED)
#include_directories(${Boost_INCLUDE_DIR})

add_library(sender sender.h sender.cpp)