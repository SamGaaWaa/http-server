//
// Created by SamGaaWaa on 2022/8/24.
//

#ifndef HTTP_SERVER_PARSER_HPP
#define HTTP_SERVER_PARSER_HPP

#include "http_parser.h"
#include "http_request.h"

namespace http{


    class request_parser{

    public:
        explicit request_parser(){
            http_parser_init(&_parser, HTTP_REQUEST);
            _parser.data = &_request;
            _request._tmp_ptr = std::make_unique<http_request::header>();
        }

        auto operator()(auto iter, size_t len){
            return http_parser_execute(&_parser, &_settings, (const char*)iter, len);
        }

        std::optional<http_request> result()noexcept{
            if(_request.state != http_request::complete)
                return std::nullopt;
            return std::move(_request);
        }

    private:
        static int on_url(http_parser* parser, const char *at, size_t length){
            auto req = (http_request*)(parser->data);
            req->url.append(at, length);
            return 0;
        }

        static int on_body(http_parser* parser, const char *at, size_t length){
            auto req = (http_request*)(parser->data);
            req->body.append(at, length);
            return 0;
        }

        static int on_header_field(http_parser* parser, const char *at, size_t length){
            auto req = (http_request*)(parser->data);
            if(req->state == http_request::parsing_field){
                req->_tmp_ptr->first.append(at, length);
            }else if(req->state == http_request::parsing_value){
                req->headers.insert( std::move(*req->_tmp_ptr) );
                req->_tmp_ptr = std::make_unique<http_request::header>();
                req->state = http_request::parsing_field;
                req->_tmp_ptr->first.append(at, length);
            }

            return 0;
        }

        static int on_header_value(http_parser* parser, const char *at, size_t length){
            auto req = (http_request*)(parser->data);
            req->state = http_request::parsing_value;
            req->_tmp_ptr->second.append(at, length);
            return 0;
        }

        static int on_headers_complete(http_parser* parser){
            auto req = (http_request*)(parser->data);
            req->headers.insert( std::move(*req->_tmp_ptr) );
            req->_tmp_ptr.reset(nullptr);
            return 0;
        }


        static auto get_method(enum http_method method)noexcept{
            switch (method) {
                case HTTP_GET:
                    return http_request::GET;
                case HTTP_POST:
                    return http_request::POST;
                case HTTP_HEAD:
                    return http_request::HEAD;
                case HTTP_PUT:
                    return http_request::PUT;
                case HTTP_DELETE:
                    return http_request::DEL;
                default:
                    return http_request::GET;
            }
        }

        static int on_message_complete(http_parser* parser)noexcept{
            auto req = (http_request*)(parser->data);
            req->state = http_request::complete;
            req->method = get_method(parser->method);
            return 0;
        }

        http_parser _parser{};
        http_request _request;


        constexpr static http_parser_settings _settings{
            .on_url = on_url,
            .on_header_field = on_header_field,
            .on_header_value = on_header_value,
            .on_headers_complete = on_headers_complete,
            .on_body = on_body,
            .on_message_complete = on_message_complete
        };
    };




}



#endif
