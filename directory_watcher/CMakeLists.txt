cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(directory_watcher)

include_directories("..\\reader")
include_directories("..\\parser")
include_directories("..\\logger")

add_library(directory_watcher "directory_watcher.h" "directory_watcher.cpp")

target_link_libraries(${PROJECT_NAME} reader)