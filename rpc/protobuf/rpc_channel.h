#pragma once
#include "rpc/tcp/connection.h"

/**
 * rpc_channel 是对tcp_connection的封装
 */

namespace rpc {
namespace protbuf {
class RpcChannel {
    private:
        ConnectionPtr _tcp_conn;
};
} // protobuf
} // rpc
