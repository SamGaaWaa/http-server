#include "http/io_context_pool.hpp"

namespace {

    io_context_pool::io_context_pool(size_t ths) {
        for (auto i{ 0 }; i < ths; ++i) {
            _contexts.emplace_back(std::make_unique<asio::io_context>());
        }
    }

    void io_context_pool::start() {
        for (auto& context : _contexts) {
            _guards.emplace_back(asio::make_work_guard(context));
            _threads.emplace_back([&context] {
                context->run();
                });
        }
    }

    void io_context_pool::stop() {
        for (auto& g : _guards)
            g.reset();
        _stop = true;
    }

    io_context_pool::~io_context_pool() {
        if (!_stop)
            stop();
        for (auto& th : _threads)
            if (th.joinable())
                th.join();
    }

    asio::io_context* io_context_pool::get_executor() noexcept {
        return _contexts[_index++ % _contexts.size()].get();
    }

}