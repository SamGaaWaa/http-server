#ifndef HTTP_IO_CONTEXT_POOL_HPP
#define HTTP_IO_CONTEXT_POOL_HPP

#include <vector>
#include <list>
#include <thread>
#include <memory>
#include <atomic>
#include "http/config.hpp"

namespace http {

    class io_context_pool: asio::noncopyable {
        public:
        using executor_work_guard = asio::executor_work_guard<asio::io_context::executor_type>;

        explicit io_context_pool(size_t ths);
        void start();
        void stop();
        asio::io_context* get_executor()noexcept;
        ~io_context_pool();
        private:
        std::vector<std::unique_ptr<asio::io_context>> _contexts;
        std::vector<std::thread> _threads;
        std::list<executor_work_guard> _guards;
        size_t _index{ 0 };
        std::atomic_bool _stop{ false };
    };

}


#endif //HTTP_IO_CONTEXT_POOL_HPP
