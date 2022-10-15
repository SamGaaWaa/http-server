//
// Created by SamGaaWaa on 2022/9/2.
//

#ifndef HTTP_SERVER_HTTP_RESPONSE_H
#define HTTP_SERVER_HTTP_RESPONSE_H

#include <utility>
#include <string>
#include <map>

namespace http {
    namespace status_strings {

        const std::string ok =
            "HTTP/1.0 200 OK\r\n";
        const std::string created =
            "HTTP/1.0 201 Created\r\n";
        const std::string accepted =
            "HTTP/1.0 202 Accepted\r\n";
        const std::string no_content =
            "HTTP/1.0 204 No Content\r\n";
        const std::string multiple_choices =
            "HTTP/1.0 300 Multiple Choices\r\n";
        const std::string moved_permanently =
            "HTTP/1.0 301 Moved Permanently\r\n";
        const std::string moved_temporarily =
            "HTTP/1.0 302 Moved Temporarily\r\n";
        const std::string not_modified =
            "HTTP/1.0 304 Not Modified\r\n";
        const std::string bad_request =
            "HTTP/1.0 400 Bad Request\r\n";
        const std::string unauthorized =
            "HTTP/1.0 401 Unauthorized\r\n";
        const std::string forbidden =
            "HTTP/1.0 403 Forbidden\r\n";
        const std::string not_found =
            "HTTP/1.0 404 Not Found\r\n";
        const std::string internal_server_error =
            "HTTP/1.0 500 Internal Server Error\r\n";
        const std::string not_implemented =
            "HTTP/1.0 501 Not Implemented\r\n";
        const std::string bad_gateway =
            "HTTP/1.0 502 Bad Gateway\r\n";
        const std::string service_unavailable =
            "HTTP/1.0 503 Service Unavailable\r\n";
    }

    struct http_response {

        enum status_type {
            ok = 200,
            created = 201,
            accepted = 202,
            no_content = 204,
            multiple_choices = 300,
            moved_permanently = 301,
            moved_temporarily = 302,
            not_modified = 304,
            bad_request = 400,
            unauthorized = 401,
            forbidden = 403,
            not_found = 404,
            internal_server_error = 500,
            not_implemented = 501,
            bad_gateway = 502,
            service_unavailable = 503
        } status;

        std::map<std::string, std::string> headers;
        std::string content;

        static std::string status_to_string(enum status_type status) {
            switch (status) {
                case ok:
                    return status_strings::ok;
                case created:
                    return status_strings::created;
                case accepted:
                    return status_strings::accepted;
                case no_content:
                    return status_strings::no_content;
                case multiple_choices:
                    return status_strings::multiple_choices;
                case moved_permanently:
                    return status_strings::moved_permanently;
                case moved_temporarily:
                    return status_strings::moved_temporarily;
                case not_modified:
                    return status_strings::not_modified;
                case bad_request:
                    return status_strings::bad_request;
                case unauthorized:
                    return status_strings::unauthorized;
                case forbidden:
                    return status_strings::forbidden;
                case not_found:
                    return status_strings::not_found;
                case internal_server_error:
                    return status_strings::internal_server_error;
                case not_implemented:
                    return status_strings::not_implemented;
                case bad_gateway:
                    return status_strings::bad_gateway;
                case service_unavailable:
                    return status_strings::service_unavailable;
                default:
                    return status_strings::bad_request;
            }
        }
    };
}


#endif //HTTP_SERVER_HTTP_RESPONSE_H
