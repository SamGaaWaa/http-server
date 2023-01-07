#include "http/server.hpp"
#include "http/connection.hpp"
#include <array>
#include <algorithm>

namespace http {
    constexpr size_t http_buffer_size = 1 << 16; //65536

    server::server(short port, size_t ths):
        _acceptor{ _context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) },
        _ths{ ths },
        _pool{ ths - 1 } {

    }

    void server::get(const std::string& path, handle_type&& handle) {
        _matchers.emplace_back(pattern(path), request::Method::GET, std::make_shared<handle_type>(std::move(handle)));
    }

    void server::post(const std::string& path, handle_type&& handle) {
        _matchers.emplace_back(pattern(path), request::Method::POST, std::make_shared<handle_type>(std::move(handle)));
    }

    server::handle_type* server::route(const request& req) {
        auto iter = _map.find({ req.url, req.method });
        if (iter != _map.end())
            return iter->second.get();
        auto match = std::find_if(_matchers.begin(), _matchers.end(), [&req](const auto& item) -> bool {
            return std::get<1>(item) == req.method and std::get<0>(item)(req.url);
            });
        if (match == _matchers.end())
            return nullptr;
        _map.insert({ std::tuple{req.url, req.method}, std::get<2>(*match) });
        return std::get<2>(*match).get();
    }

    void server::listen() {
        _pool.start();
        asio::co_spawn(_context, [this]() -> task<void> {
            while (true) {
                auto&& [err, s] = co_await _acceptor.async_accept();
                if (err)
                    continue;

                asio::co_spawn(_context, [this, socket = std::move(s)]()mutable -> task<void> {
                    std::array<char, http_buffer_size> buffer{};
                std::optional<request> req_op;
                request::header_map::iterator iter;
                do {
                    req_op = co_await get_request(&socket, buffer.data(), buffer.size());
                    if (!req_op.has_value())
                        co_return;

                    auto handle = route(*req_op);
                    if (handle == nullptr)
                        co_return;
                    if (_ths > 1) {
                        auto ret = co_await asio::co_spawn(*_pool.get_executor(), (*handle)(*req_op), default_token{});
                        if (std::get<0>(ret))
                            co_return;
                        co_await response_writer(&socket, std::move(std::get<1>(ret)), buffer.data(), buffer.size());
                    }
                    else {
                        try {
                            auto res = co_await(*handle)(*req_op);
                            co_await response_writer(&socket, std::move(res), buffer.data(), buffer.size());
                        }
                        catch (...) {
                            co_return;
                        }
                    }
                } while (0);
                    }, asio::detached);
            }
            }, asio::detached);
        _context.run();
    }

    asio::io_context& server::get_executor() {
        return _context;
    }


}