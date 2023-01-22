#ifndef MEDIASERVER_REQUEST_HPP
#define MEDIASERVER_REQUEST_HPP

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <optional>
#include <cstring>
//#include "boost/container/flat_map.hpp"


namespace http{
    struct request{
        using view = std::pair<size_t, size_t>;
        using header = std::pair<view, view>;

        enum struct Method: char{
            GET,
            POST,
            DEL,
            PUT,
            HEAD
        };

        Method method;
        std::string_view url;
        std::string_view body;

        std::optional<std::string_view> find_header(const std::string&)const;
        std::optional<std::string_view> find_header(const std::string_view&)const noexcept;
        std::optional<std::string_view> find_header(const char*)const noexcept;

        const char *data()const noexcept;
        size_t size()const noexcept;
        bool is_upgrade()const noexcept;
    private:
        friend class parser;
        std::string _raw_data;
        std::vector<header> _headers;
        view _url;
        view _body;
        bool _upgrade{false};
    };
}

#endif //MEDIASERVER_REQUEST_HPP
