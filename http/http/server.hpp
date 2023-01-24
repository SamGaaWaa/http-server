#ifndef MEDIASERVER_SERVER_HPP
#define MEDIASERVER_SERVER_HPP

#include "boost/container/flat_map.hpp"
#include "http/config.hpp"
#include "http/response.hpp"
#include "http/request.hpp"
#include "http/io_context_pool.hpp"
#include "http/stream.hpp"

#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <variant>

namespace http {

    class server {
        public:
        using sync_handle = std::function<response(const request&)>;
        using async_handle = std::function<task<response>(const request&)>;
        using handle_type = std::variant<sync_handle, async_handle>;
        using handle_ptr = std::unique_ptr<handle_type>;
        using websocket_handle = std::function<task<void>(ws_stream)>;

        server(short port = 80, size_t ths = 1);

        void get(const std::string&, async_handle&&);
        void get(const std::string&, sync_handle&&);
        void post(const std::string&, async_handle&&);
        void post(const std::string&, sync_handle&&);
        void websocket(std::string_view, websocket_handle&&);
        void listen();
        asio::io_context& get_executor();

        private:
        friend class ws_stream;
        using route_map = boost::container::flat_map<std::tuple<std::string_view, request::Method>, handle_type*>;
        using matcher = std::function<bool(const std::string_view&)>;
        using matcher_pair = std::tuple<matcher, request::Method, handle_ptr>;
        using websocket_pair = std::pair<std::string_view, websocket_handle>;
        using websocket_map = std::vector<websocket_pair>;

        task<void> _handle_connection(tcp_socket);

        handle_type* route(const request&);
        route_map _map;
        websocket_map _websocket_map;
        std::deque<matcher_pair> _matchers;
        asio::io_context _context{};
        tcp_acceptor _acceptor;
        size_t _ths;
        std::shared_mutex _m;
        io_context_pool _pool;
    };

}

#endif //MEDIASERVER_SERVER_HPP
