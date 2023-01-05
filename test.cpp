#include "http/server.hpp"

int main() {
    http::server server{8080};

    server.get("/hello", [](const http::request &) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok;
        res.content.emplace<std::string>("<h1>Hello world!</h1>");
        co_return res;
    });

    server.get("/.*\\.html", [](const http::request &req) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok;
        res.headers.insert({"Content-Type", "text/html"});
        res.content.emplace<std::filesystem::path>(std::string{R"(../public)"} + req.url);
        co_return res;
    });

    server.get("/.*\\.jpeg", [](const http::request &req) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok;
        res.headers.insert({"Content-Type", "image/jpeg"});
        res.content.emplace<std::filesystem::path>(std::string{R"(../public)"} + req.url);
        co_return res;
    });

    server.get("/.*\\.mp4", [](const http::request &req) -> http::task<http::response> {
        http::response res;
        res.status = http::status_type::ok;
        res.headers.insert({"Content-Type", "video/mp4"});
        res.content.emplace<std::filesystem::path>(std::string{R"(../public)"} + req.url);
        co_return res;
    });

    server.listen();

    return 0;
}
