#ifndef MEDIASERVER_SERVER_HPP
#define MEDIASERVER_SERVER_HPP

#include "boost/container/flat_map.hpp"
#include "http/config.hpp"
#include "http/response.hpp"
#include "http/request.hpp"
#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <optional>
#include <regex>
#include <shared_mutex>
#include <thread>

namespace http{

    static inline auto pattern(const std::string& str){
        return [re = std::regex{str}](const std::string& s){
            return std::regex_match(s, re);
        };
    }

    class server{
    public:
        using handle_type = std::function<task<response>(const request&)>;
        using handle_ptr = std::shared_ptr<handle_type>;
        using route_map = boost::container::flat_map<std::tuple<std::string, request::Method>, handle_ptr>;
        using matcher = std::function<bool(const std::string&)>;
        using matcher_pair = std::tuple<matcher, request::Method, handle_ptr>;

        server(short port = 80, int ths = 1): _acceptor{_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)}, _ths{ths}{}

        void get(const std::string&, handle_type&&);
        void post(const std::string&, handle_type&&);
        void listen();
        asio::io_context &get_executor();
    private:
        handle_type *route(const request&);
        route_map _map;
        std::deque<matcher_pair> _matchers;
        asio::io_context _context{};
        asio::ip::tcp::acceptor _acceptor;
        int _ths;
        std::shared_mutex _m;
    };


}

#endif //MEDIASERVER_SERVER_HPP
