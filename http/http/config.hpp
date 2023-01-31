#ifndef HTTP_CONFIG_HPP
#define HTTP_CONFIG_HPP

#define BOOST_ASIO_HAS_CO_AWAIT
#define _WIN32_WINNT 0x0601
#include <coroutine>

#ifdef __linux__
#include "linux/version.h"

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0)
#define BOOST_ASIO_HAS_IO_URING
#define BOOST_ASIO_DISABLE_EPOLL
#include "liburing.h"
#else
#define USE_THREAD_POOL
#endif

#endif //__linux__

#include <boost/asio/awaitable.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>


namespace http {

    namespace asio = boost::asio;
    template<class T> using task = boost::asio::awaitable<T>;

    using default_token = asio::as_tuple_t<asio::use_awaitable_t<>>;
    using tcp_socket = default_token::as_default_on_t<asio::ip::tcp::socket>;
    using tcp_acceptor = default_token::as_default_on_t<asio::ip::tcp::acceptor>;

}

#endif //HTTP_CONFIG_HPP
