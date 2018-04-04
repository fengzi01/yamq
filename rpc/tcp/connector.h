#pragma once
#include "rpc/tcp/socket_utilties.h"
#include "rpc/channel.h"
#include <functional>

class Connector : public Channel {
    public:
        using ConnectCallback = std::function<void (int fd)>;
        Connector();
        ~Connector();

        void Connect();
        virtual void HandleWrite() override;
        virtual void HandleRead() override;
    private:
        const InetAddr _remote_addr; 
        ConnectCallback _conn_cb;
};
