#pragma once
#include "rpc/tcp/server_multi.h"

class Service;

namespace rpc {
namespace protobuf {
class RpcServer {
    public:
        RpcServer(EventDispatcher *evd,InetAddr *addr);
        ~RpcServer();

        void AddService(Service *service);
        void Start();
    private:
        EventDispatcher *_evd;
        multi::Server _tcp_server;
        
};
} // protobuf
} // rpc
