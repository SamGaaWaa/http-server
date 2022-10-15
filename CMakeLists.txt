cmake_minimum_required(VERSION 3.18.4)
project(http_server)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS -pthread)
endif()

add_executable(http_server main.cpp include/connection.hpp include/parser.hpp src/http_parser.c include/http_request.h include/http_response.h include/request_handler.h include/mime_types.h include/response_writer.h include/server.h)
include_directories(include/)

# find_package(asio CONFIG REQUIRED)
# if(asio_FOUND)
#     include_directories(asio_include_dirs)
#     target_link_libraries(http_server asio asio::asio)
# endif()

