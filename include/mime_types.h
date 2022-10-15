//
// Created by SamGaaWaa on 2022/9/4.
//

#ifndef HTTP_SERVER_MIME_TYPES_H
#define HTTP_SERVER_MIME_TYPES_H

#include <map>
#include <string>

namespace http{
    namespace mime_types{

        const std::map<std::string, std::string> map{
                { ".gif", "image/gif" },
                { ".htm", "text/html" },
                { ".html", "text/html" },
                { ".jpg", "image/jpeg" },
                { ".png", "image/png" },
                { ".jpeg", "image/jpeg" }
        };


        std::string extension_to_type(const std::string& extension)
        {
            auto res = map.find(extension);
            if(res != map.end())
                return res->second;
            return "text/plain";
        }
    }
}


#endif //HTTP_SERVER_MIME_TYPES_H
