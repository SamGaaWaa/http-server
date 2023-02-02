#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <optional>
#include <cstring>
#include <memory_resource>

namespace http{
    struct request{
        using view = std::pair<size_t, size_t>;
        using header = std::pair<view, view>;

        using allocator_type = std::pmr::polymorphic_allocator<char>;

        request(allocator_type alloc = {})noexcept;
        request(request&&)noexcept=default;
        request(request&&, allocator_type)noexcept;
        request& operator=(request&&)noexcept;


        request(const request&)=delete;
        request& operator=(const request&)=delete;

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

        [[nodiscard]] std::optional<std::string_view> find_header(const std::string&)const;
        [[nodiscard]] std::optional<std::string_view> find_header(const std::string_view&)const noexcept;
        std::optional<std::string_view> find_header(const char*)const noexcept;

        [[nodiscard]] const char *data()const noexcept;
        size_t size()const noexcept;
        bool is_upgrade()const noexcept;
    private:
        friend class parser;
        std::pmr::string _raw_data;
        std::pmr::vector<header> _headers;
        view _url;
        view _body;
        bool _upgrade{false};
    };
}

#endif //HTTP_REQUEST_HPP
