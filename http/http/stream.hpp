#ifndef HTTP_STREAM_HPP
#define HTTP_STREAM_HPP

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "http/config.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/beast/core/tcp_stream.hpp"
#include <boost/asio/dispatch.hpp>

#include <string>
#include <memory>
#include <deque>
#include <any>

namespace http {
    class room;

    namespace websocket = boost::beast::websocket;
    using ws_stream_t = boost::beast::websocket::stream<typename boost::beast::tcp_stream::rebind_executor<typename default_token::executor_with_default<asio::any_io_executor>>::other>;

    class ws_stream {
    public:
        using ws_stream_t = boost::beast::websocket::stream<typename boost::beast::tcp_stream::rebind_executor<typename default_token::executor_with_default<asio::any_io_executor>>::other>;

        explicit ws_stream(ws_stream_t) noexcept;
        ws_stream(const ws_stream&)=delete;
        ws_stream &operator=(const ws_stream&)=delete;
        ws_stream(ws_stream&&)=default;
        ~ws_stream();
        void send(const std::shared_ptr<std::string> &);
        void join(const std::string&);
        void leave();
        http::room &room()noexcept;
        const boost::uuids::uuid &id()const noexcept;

        template<class Type>
        auto get_property() {
            return std::any_cast<Type>(&_property);
        }

        template<class Buffer, class Token = default_token>
        auto async_read(Buffer &buffer, Token h = {}) {
            auto initiate = [this, &buffer]<typename Handler>(Handler&& handler)mutable {
                _stream.async_read(buffer, [handler=std::move(handler)](boost::system::error_code err, size_t)mutable{
                    auto ex = asio::get_associated_executor(handler);
                    boost::asio::dispatch(ex, [err, handler=std::move(handler)]()mutable {
                        handler(err);
                    });
                });
            };
            return asio::async_initiate<Token, void(boost::system::error_code)>(initiate, h);
        }

    private:
        using message_t = std::shared_ptr<std::string>;

        void join(http::room&);

        ws_stream_t _stream;
        std::deque<message_t> _q;
        std::any _property;
        std::shared_ptr<http::room> _room;
        boost::uuids::uuid _id;
        static boost::uuids::random_generator gen;
    };
}

#endif //HTTP_STREAM_HPP
