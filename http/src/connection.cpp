#include "http/connection.hpp"
#include "http/parser.hpp"
#include <algorithm>
#include <utility>
#include <chrono>
#include <iostream>

namespace http{

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

    task<void> response_writer(tcp_socket *socket, response res, const char* buffer, size_t size){
        std::string resheaders = response::status_to_string(res.status);
        asio::steady_timer timer{socket->get_executor()};

//        if(res.content.index() == 1){
//            res.headers["Content-Length"] = std::to_string(std::get<std::string>(res.content).size());
//        }else{
//        }
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

        auto&& ret = co_await socket->async_write_some(asio::buffer(resheaders));
        if(std::get<0>(ret) || std::get<1>(ret) < resheaders.size())
            co_return;

        if(timeout_ctl(timer, std::get<1>(ret)) > 0){
            timer.async_wait(timeout_handle);
        }else co_return;

        if(res.content.index() == 1){
            co_await socket->async_write_some(asio::buffer(std::get<1>(res.content)));
        }else{
            try{
                stream_file file{socket->get_executor(), std::get<2>(res.content).string(), boost::asio::file_base::read_only};
                while(true){
                    ret = co_await file.async_read_some(asio::buffer((void*)buffer, size));
                    if(std::get<0>(ret))
                        co_return ;

                    if(std::get<1>(ret) < size){
                        co_await socket->async_write_some(asio::buffer((void*)buffer, std::get<1>(ret)));
                        co_return;
                    }
                    ret = co_await socket->async_write_some(asio::buffer((void*)buffer, size));
                    if(std::get<0>(ret))
                        co_return;
                    if(std::get<1>(ret) < size)
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

}