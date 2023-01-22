#include "http/parser.hpp"

namespace http{

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
        _req._raw_data.append(at, length);
        return http_parser_execute(&_parser, &_settings, (&_req._raw_data.back()) + 1 - length, length);

    }

    bool parser::finish() const noexcept {
        return _state == State::FINISH;
    }

    request parser::result() noexcept {
        return std::move(_req);
    }

}
