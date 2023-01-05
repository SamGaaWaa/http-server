cmake_minimum_required(VERSION 3.20)
project(http-server)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(http)

add_executable(http-server test.cpp)
target_link_libraries(http-server http)
