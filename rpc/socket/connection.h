#pragma once
#include "rpc/channel.h"
#include "rpc/socket/socket_utilties.h"
#include <functional>
#include "rpc/io_buffer.h"
#include <memory>

class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;
using ConnectCallback = std::function<void (const ConnectionPtr &)>;
//using MessageCallback = std::function<void (Connection *,IoBuffer *)>;
using MessageCallback = std::function<void (const ConnectionPtr &,char *buf,int len)>;
using CloseCallback = std::function<void (const ConnectionPtr &)>;

class Connection : public Channel, public std::enable_shared_from_this<Connection> {
    public:
        Connection(int64_t id,EventDispatcher *evd,int fd,const InetAddr &local_side,const InetAddr &remote_side);
        void SetMessageCb(MessageCallback cb) {_message_cb = cb;}
        //void SetConnectCb(ConnectCallback cb) {_connect_cb = cb;}
        void SetCloseCb(CloseCallback cb) {_close_cb = cb;}

        virtual void OnRead() override;
        virtual void OnWrite() override;
        
        void Send(const char *data,size_t len);

        int64_t GetId() { return _id;}
        const InetAddr & GetRemoteSide() const { return _remote_side; }
        const InetAddr & GetLocalSide() const { return _local_side; }
        
    private:
        int64_t _id;

        InetAddr _local_side;
        InetAddr _remote_side;

        //ConnectCallback _connect_cb;
        MessageCallback _message_cb;
        CloseCallback _close_cb;

        std::unique_ptr<IoBuffer> _input_buf;
        std::unique_ptr<IoBuffer> _output_buf;
};
