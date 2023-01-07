cmake_minimum_required(VERSION 3.20)
project(http)

aux_source_directory(./src SOURCES)

add_library(http OBJECT ${SOURCES})
target_include_directories(http PUBLIC ./)

if(CMAKE_HOST_UNIX)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -luring -pthread")
        target_include_directories(http PUBLIC /usr/include/)
        target_link_libraries(http uring)
    endif()
endif()