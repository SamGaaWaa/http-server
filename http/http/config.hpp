#ifndef MEDIASERVER_CONFIG_HPP
#define MEDIASERVER_CONFIG_HPP

#define __cpp_lib_coroutine
#define BOOST_ASIO_HAS_CO_AWAIT
#define _WIN32_WINNT 0x0601
#include <coroutine>
#undef __cpp_lib_coroutine

#ifdef __linux__
#define BOOST_ASIO_HAS_IO_URING
#define BOOST_ASIO_DISABLE_EPOLL
#include "liburing.h"
#endif

#include "boost/asio.hpp"




namespace http {

    namespace asio = boost::asio;
    template<class T> using task = boost::asio::awaitable<T>;

    constexpr auto use_awaitable = asio::as_tuple(asio::use_awaitable);

    using default_token = asio::as_tuple_t<asio::use_awaitable_t<>>;
    using tcp_socket = default_token::as_default_on_t<asio::ip::tcp::socket>;
    using tcp_acceptor = default_token::as_default_on_t<asio::ip::tcp::acceptor>;
    using stream_file = default_token::as_default_on_t<asio::stream_file>;
}
#endif //MEDIASERVER_CONFIG_HPP
