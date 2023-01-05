#ifndef MEDIASERVER_CONNECTION_HPP
#define MEDIASERVER_CONNECTION_HPP

#include "http/config.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include <optional>

namespace http{

task<std::optional<request>> get_request(asio::ip::tcp::socket *, const char *, size_t);

task<response> handle_request(request);

task<void> response_writer(asio::ip::tcp::socket *, response, const char*, size_t);

}


#endif //MEDIASERVER_CONNECTION_HPP
