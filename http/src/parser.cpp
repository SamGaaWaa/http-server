#include "http/parser.hpp"

namespace http{

    parser::parser(std::pmr::memory_resource *resource) noexcept: _req{resource}{
        http_parser_init(&_parser, HTTP_REQUEST);
        _parser.data = this;
        _settings.on_url = on_url;
        _settings.on_header_field = on_header_field;
        _settings.on_header_value = on_header_value;
        _settings.on_headers_complete = on_headers_complete;
        _settings.on_body = on_body;
        _settings.on_message_complete = on_message_complete;
    }

    int parser::parse_tail(const char *at, size_t length) noexcept {
        _req._raw_data.resize(at + length - _req.data());
        return http_parser_execute(&_parser, &_settings, at, length);
    }

    bool parser::finish() const noexcept {
        return _state == State::FINISH;
    }

    request parser::result() noexcept {
        return std::move(_req);
    }


    std::string_view parser::get_buffer(size_t size) {
        auto &data = _req._raw_data;
        auto old_size = data.size();
        data.resize(old_size + size);
        return {data.data() + old_size, size};
    }

}
