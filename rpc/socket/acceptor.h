#pragma once
#include "rpc/channel.h"
class Acceptor : public Channel {
    // 用别的办法可以做到吗？
    // 用胶水functor来做吧！
    public:
        void listen();
};
