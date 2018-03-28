#pragma once
#include "rpc/socket/acceptor.h"
#include "rpc/socket/connection.h"
#include <unordered_map>

class Server {
    public:
        using MessageCallback = std::function<void (const ConnectionPtr &con,Any &message)>;
        Server(const InetAddr &addr);
        ~Server();

        void SetConnectCb(ConnectCallback cb) {_connect_cb = cb;}
        void SetMessageCb(MessageCallback cb) {_message_cb = cb;}
        void SetCloseCb(CloseCallback cb) {_close_cb = cb;}

        // 添加编解码器
        CodecChain &GetCodecChain();

        void Start();
        const ConnectionPtr& GetConnection(int64_t id) {return _ref[id];}
    private:
        void newConnect(int sockfd,const InetAddr &peeraddr);
        void closeConnect(const ConnectionPtr &);
        void removeConnect(const ConnectionPtr &);

        int64_t next_id();
        int64_t _next_id = 0;

        std::unordered_map<int64_t,ConnectionPtr> _connections; // hash 

        ConnectCallback _connect_cb;
        MessageCallback _message_cb;
        CloseCallback _close_cb;

        EventDispatcher *_evd;
        std::unique_ptr<Acceptor> _acceptor;
};
