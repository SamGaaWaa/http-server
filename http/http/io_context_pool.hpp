#ifndef AVSERVER_IO_CONTEXT_POOL_HPP
#define AVSERVER_IO_CONTEXT_POOL_HPP

#include <vector>
#include <list>
#include <thread>
#include <memory>
#include <atomic>
#include "config.hpp"

namespace http {

    class io_context_pool: asio::noncopyable {
        using executor_work_guard = asio::executor_work_guard<asio::io_context::executor_type>;

        io_context_pool(size_t ths);
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


#endif //AVSERVER_IO_CONTEXT_POOL_HPP
