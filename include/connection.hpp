//
// Created by SamGaaWaa on 2022/8/23.
//

#ifndef HTTP_SERVER_CONNECTION_HPP
#define HTTP_SERVER_CONNECTION_HPP

#include <memory>
#include <array>
#include <iostream>
#include <atomic>

#include "boost/asio.hpp"
#include "parser.hpp"
#include "request_handler.h"
#include "response_writer.h"

using boost::asio::ip::tcp;

namespace http {

    template<typename Request_Handle = request_handler, typename Response_Writer = response_writer>
    class connection: public std::enable_shared_from_this<connection<Request_Handle, Response_Writer>> {
        public:

        explicit connection(tcp::socket socket, boost::asio::io_context& context, const Request_Handle& request_handle,
            const Response_Writer& response_writer) noexcept
            : _socket(std::move(socket)), _context{ context }, _request_handle{ request_handle },
            _response_writer{ response_writer } {}

        ~connection()noexcept {
            _socket.close();
        }

        void start() {
            do_read();
        }

        void close() {
            _socket.close();
        }

        private:
        using buffer_type = std::array<char, 65536>;

        void do_read() {
            auto self{ this->shared_from_this() };
            _socket.async_read_some(boost::asio::buffer(_data),
                [this, self](auto error, size_t length) {
                    auto s = _parser(_data.data(), length);
            _request = _parser.result();
            if (_request) {
                http_response response = _request_handle(_request.value());
                _response_writer(std::move(response), _socket, _context,
                    (buffer_type*)&_data, self);
            }
            else if (!error) {
                do_read();
            }
            else close();

                });
        }


        tcp::socket _socket;
        buffer_type _data{};
        request_parser _parser{};
        boost::asio::io_context& _context;

        const Request_Handle& _request_handle;
        const Response_Writer& _response_writer;


        std::optional<http_request> _request;
    };


}
#endif //HTTP_SERVER_CONNECTION_HPP
