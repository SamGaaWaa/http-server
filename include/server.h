//
// Created by SamGaaWaa on 2022/9/6.
//

#ifndef HTTP_SERVER_SERVER_H
#define HTTP_SERVER_SERVER_H

#include "connection.hpp"
#include "boost/asio.hpp"

namespace http {

    template<typename Request_Handle = request_handler, typename Response_Writer = response_writer>
    class server {
        public:
        using connection_type = connection<Request_Handle, Response_Writer>;

        explicit server(boost::asio::io_context& context, short port, const std::string& root)
            : _acceptor(context, tcp::endpoint(tcp::v4(), port)),
            _request_handler(root), _response_writer(), _context{ context } {
            // boost::asio::socket_base::reuse_address option{ true };
            // _acceptor.set_option(option);
            do_accept();
        }

        private:
        void do_accept() {
            _acceptor.async_accept([this](auto error, tcp::socket socket) {
                if (!error) {
                    std::make_shared<connection_type>(std::move(socket), _context, _request_handler, _response_writer)->start();
                }
            do_accept();
                });
        }

        tcp::acceptor _acceptor;
        request_handler _request_handler;
        response_writer _response_writer;
        boost::asio::io_context& _context;

    };


}


#endif //HTTP_SERVER_SERVER_H
