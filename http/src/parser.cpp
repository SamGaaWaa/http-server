#include "http/parser.hpp"

namespace http{
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
        ptr->_req.url.append(at, length);
        return 0;
    }

    static int on_body(http_parser *parser, const char *at, size_t length){
        auto ptr = static_cast<class parser*>(parser->data);
        ptr->_req.body.append(at, length);
        return 0;
    }

    static int on_header_field(http_parser *parser, const char *at, size_t length){
        auto ptr = static_cast<class parser*>(parser->data);
        if(ptr->_state == parser::State::NORMAL){
            ptr->_header.first.append(at, length);
            ptr->_state = parser::State::P_HEADER_FIELD;
        }else if(ptr->_state == parser::State::P_HEADER_FIELD){
            ptr->_header.first.append(at, length);
        }else{
            ptr->_req.headers.insert(std::move(ptr->_header));
            ptr->_header.first.append(at, length);
            ptr->_state = parser::State::P_HEADER_FIELD;
        }
        return 0;
    }

    static int on_header_value(http_parser *parser, const char *at, size_t length){
        auto ptr = static_cast<class parser*>(parser->data);
        if(ptr->_state == parser::State::NORMAL){
            return -1;
        }else if(ptr->_state == parser::State::P_HEADER_FIELD){
            ptr->_header.second.append(at, length);
            ptr->_state = parser::State::P_HEADER_VALUE;
        }else{
            ptr->_header.second.append(at, length);
        }
        return 0;
    }

    static int on_headers_complete(http_parser *parser){
        auto ptr = static_cast<class parser*>(parser->data);
        if(ptr->_state != parser::State::P_HEADER_VALUE)
            return -1;
        ptr->_req.headers.insert(std::move(ptr->_header));
        return 0;
    }

    static int on_message_complete(http_parser *parser)noexcept{
        auto ptr = static_cast<class parser*>(parser->data);
        ptr->_state = parser::State::FINISH;
        ptr->_req.method = get_method(parser->method);
        return 0;
    }

    parser::parser() noexcept{
        http_parser_init(&_parser, HTTP_REQUEST);
        _parser.data = this;
        _settings.on_url = on_url;
        _settings.on_header_field = on_header_field;
        _settings.on_header_value = on_header_value;
        _settings.on_headers_complete = on_headers_complete;
        _settings.on_body = on_body;
        _settings.on_message_complete = on_message_complete;
    }

    int parser::parse(const char *at, size_t length) {
        return http_parser_execute(&_parser, &_settings, at, length);
    }

    bool parser::finish() const noexcept {
        return _state == State::FINISH;
    }

    request parser::result() noexcept {
        return std::move(_req);
    }




}
