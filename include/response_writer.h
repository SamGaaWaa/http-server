//
// Created by SamGaaWaa on 2022/9/4.
//

#ifndef HTTP_SERVER_RESPONSE_WRITER_H
#define HTTP_SERVER_RESPONSE_WRITER_H

#include "http_response.h"
#include "boost/asio.hpp"
#include <memory>
#include <array>
#include <algorithm>
#include <cstdio>


namespace http {

    class response_writer {
        public:
        auto operator()(http_response response, boost::asio::ip::tcp::socket& socket,
            boost::asio::io_context& context, auto buff, auto connection_ptr)const {
            auto tmp_ptr = std::make_shared<std::string>(http_response::status_to_string(response.status));
            tmp_ptr->reserve(128);

            for (const auto& [k, v] : response.headers) {
                tmp_ptr->append(k);
                tmp_ptr->append(": ");
                tmp_ptr->append(v);
                tmp_ptr->append("\r\n");
            }
            tmp_ptr->append("\r\n");
            std::string root = response.content;
            boost::asio::async_write(socket, boost::asio::buffer(*tmp_ptr), [tmp_ptr, buff, &socket, this, root = std::move(root),
                connection_ptr](auto error, size_t) {
                    if (error)
                    return;
            FILE* file = std::fopen(root.c_str(), "rb");
            if (!file) {
                std::printf("文件打不开\n");
                std::cerr << ::strerror(errno) << '\n';
                return;
            }
            read_file_and_async_write(file, buff->data(), buff->size(), socket, connection_ptr);
                });
        }

        private:
        void read_file_and_async_write(FILE* file, char* buff, size_t size, boost::asio::ip::tcp::socket& socket,
            auto connection_ptr)const {
            auto res{ 0 };
            try {
                res = std::fread(buff, 1, size, file);
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << '\n';
                std::fclose(file);
                return;
            }
            if (res <= 0) {
                std::fclose(file);
                return;
            }
            boost::asio::async_write(socket, boost::asio::buffer(buff, res), [&socket, this, file, buff, size,
                connection_ptr](auto error, size_t) {
                    read_file_and_async_write(file, buff, size, socket, connection_ptr);
                });
        }
    };



}

#endif //HTTP_SERVER_RESPONSE_WRITER_H
