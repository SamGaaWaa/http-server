#ifndef MEDIASERVER_PARSER_HPP
#define MEDIASERVER_PARSER_HPP

#include "http_parser.h"
#include "http/request.hpp"

#include <optional>

namespace http{
    struct parser{
        parser()noexcept;
        int parse(const char *, size_t);
        request result()noexcept;
        bool finish()const noexcept;

        http_parser _parser{};
        request _req;
        http_parser_settings _settings{};

        enum struct State{
            NORMAL,
            P_HEADER_FIELD,
            P_HEADER_VALUE,
            FINISH
        };
        State _state = State::NORMAL;
        request::header _header{};
    };
}

#endif //MEDIASERVER_PARSER_HPP
