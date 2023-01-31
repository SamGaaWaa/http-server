#include "http/server.hpp"
#include "http/parser.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/detached.hpp"
#include <array>
#include <algorithm>
#include <regex>
#include <iostream>

#ifdef USE_THREAD_POOL
#include "http/file.hpp"
#else
#include "boost/asio/stream_file.hpp"
namespace http{
    using stream_file = default_token::as_default_on_t<boost::asio::stream_file>;
    using file_base = boost::asio::file_base;
}
#endif

namespace http {
    constexpr size_t http_buffer_size = 1 << 16; //64KB

    thread_local server::route_map server::_map{};

    static auto pattern(const std::string &str) {
        return [re = std::regex{str}](const std::string_view &s) {
            return std::regex_match(s.begin(), s.end(), re);
        };
    }

    static auto timeout_ctl(asio::steady_timer &timer, size_t bytes){
        auto now = std::chrono::steady_clock::now();
        auto begin = timer.expiry() - std::chrono::seconds{10};
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() ;
        auto v = static_cast<double>(bytes) / time;
        if(v < 0.01){
            return timer.expires_after(std::chrono::milliseconds{time / 2});
        }
        return timer.expires_after(std::chrono::seconds{10});
    }


    task<std::optional<request>> get_request(tcp_socket *socket, const char *buffer, size_t size){
        parser p;
        asio::steady_timer timer{socket->get_executor()};
        timer.expires_after(std::chrono::seconds(10));

        auto timeout_handle = [socket](boost::system::error_code err){
            if(err != asio::error::operation_aborted){
                socket->close();
            }
        };

        timer.async_wait(timeout_handle);
        while (true){
            auto [err, n] = co_await socket->async_read_some(asio::buffer((void*)buffer, size));
            if(err)
                co_return std::nullopt;

            if(timeout_ctl(timer, n) > 0){
                timer.async_wait(timeout_handle);
            }else{
                co_return std::nullopt;
            }

            p.parse(buffer, n);
            if(p.finish())
                co_return p.result();
            if(n < size)
                co_return std::nullopt;
        }
    }


    task<response> handle_request(request){
        co_return response{};
    }

    struct response_writer_t{
        task<void> operator()(tcp_socket *socket, response res, const char* buffer, size_t size){
            std::string resheaders = response::status_to_string(res.status);
            asio::steady_timer timer{ socket->get_executor() };
            const auto& ex = socket->get_executor();

            for(const auto& [k, v]: res.headers){
                resheaders.append(k);
                resheaders.append(": ");
                resheaders.append(v);
                resheaders.append("\r\n");
            }
            resheaders.append("\r\n");

            timer.expires_after(std::chrono::seconds{10});
            auto timeout_handle = [socket](boost::system::error_code err){
                if(err != asio::error::operation_aborted){
                    socket->close();
                }
            };
            timer.async_wait(timeout_handle);

            auto&& ret = co_await asio::async_write(*socket, asio::buffer(resheaders), default_token{});
            if(std::get<0>(ret))
                co_return;

            if(timeout_ctl(timer, std::get<1>(ret)) > 0){
                timer.async_wait(timeout_handle);
            }else co_return;

            if(res.content.index() == 1){
                co_await asio::async_write(*socket, asio::buffer(std::get<1>(res.content)), default_token{});
            }else{
                try {
#ifdef USE_THREAD_POOL
                    stream_file file{ std::get<2>(res.content).string(), file_base::read_only };
#else
                    stream_file file{ ex, std::get<2>(res.content).string(), file_base::read_only };

#endif
                    while (true) {
                        ret = co_await file.async_read_some(asio::buffer((void*)buffer, size));
                        if (std::get<0>(ret)) {
                            co_return;
                        }

                        if(std::get<1>(ret) < size){
                            co_await asio::async_write(*socket, asio::buffer((void*)buffer, std::get<1>(ret)), default_token{});
                            co_return;
                        }
                        auto [err, n] = co_await asio::async_write(*socket, asio::buffer((void*)buffer, size), default_token{});
                        if(err)
                            co_return;

                        if(timeout_ctl(timer, std::get<1>(ret)) > 0){
                            timer.async_wait(timeout_handle);
                        }else co_return;
                    }
                }catch(...){
                    std::cerr<<std::get<2>(res.content).string()<<" can't open.\n";
                }
            }
        }
    };

    template<class Func, class Token = default_token>
    static auto async_post(asio::io_context &executor, Func &&func, Token token = {}) {
        auto init = [func = std::forward<Func>(func), &executor]<typename Handler>(Handler &&handler)mutable {
            asio::post(executor, [func = std::move(func), handler = std::forward<Handler>(handler)]()mutable {
                auto ex = asio::get_associated_executor(handler);
                asio::dispatch(ex, [res = func(), handler = std::move(handler)]()mutable {
                    handler(std::move(res));
                });
            });
        };
        return asio::async_initiate<Token, void(response &&)>(init, token);
    }

    server::server(short port, size_t ths) :
            _acceptor{_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)},
            _ths{ths},
            _pool{ths - 1} {

    }

    void server::get(const std::string &path, async_handle &&handle) {
        _matchers.emplace_back(pattern(path), request::Method::GET, std::make_unique<handle_type>(std::move(handle)));
    }

    void server::get(const std::string &path, sync_handle &&handle) {
        _matchers.emplace_back(pattern(path), request::Method::GET, std::make_unique<handle_type>(std::move(handle)));
    }

    void server::post(const std::string &path, async_handle &&handle) {
        _matchers.emplace_back(pattern(path), request::Method::POST, std::make_unique<handle_type>(std::move(handle)));
    }

    void server::post(const std::string &path, sync_handle &&handle) {
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
        static constexpr auto comp = [](const websocket_pair &x, const websocket_pair &y) {
            return x.first < y.first;
        };
        static constexpr auto equal = [](const websocket_pair &x, const websocket_pair &y) {
            return x.first == y.first;
        };
        _pool.start();
        std::sort(_websocket_map.begin(), _websocket_map.end(), comp);
        auto last = std::unique(_websocket_map.begin(), _websocket_map.end(), equal);
        _websocket_map.erase(last, _websocket_map.end());
        asio::co_spawn(_context, [this]() -> task<void> {
            while (true) {
                tcp_socket socket{_ths > 1 ? *_pool.get_executor() : _context};
                auto [err] = co_await _acceptor.async_accept(socket, default_token{});
                if(err)
                    continue;
                const auto &ex = socket.get_executor();
                asio::co_spawn(ex, _handle_connection(std::move(socket)), asio::detached);
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
        const auto &ex = socket.get_executor();

        req_op = co_await get_request(&socket, buffer.data(), buffer.size());
        if (!req_op.has_value())
            co_return;
        if (req_op->is_upgrade()) {
            static constexpr auto comp = [](const websocket_pair &x, const websocket_pair &y) {
                return x.first < y.first;
            };
            auto handle = std::lower_bound(_websocket_map.begin(), _websocket_map.end(),
                                           websocket_pair{req_op->url, {}}, comp);
            if (handle == _websocket_map.end() || handle->first != req_op->url)
                co_return;
            ws_stream::ws_stream_t stream{std::move(socket)};
            auto [err] = co_await stream.async_accept(asio::buffer(req_op->data(), req_op->size()), default_token{});
            if (err)
                co_return;
            asio::co_spawn(ex, handle->second(ws_stream{std::move(stream)}),
                           asio::detached);
            co_return;
        }

        auto handle = route(*req_op);
        if (handle == nullptr)
            co_return;

        if (_ths > 1) {
            if (handle->index() == 0) {
                auto res = std::get<0>(*handle)(*req_op);
                if (res.status == status_type::ok) {
                    _map.insert({std::tuple{req_op->url, req_op->method}, handle});
                }
                co_await response_writer_t{}(&socket, std::move(res), buffer.data(), buffer.size());
                co_return;
            }
            auto ret = co_await asio::co_spawn(ex, std::get<1>(*handle)(*req_op), default_token{});
            if (std::get<0>(ret))
                co_return;
            if (std::get<1>(ret).status == status_type::ok)
                _map.insert({std::tuple{req_op->url, req_op->method}, handle});
            co_await response_writer_t{}(&socket, std::move(std::get<1>(ret)), buffer.data(), buffer.size());
        } else {
            try {
                std::optional<response> res_op;
                if (handle->index() == 0) {
                    res_op = std::get<0>(*handle)(*req_op);
                } else res_op = co_await (std::get<1>(*handle))(*req_op);
                if ((*res_op).status == status_type::ok)
                    _map.insert({std::tuple{req_op->url, req_op->method}, handle});
                co_await response_writer_t{}(&socket, std::move(*res_op), buffer.data(), buffer.size());
            }
            catch (...) {
                co_return;
            }
        }
    }

}