#pragma once
#include "rpc/channel.h"
#include "rpc/socket/socket_utilties.h"
#include <functional>
#include "rpc/io_buffer.h"

class Connection : public Channel {
    public:
        using ConnectCallback = std::function<void (Connection *)>;
        using MessageCallback = std::function<void (Connection *,IoBuffer *)>;
        void SetMessageCb();
        void SetConnectCb();
        void SetCloseCb();
        
        void Send(const char *data,size_t len);
        
    private:
        InetAddr _local_side;
        InetAddr _remote_side;

};
