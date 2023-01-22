#include "http/server.hpp"
#include "http/connection.hpp"
#include <array>
#include <algorithm>
#include <iostream>

namespace http {
    constexpr size_t http_buffer_size = 1 << 16; //64KB

    server::server(short port, size_t ths) :
            _acceptor{_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)},
            _ths{ths},
            _pool{ths - 1} {

    }

    void server::get(const std::string &path, handle_type &&handle) {
        _matchers.emplace_back(pattern(path), request::Method::GET, std::make_unique<handle_type>(std::move(handle)));
    }

    void server::post(const std::string &path, handle_type &&handle) {
        _matchers.emplace_back(pattern(path), request::Method::POST, std::make_unique<handle_type>(std::move(handle)));
    }

    void server::websocket(std::string_view path, websocket_handle &&handle) {
        _websocket_map.push_back({path, std::move(handle)});
    }

    server::handle_type *server::route(const request &req) {
        auto iter = _map.find({req.url, req.method});
        if (iter != _map.end())
            return iter->second;
        auto match = std::find_if(_matchers.begin(), _matchers.end(), [&req](const auto &item) -> bool {
            return std::get<1>(item) == req.method and std::get<0>(item)(req.url);
        });
        if (match == _matchers.end())
            return nullptr;
        return std::get<2>(*match).get();
    }

    void server::listen() {
        static constexpr auto comp = [](const websocket_pair& x, const websocket_pair& y){
            return x.first < y.first;
        };
        static constexpr auto equal = [](const websocket_pair& x, const websocket_pair& y){
            return x.first == y.first;
        };
        _pool.start();
        std::sort(_websocket_map.begin(), _websocket_map.end(), comp);
        auto last = std::unique(_websocket_map.begin(), _websocket_map.end(), equal);
        _websocket_map.erase(last, _websocket_map.end());
        asio::co_spawn(_context, [this]() -> task<void> {
            while (true) {
                auto &&[err, s] = co_await _acceptor.async_accept();
                if (err)
                    continue;
                asio::co_spawn(_context, _handle_connection(std::move(s)), asio::detached);
            }
        }, asio::detached);
        _context.run();
    }

    asio::io_context &server::get_executor() {
        return _context;
    }

    task<void> server::_handle_connection(tcp_socket socket) {
        std::array<char, http_buffer_size> buffer{};
        std::optional<request> req_op;

        req_op = co_await get_request(&socket, buffer.data(), buffer.size());
        if (!req_op.has_value())
            co_return;
        if(req_op->is_upgrade()){
            static constexpr auto comp = [](const websocket_pair& x, const websocket_pair& y){
                return x.first < y.first;
            };
            auto handle = std::lower_bound(_websocket_map.begin(), _websocket_map.end(), websocket_pair{req_op->url, {}}, comp);
            if(handle == _websocket_map.end() || handle->first != req_op->url)
                co_return;
            ws_stream::ws_stream_t stream{std::move(socket)};
            auto [err] = co_await stream.async_accept(asio::buffer(req_op->data(), req_op->size()), default_token{});
            if(err)
                co_return;
            asio::co_spawn(_ths > 1 ? *_pool.get_executor() : _context, handle->second(ws_stream{std::move(stream)}), asio::detached);
            co_return;
        }

        auto handle = route(*req_op);
        if (handle == nullptr)
            co_return;
        if (_ths > 1) {
            auto ret = co_await asio::co_spawn(*_pool.get_executor(), (*handle)(*req_op), default_token{});
            if (std::get<0>(ret))
                co_return;
            if (std::get<1>(ret).status == status_type::ok)
                _map.insert({std::tuple{req_op->url, req_op->method}, handle});
            co_await response_writer(&socket, std::move(std::get<1>(ret)), buffer.data(), buffer.size());
        } else {
            try {
                auto res = co_await (*handle)(*req_op);
                if (res.status == status_type::ok)
                    _map.insert({std::tuple{req_op->url, req_op->method}, handle});
                co_await response_writer(&socket, std::move(res), buffer.data(), buffer.size());
            }
            catch (...) {
                co_return;
            }
        }
    }

    task<void> server::_handle_websocket(tcp_socket socket, request req) {
        std::cout<<req.url<<'\n';
        co_return;
    }

}