cmake_minimum_required(VERSION 3.20)
project(http-server)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_HOST_UNIX)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -luring")
        include_directories(/usr/include/)
    endif()
endif()

find_package(Boost 1.80.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(http)

add_executable(http-server test.cpp)
target_link_libraries(http-server http)

if(CMAKE_HOST_UNIX)
    target_link_libraries(http-server uring)
endif()
