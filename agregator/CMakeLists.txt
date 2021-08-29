cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(agregator)

include_directories("..\\parser")

add_library(agregator "agregator.h" "agregator.cpp")
