cmake_minimum_required(VERSION 3.20)
project(http)

aux_source_directory(./src HTTP_SOURCES)

add_library(http OBJECT ${HTTP_SOURCES} src/request.cpp http/stream.hpp src/stream.cpp http/room.hpp src/room.cpp)
target_include_directories(http PUBLIC ./)

if(CMAKE_HOST_UNIX)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -luring -pthread")
        target_include_directories(http PUBLIC /usr/include/)
        target_link_libraries(http uring)
    endif()
endif()