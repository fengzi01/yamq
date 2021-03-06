#pragma once
#include "rpc/channel.h"
#include "rpc/tcp/socket_utilties.h"

class EventDispatcher;
class Acceptor : public Channel {
    // 用胶水functor来做吧！
    public:
        using NewConnectCallback = std::function<void (int fd,const InetAddr &peer_addr)>;
        Acceptor(EventDispatcher *evd,const InetAddr &addr);
        ~Acceptor();

        virtual void HandleRead() override;

        void Listen();
        void SetNewConnectCb(const NewConnectCallback &cb) { _new_conn_cb = cb;}
        const InetAddr & GetInetAddr() const {return _inet_addr;} 
    private:
        bool _listening;
        NewConnectCallback _new_conn_cb;
        InetAddr _inet_addr;
};
