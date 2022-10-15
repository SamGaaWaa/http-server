#include <iostream>
#include "include/connection.hpp"
#include "include/server.h"
#include <string>
#include <vector>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "args error.\n";
        return -1;
    }

    short port = std::atoi(argv[1]);
    std::string root = argv[2];
    boost::asio::io_context context;

    auto thread_nums = std::thread::hardware_concurrency();

    std::vector<std::thread> threads;
    http::server s{ context, port, root };


    for (auto i{ 0 }; i < thread_nums; ++i) {
        threads.emplace_back([&] {
            try {
            context.run();
        }
        catch (std::exception& e) {
            std::cout << "Exception: " << e.what() << '\n';
        }
            });
    }

    for (auto& t : threads)
        if (t.joinable())
            t.join();


    return 0;
}
