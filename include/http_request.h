//
// Created by SamGaaWaa on 2022/9/2.
//

#ifndef HTTP_SERVER_HTTP_REQUEST_H
#define HTTP_SERVER_HTTP_REQUEST_H

#include <map>
#include <memory>
#include <optional>
#include <string>

namespace http {

    struct http_request {
        using header = std::pair<std::string, std::string>;
        using header_map = std::map<std::string, std::string>;


        enum {
            GET,
            POST,
            DEL,
            PUT,
            HEAD
        } method;
        std::string url;
        header_map headers;
        std::string body;

        private:
        std::unique_ptr<header> _tmp_ptr;
        enum {
            parsing_field, parsing_value, complete
        } state = parsing_field;

        friend class request_parser;
    };



}

#endif //HTTP_SERVER_HTTP_REQUEST_H
