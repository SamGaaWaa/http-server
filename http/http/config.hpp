#ifndef MEDIASERVER_CONFIG_HPP
#define MEDIASERVER_CONFIG_HPP

// #define __cpp_lib_coroutine
#define BOOST_ASIO_HAS_CO_AWAIT
#define _WIN32_WINNT 0x0601
#include <coroutine>
 
#if defined(__linux__) 
#define     BOOST_ASIO_HAS_IO_URING
#define     BOOST_ASIO_DISABLE_EPOLL
#include    "liburing.h"
#endif

#include "boost/asio.hpp"


namespace http {

    namespace asio = boost::asio;
    template<class T> using task = boost::asio::awaitable<T>;

    constexpr auto use_awaitable = asio::as_tuple(asio::use_awaitable);

}
#endif //MEDIASERVER_CONFIG_HPP
