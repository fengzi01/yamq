#pragma once
#include "rpc/socket/acceptor.h"
#include "rpc/socket/connection.h"

class Server {
    public:

    private:
        void newConnection(int sockfd);
        // 依赖翻转，怎么实现？？？
        Acceptor _acceptor;
};
