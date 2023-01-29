#ifndef AVSERVER_STREAM_FILE_HPP
#define AVSERVER_STREAM_FILE_HPP

// #include "boost/asio/thread_pool.hpp"
// #include "boost/asio/post.hpp"
// #include "boost/asio/dispatch.hpp"
// #include "boost/asio/use_awaitable.hpp"
// #include "boost/asio/as_tuple.hpp"

#include "http/config.hpp"

#include <string>
#include "stdio.h"

#define _WIN32_WINNT 0x0601

namespace http{

    struct file_base{
        enum flags{
            read_only,
            write_only,
            append,
            read_write
        };
    };

    class stream_file {
    public:
        explicit stream_file(const std::string&, file_base::flags);
        stream_file(const stream_file&)=delete;
        stream_file& operator=(const stream_file&)=delete;
        stream_file(stream_file&&)=default;
        stream_file& operator=(stream_file&&)=default;
        ~stream_file();

    public:
        template<class Buffer, class Token = default_token>
        auto async_read_some(const Buffer& buffer, Token token = {}) {
            auto init = [this, &buffer]<typename Handler>(Handler&& handler)mutable {
                boost::asio::post(_pool, [this, &buffer, handler = std::forward<Handler>(handler)]()mutable {
                    const auto &ex = boost::asio::get_associated_executor(handler);
                    auto ret = ::fread(buffer.data(), 1, buffer.size(), _file);
                    if(ret == buffer.size()){
                        boost::asio::dispatch(ex, [ret, handler = std::move(handler)]()mutable {
                            handler(boost::system::error_code{}, ret);
                        });
                    }else if(::feof(_file)){
                        boost::asio::dispatch(ex, [ret, handler = std::move(handler)]()mutable {
                            handler(boost::system::error_code{}, ret);
                        });
                    }else{
                        boost::asio::dispatch(ex, [this, ret, handler = std::move(handler)]()mutable {
                            handler(boost::system::error_code(::ferror(_file), boost::system::system_category()), ret);
                        });
                    }
                });
            };
            return boost::asio::async_initiate<Token, void(boost::system::error_code, std::size_t)>(init, token);
        }


        template<class Buffer, class Token>
        auto async_write_some(Buffer& buffer, Token&& token = {}) {
            auto init = [this, &buffer]<typename Handler>(Handler&& handler)mutable {
                boost::asio::post(_pool, [this, &buffer, handler = std::forward<Handler>(handler)]()mutable {
                    const auto &ex = boost::asio::get_associated_executor(handler);
                    auto ret = ::fwrite(buffer.data(), 1, buffer.size(), _file);
                    if(ret == buffer.size()){
                        boost::asio::dispatch(ex, [ret, handler = std::move(handler)]()mutable {
                            handler(boost::system::error_code{}, ret);
                        });
                    }else{
                        boost::asio::dispatch(ex, [this, ret, handler = std::move(handler)]()mutable {
                            handler(boost::system::error_code(::ferror(_file), boost::system::system_category()), ret);
                        });
                    }
                });
            };
            return boost::asio::async_initiate<Token, void(boost::system::error_code, std::size_t)>(init, std::forward<Token>(token));
        }

    private:
        FILE *_file;
        static boost::asio::thread_pool _pool;
    };

}

#endif //AVSERVER_STREAM_FILE_HPP
