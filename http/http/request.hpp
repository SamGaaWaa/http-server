#ifndef MEDIASERVER_REQUEST_HPP
#define MEDIASERVER_REQUEST_HPP

#include <string>
#include "boost/container/flat_map.hpp"


namespace http{
    struct request{
        using header_map = boost::container::flat_map<std::string, std::string>;
        using header = header_map::value_type;
        using header_map_iterator = header_map::iterator;

        enum struct Method: char{
            GET,
            POST,
            DEL,
            PUT,
            HEAD
        };

        Method method;
        std::string url;
        header_map headers;
        std::string body;
    };
}

#endif //MEDIASERVER_REQUEST_HPP
