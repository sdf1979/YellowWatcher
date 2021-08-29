cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(logger)

add_library(logger logger.h logger.cpp)