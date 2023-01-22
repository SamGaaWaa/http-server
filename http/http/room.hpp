#ifndef AVSERVER_ROUTER_HPP
#define AVSERVER_ROUTER_HPP

#include "boost/container/flat_set.hpp"
#include "boost/intrusive/avl_set.hpp"
#include "boost/uuid/uuid.hpp"
#include <string>
#include <memory>
#include <shared_mutex>

namespace http {
    class ws_stream;

    class room : public std::enable_shared_from_this<room>, public boost::intrusive::avl_set_base_hook<> {
        struct stream_compare{
            bool operator()(ws_stream*, ws_stream*)const noexcept;
            bool operator()(ws_stream*, const boost::uuids::uuid&)const noexcept;
            bool operator()(const boost::uuids::uuid&, ws_stream*)const noexcept;
        };

        boost::container::flat_set<ws_stream *, stream_compare> _streams;
        std::shared_mutex _m;
        std::string _name;

        friend bool operator<(const room &, const room &) noexcept;
        friend struct room_compare;

        struct room_compare{
            bool operator()(const std::string& name, const room& r)const noexcept{ return name < r._name; }
            bool operator()(const room& r, const std::string& name)const noexcept{ return r._name < name; }
        };

        using room_set_t = boost::intrusive::avl_set<room>;
        static room_set_t room_set;
        static std::shared_mutex room_set_mutex;
    public:
        explicit room(std::string);

        ~room();

        room(const room &) = delete;

        room &operator=(const room &) = delete;

        void publish(const std::shared_ptr<std::string> &);

        void send_to(const std::shared_ptr<std::string> &, const boost::uuids::uuid&);

        void join(ws_stream *);

        void leave(ws_stream *) noexcept;

        static std::shared_ptr<room> new_room(const std::string &);

        static std::shared_ptr<room> find_room(const std::string &);

    };

}

#endif //AVSERVER_ROUTER_HPP
