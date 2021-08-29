cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(program_options)

set(CMAKE_PREFIX_PATH E:/solomatin/boost/)
find_package(Boost REQUIRED COMPONENTS program_options)
add_library(program_options program_options.h program_options.cpp)

include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(program_options ${Boost_LIBRARIES})