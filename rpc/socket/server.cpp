#include "rpc/socket/server.h"
#include "rpc/event_dispatcher.h"
#include "rpc/socket/acceptor.h"
#include "rpc/selector.h"
#include <arpa/inet.h>  // inet_ntop
#include "rpc/socket/connection.h"

Server::Server(EventDispatcher *evd,const InetAddr &addr):
    _next_id(0),
    _evd(evd),
    _acceptor(new Acceptor(evd,addr)) 
{
    // set acceptor ConnectionCb to Server createConnection
    _acceptor->SetNewConnectCb(
            std::bind(&Server::newConnect,
                this,
                std::placeholders::_1,
                std::placeholders::_2));
}

Server::~Server() {
}

int64_t Server::nextId() {
    int64_t id = _next_id;
    for (;;) {
        if (id < 0) {
            id = 0;
        }
        if (_connections.count(id) > 0) {
            ++id;
            continue;
        }
        break;
    }
    _next_id = id;
    return id;
}

void Server::newConnect(int sockfd,const InetAddr &peeraddr) {
    int64_t id = nextId();
    ConnectionPtr con = std::make_shared<Connection>(id,_evd,sockfd,_acceptor->GetInetAddr(),peeraddr);
    con->SetMessageCb(_message_cb);
    con->SetCloseCb(std::bind(&Server::closeConnect,this,std::placeholders::_1));
    con->SetEvents(EV_READ); // start read!

    _connections[id] = con;

    con->establish();

    if (_connect_cb) {
        _connect_cb(con);
    }
}

void Server::closeConnect(const ConnectionPtr &con) {
    // FIXME
    con->SetEvents(EV_NONE);
    if (_close_cb) {
        _close_cb(con);
    }
    removeConnect(con);
}

void Server::removeConnect(const ConnectionPtr &con) {
    // FIXME
    int64_t id = con->GetId();
    con->Remove();
    _connections.erase(id);
}

void Server::Start() {
    _acceptor->Listen();
    _evd->Start();
}


