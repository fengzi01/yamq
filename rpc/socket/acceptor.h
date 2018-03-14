#pragma once
#include "rpc/channel.h"
#include <functional>
#include "rpc/socket/socket_utilties.h"

class EventDispatcher;
class Connection;
class Acceptor : public Channel {
    // 用胶水functor来做吧！
    using ConnectionCallback = std::function<void(int fd,InetAddr *)>;
    //using ConnectionCallback = std::function<void()>;

    public:
        Acceptor(EventDispatcher *evd,const InetAddr *addr);
        ~Acceptor();

        virtual void OnRead() override;

        void Listen();
        void SetConnectionCb(ConnectionCallback cb) { _conn_cb = cb;}
    private:
        bool _listening;
        ConnectionCallback _conn_cb;
        const InetAddr _inet_addr;
};
