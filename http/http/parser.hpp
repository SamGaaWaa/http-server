#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include "http_parser.h"
#include "http/request.hpp"

#include <optional>
#include <concepts>
#include <string_view>

namespace http{
    struct parser{
        parser(std::pmr::memory_resource *)noexcept;
        int parse_tail(const char *, size_t)noexcept;
        request result()noexcept;
        [[nodiscard]] bool finish()const noexcept;
        std::string_view get_buffer(size_t);
        void reserve(size_t);
    private:
        http_parser _parser{};
        request _req;
        http_parser_settings _settings{};

        enum struct State{
            START,
            P_URL,
            P_HEADER_FIELD,
            P_HEADER_VALUE,
            P_BODY,
            FINISH
        };
        State _state = State::START;
        request::header _header{};

    private:
        static request::Method get_method(unsigned int method)noexcept{
            switch (method) {
                case HTTP_GET:
                    return request::Method::GET;
                case HTTP_POST:
                    return request::Method::POST;
                case HTTP_HEAD:
                    return request::Method::HEAD;
                case HTTP_PUT:
                    return request::Method::PUT;
                case HTTP_DELETE:
                    return request::Method::DEL;
                default:
                    return request::Method::GET;
            }
        }

        static int on_url(http_parser *parser, const char *at, size_t length){
            auto ptr = static_cast<class parser*>(parser->data);
            if(ptr->_state == State::START){
                ptr->_req._url = {at - ptr->_req._raw_data.data(), at - ptr->_req._raw_data.data() + length};
                ptr->_state = State::P_URL;
            }
            else ptr->_req._url.second += length;
            return 0;
        }

        static int on_body(http_parser *parser, const char *at, size_t length){
            auto ptr = static_cast<class parser*>(parser->data);
            if(ptr->_state != State::P_BODY){
                ptr->_req._body = {at - ptr->_req._raw_data.data(), at - ptr->_req._raw_data.data() + length};
                ptr->_state = State::P_BODY;
            }
            else ptr->_req._body.second += length;
            return 0;
        }

        static int on_header_field(http_parser *parser, const char *at, size_t length){
            auto ptr = static_cast<class parser*>(parser->data);
            if(ptr->_state == parser::State::P_URL){
                ptr->_header.first = {at - ptr->_req._raw_data.data(), at - ptr->_req._raw_data.data() + length};
                ptr->_state = parser::State::P_HEADER_FIELD;
            }else if(ptr->_state == parser::State::P_HEADER_FIELD){
                ptr->_header.first.second += length;
            }else{
                ptr->_req._headers.push_back(std::move(ptr->_header));
                ptr->_header.first = {at - ptr->_req._raw_data.data(), at - ptr->_req._raw_data.data() + length};
                ptr->_state = parser::State::P_HEADER_FIELD;
            }
            return 0;
        }

        static int on_header_value(http_parser *parser, const char *at, size_t length){
            auto ptr = static_cast<class parser*>(parser->data);
            if(ptr->_state == parser::State::P_URL){
                return -1;
            }else if(ptr->_state == parser::State::P_HEADER_FIELD){
                ptr->_header.second = {at - ptr->_req._raw_data.data(), at - ptr->_req._raw_data.data() + length};
                ptr->_state = parser::State::P_HEADER_VALUE;
            }else{
                ptr->_header.second.second += length;
            }
            return 0;
        }

        static int on_headers_complete(http_parser *parser){
            auto ptr = static_cast<class parser*>(parser->data);
            if(ptr->_parser.upgrade == 1){
                ptr->_req._upgrade = true;
                return 0;
            }
            if(ptr->_state != parser::State::P_HEADER_VALUE)
                return -1;
            ptr->_req._headers.push_back(std::move(ptr->_header));
            auto data = ptr->_req._raw_data.data();
            const auto compare = [data](const request::header& x, const request::header& y){
                std::string_view xv = {data + x.first.first, data + x.first.second};
                std::string_view yv = {data + y.first.first, data + y.first.second};
                return xv < yv;
            };
            const auto equal = [data](const request::header& x, const request::header& y){
                std::string_view xv = {data + x.first.first, data + x.first.second};
                std::string_view yv = {data + y.first.first, data + y.first.second};
                return xv == yv;
            };
            std::sort(ptr->_req._headers.begin(), ptr->_req._headers.end(), compare);
            auto last = std::unique(ptr->_req._headers.begin(), ptr->_req._headers.end(), equal);
            ptr->_req._headers.erase(last, ptr->_req._headers.end());
            return 0;
        }

        static int on_message_complete(http_parser *parser)noexcept{
            auto ptr = static_cast<class parser*>(parser->data);
            ptr->_state = parser::State::FINISH;
            ptr->_req.method = get_method(parser->method);
            auto data = ptr->_req._raw_data.data();
            ptr->_req.url = std::string_view{data + ptr->_req._url.first, data + ptr->_req._url.second};
            ptr->_req.body = std::string_view{data + ptr->_req._body.first, data + ptr->_req._body.second};
            return 0;
        }
    };

}

#endif //HTTP_PARSER_HPP
