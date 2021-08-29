cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CSS_STANDARD_LIBRARIES}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")

project(YellowWatcher VERSION 0.1.0)

set(CMAKE_PREFIX_PATH E:/solomatin/boost/)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(reader)
add_subdirectory(directory_watcher)
add_subdirectory(parser)
add_subdirectory(agregator)
add_subdirectory(sender)
add_subdirectory(settings)
add_subdirectory(logger)
add_subdirectory(program_options)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC reader)
target_include_directories(${PROJECT_NAME} PUBLIC directory_watcher)
target_include_directories(${PROJECT_NAME} PUBLIC parser)
target_include_directories(${PROJECT_NAME} PUBLIC agregator)
target_include_directories(${PROJECT_NAME} PUBLIC sender)
target_include_directories(${PROJECT_NAME} PUBLIC settings)
target_include_directories(${PROJECT_NAME} PUBLIC logger)
target_include_directories(${PROJECT_NAME} PUBLIC program_options)

target_link_libraries(${PROJECT_NAME} reader)
target_link_libraries(${PROJECT_NAME} directory_watcher)
target_link_libraries(${PROJECT_NAME} parser)
target_link_libraries(${PROJECT_NAME} agregator)
target_link_libraries(${PROJECT_NAME} sender)
target_link_libraries(${PROJECT_NAME} settings)
target_link_libraries(${PROJECT_NAME} logger)
target_link_libraries(${PROJECT_NAME} program_options)

set(CMAKE_CXX_FLAGS "-municode")
