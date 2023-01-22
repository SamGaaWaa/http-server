#include "http/server.hpp"

int main() {
    http::server server{ 8080, 8 }; //8 threads

    server.get("/hello", [ ](const http::request&) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok;
        res.content.emplace<std::string>("<h1>Hello world!</h1>");
        co_return res;
    });

    server.get("/.*\\.html", [ ](const http::request& req) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok;
        res.headers.insert({ "Content-Type", "text/html" });
        res.content.emplace<std::filesystem::path>(std::string{ R"(../public)" } + std::string{req.url});
        co_return res;
    });

    server.get("/.*\\.jpeg", [ ](const http::request& req) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok;
        res.headers.insert({ "Content-Type", "image/jpeg" });
        res.content.emplace<std::filesystem::path>(std::string{ R"(../public)" } + std::string{req.url});
        co_return res;
    });

    server.get("/.*\\.mp4", [ ](const http::request& req) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok;
        res.headers.insert({ "Content-Type", "video/mp4" });
        res.content.emplace<std::filesystem::path>(std::string{ R"(../public)" } + std::string{req.url});
        co_return res;
    });

    //Simulate CPU-intensive request. For exemple, /fib28.
    server.get("/fib.*", [ ](const http::request& req) -> http::task<http::response> {
        http::response res;
        struct cal {
            unsigned long long fib(int x) {
                if (x <= 0 or x > 94 or x == 1 or x == 2)
                    return 1;
                else return fib(x - 1) + fib(x - 2);
            }
        };
        cal c;
        res.status = http::status_type::ok;
        res.headers.insert({ "Content-Type", "text/plain" });
        try {
            int x = std::stoi(std::string{ req.url.begin() + 4, req.url.end() });
            res.content.emplace<std::string>(std::to_string(c.fib(x)));
        }
        catch (...) {
            res.content.emplace<std::string>("Invalid argument.");
        }
        co_return res;
    });

    server.websocket("/echo", [ ](http::ws_stream stream)->http::task<void> {
        while (true) {
            std::string msg;
            boost::asio::dynamic_string_buffer buffer{ msg, 1 << 16 };
            auto ret = co_await stream.async_read(buffer);
            if (std::get<0>(ret))
                co_return;
            stream.send(std::make_shared<std::string>(std::move(msg)));
        }
    });

    //subscribe-publish
    server.websocket("/chat-room", [ ](http::ws_stream stream)->http::task<void> {
        stream.join("chat"); //join in a room named "chat"
        while (true) {
            auto msg = std::make_shared<std::string>();
            msg->reserve(128);
            boost::asio::dynamic_string_buffer buffer{ *msg, 1 << 16 };
            auto ret = co_await stream.async_read(buffer);
            if(std::get<0>(ret))
                co_return;     //automatic call stream.leave() to leave room
            /*
             *  deal with message
             */
            stream.room().publish(msg);
        }
    });

    server.listen();

    return 0;
}
