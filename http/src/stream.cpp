#include "http/stream.hpp"
#include "http/room.hpp"
#include <iostream>
#include <algorithm>

namespace http{

    boost::uuids::random_generator ws_stream::gen{};

    ws_stream::ws_stream(ws_stream_t stream) noexcept: _stream{std::move(stream)}, _id{gen()}{}

    ws_stream::~ws_stream(){
        if(_room)
            _room->leave(this);
    }

    void ws_stream::send(const std::shared_ptr<std::string> &msg) {
        // asio::post(_stream.get_executor(), [this, msg = std::shared_ptr<std::string>{msg}]()mutable{
            _q.emplace_back(std::move(msg));
            if(_q.size() > 1)
                return;
            asio::co_spawn(_stream.get_executor(), [this]()->task<void>{
                while(!_q.empty()){
                    co_await _stream.async_write(asio::buffer(*_q.front()));
                    _q.pop_front();
                }
            }, asio::detached);
        // });
    }

    void ws_stream::join(http::room &r) {
        r.join(this);
        _room = r.shared_from_this();
    }

    void ws_stream::leave() {
        if(!_room)
            return;
        _room->leave(this);
        _room.reset();
    }

    room &ws_stream::room() noexcept {
        return *_room;
    }

    void ws_stream::join(const std::string &name) {
        auto ptr = room::find_room(name);
        if(!ptr)
            ptr = room::new_room(name);
        join(*ptr);
    }


    const boost::uuids::uuid &ws_stream::id() const noexcept {
        return _id;
    }

}