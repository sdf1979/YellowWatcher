cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(settings)

include_directories("..\\logger")

add_library(settings settings.h settings.cpp)