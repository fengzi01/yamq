#include "rpc/socket/connection.h"
#include "log/logging.h"
#include "unistd.h"

Connection::Connection(int64_t id,EventDispatcher *evd,int fd,const InetAddr &local_side,const InetAddr &remote_side):_id(id) {
    _local_side = local_side;
    _remote_side = remote_side;
    _fd = fd;
    _evd = evd;
}

void Connection::OnRead() {
    char buf[1000];
    int n = ::read(_fd, buf, 1000);
    if ( 0 == n ) {
        if (_close_cb) {
            _close_cb(shared_from_this());
        }
        return;
    }
    if (_message_cb) {
        _message_cb(shared_from_this(),buf,n);
    }
}

void Connection::OnWrite() {
    LOG(TRACE) << "Connection: fd = " << _fd << " is writeable";
}
