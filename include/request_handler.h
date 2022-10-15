//
// Created by SamGaaWaa on 2022/9/4.
//

#ifndef HTTP_SERVER_REQUEST_HANDLER_H
#define HTTP_SERVER_REQUEST_HANDLER_H

#include <string>
#include <string_view>
#include <utility>
#include <filesystem>
#include "http_response.h"
#include "http_request.h"
#include "mime_types.h"

namespace http {

    class request_handler {
        public:
        explicit request_handler(const std::string& root):_root{ root } {};
        request_handler(const request_handler&) = delete;
        request_handler& operator=(const request_handler&) = delete;

        http_response operator()(const http_request& request)const {
            namespace fs = std::filesystem;

            if (request.method != http_request::GET) {
                std::cout << "暂不支持非 Get 方法\n";
                return {};
            }

            fs::path path = _root.string() + request.url;
            http_response response;

            if (path.empty() || path.is_relative() || fs::is_directory(path) || !fs::exists(path)) {
                response.status = http_response::not_found;
                return response;
            }
            if ((fs::status(path).permissions() & fs::perms::others_read) == fs::perms::none) {
                response.status = http_response::forbidden;
                return response;
            }
            response.status = http_response::ok;


            auto file_size = fs::file_size(path);
            response.headers.insert({ {"Content-Length"}, std::to_string(file_size) });
            response.headers.insert({ {"Content-Type"},
                                         mime_types::extension_to_type(path.extension().string()) });
            response.headers.insert({ {"Connection"}, {"Keep-Alive"} });
            response.content = path.string();
            return response;
        }

        private:

        const std::filesystem::path _root;
    };



}


#endif //HTTP_SERVER_REQUEST_HANDLER_H
