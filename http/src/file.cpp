#include "http/file.hpp"
#include <thread>

namespace http{

    boost::asio::thread_pool stream_file::_pool{std::thread::hardware_concurrency() * 2};

    static const char *asio_file_flags_to_std(file_base::flags flag)noexcept{
        switch (flag) {
            case file_base::read_only:
                return "rb";
            case file_base::write_only:
                return "wb";
            case file_base::append:
                return "ab";
            case file_base::read_write:
                return "w+b";
            default:
                return "";
        }
    }

    stream_file::stream_file(const std::string &path, file_base::flags flag){
        _file = ::fopen(path.c_str(), asio_file_flags_to_std(flag));
        if(!_file)
            throw std::runtime_error{"File open error: "};
    }

    stream_file::stream_file(stream_file&& other)noexcept:_file{other._file}{
        other._file = nullptr;
    }

    stream_file &stream_file::operator=(stream_file&& other)noexcept{
        _file = other._file;
        other._file = nullptr;
        return *this;
    }

    stream_file::~stream_file() {
        if(_file)
            ::fclose(_file);
    }


}
