#include "http/request.hpp"

namespace http{
    std::optional<std::string_view> request::find_header(const std::string_view& field) const noexcept {
        size_t lo = 0;
        size_t hi = _headers.size();
        const char *data = _raw_data.data();

        while(lo < hi){
            const auto mid = (lo + hi) / 2;
            const auto &header = _headers[mid];
            std::string_view sv = {data + header.first.first, data + header.first.second};
            if(sv == field)
                return std::string_view{data + header.second.first, data + header.second.second};
            else if(sv < field)
                lo = mid + 1;
            else hi = mid;
        }
        return std::nullopt;
    }

    std::optional<std::string_view> request::find_header(const std::string &field) const {
        return request::find_header(std::string_view{field.begin(), field.end()});
    }

    std::optional<std::string_view> request::find_header(const char *field) const noexcept {
        return request::find_header(std::string_view{field, std::strlen(field)});
    }

    const char *request::data() const noexcept {
        return _raw_data.data();
    }

    size_t request::size() const noexcept {
        return _raw_data.size();
    }

    bool request::is_upgrade() const noexcept {
        return _upgrade;
    }

    request::request(allocator_type alloc)noexcept: _raw_data{alloc}, _headers{alloc}{}

    request::request(request &&other, allocator_type alloc) noexcept: _raw_data{std::move(other._raw_data), alloc},
                                                                        _headers{std::move(other._headers), alloc}{}

    request &request::operator=(request&& other)noexcept {
        _raw_data = std::move(other._raw_data);
        _headers = std::move(other._headers);
        return *this;
    }
}