#pragma once
#include "rpc/socket/acceptor.h"
#include "rpc/socket/connection.h"
#include <unordered_map>
#include "rpc/socket/connector.h"

class Server {
    public:
        Server(const InetAddr &addr);
        ~Server();

        void SetConnectCb(ConnectCallback cb) {_connect_cb = cb;}
        void SetMessageCb(MessageCallback cb) {_message_cb = cb;}
        void SetCloseCb(CloseCallback cb) {_close_cb = cb;}

        void Start();
        const ConnectionPtr& GetConnection(int64_t id) {return _ref[id];}
    private:
        void createConnection(int sockfd,const InetAddr &peeraddr);
        void closeConnection(const ConnectionPtr &);
        void removeConnection(const ConnectionPtr &);

        int nextId();
        int _current_id = 0;

        std::unordered_map<int64_t,ConnectionPtr> _ref; // hash reference to id 

        ConnectCallback _connect_cb;
        MessageCallback _message_cb;
        CloseCallback _close_cb;

        std::unique_ptr<EventDispatcher> _evd;
        std::unique_ptr<Acceptor> _acceptor;
};
