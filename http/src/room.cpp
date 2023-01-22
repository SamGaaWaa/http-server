#include "http/room.hpp"
#include "http/stream.hpp"

#include <algorithm>

namespace http {
    room::room_set_t room::room_set{};
    std::shared_mutex room::room_set_mutex{};

    room::room(std::string name):_name{std::move(name)} {}

    room::~room(){
        std::unique_lock lock{room_set_mutex};
        room_set.erase(room_set.iterator_to(*this));
    }

    void room::join(ws_stream *stream) {
        std::unique_lock lock{_m};
        _streams.insert(stream);
    }

    void room::leave(ws_stream *stream) noexcept {
        std::unique_lock lock{_m};
        _streams.erase(stream);
    }

    void room::publish(const std::shared_ptr<std::string> &msg) {
        std::shared_lock lock{_m};
        for (auto &stream: _streams) {
            stream->send(msg);
        }
    }

    bool operator<(const room &x, const room &y) noexcept {
        return x._name < y._name;
    }

    std::shared_ptr<room> room::new_room(const std::string& name) {
        auto ptr = std::make_shared<room>(name);
        std::unique_lock lock{room_set_mutex};
        auto [it, b] = room_set.insert(*ptr);
        if(b)
            return ptr;
        return it->shared_from_this();
    }

    std::shared_ptr<room> room::find_room(const std::string &name) {
        std::shared_lock lock{room_set_mutex};
        auto it = room_set.find(name, room_compare{});
        if(it == room_set.end())
            return {};
        return it->shared_from_this();
    }

    void room::send_to(const std::shared_ptr<std::string> &msg, const boost::uuids::uuid &id) {
        std::shared_lock lock{_m};
        auto it = std::lower_bound(_streams.begin(), _streams.end(), id, room::stream_compare{});
        if(it != _streams.end() && (*it)->id() == id){
            (*it)->send(msg);
        }
    }

    bool room::stream_compare::operator()(ws_stream *x, ws_stream *y) const noexcept {
        return x->id() < y->id();
    }

    bool room::stream_compare::operator()(ws_stream *x, const boost::uuids::uuid& y) const noexcept {
        return x->id() < y;
    }

    bool room::stream_compare::operator()(const boost::uuids::uuid &x, ws_stream *y) const noexcept {
        return x < y->id();
    }

}