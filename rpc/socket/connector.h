#pragma once
#include "rpc/socket/socket_utilties.h"
#include "rpc/channel.h"
#include <functional>

class Connector : public Channel {
    public:
        Connector();
        ~Connector();

        void Connect();
        virtual void OnWrite() override;
        virtual void OnRead() override;
    private:
        const InetAddr _remote_addr; 
        ConnectionCallback _conn_cb;
};
