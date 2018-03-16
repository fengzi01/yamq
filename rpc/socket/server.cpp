#include "rpc/socket/server.h"
#include "rpc/event_dispatcher.h"
#include "rpc/socket/acceptor.h"
#include "rpc/selector.h"
#include <arpa/inet.h>  // inet_ntop

Server::Server(const InetAddr &addr):_current_id(0),_evd(new EventDispatcher()),_acceptor(new Acceptor(_evd.get(),addr)) {
    // set acceptor ConnectionCb to Server createConnection
    _acceptor->SetConnectionCb(
            std::bind(&Server::createConnection,
                this,
                std::placeholders::_1,
                std::placeholders::_2));
}

Server::~Server() {
}

int Server::nextId() {
    int id = _current_id;
    for (;;) {
        if (_ref.count(id) > 0) {
            ++id;
        } else {
            break;
        }
    }
    _current_id = id;
    return id;
}

void Server::createConnection(int sockfd,const InetAddr &peeraddr) {
    int64_t id = nextId();
    //char buf[1024];
    //fprintf(stderr,"CreateConnection IPV4 ip = %s, port = %d, socket = %d\n",  
    //        inet_ntop(AF_INET, &peeraddr.ip_addr.addr4.sin_addr, buf, sizeof(buf)), // IPv6  
    //            ntohs(peeraddr.ip_addr.addr6.sin6_port), sockfd);
    ConnectionPtr con = std::make_shared<Connection>(id,_evd.get(),sockfd,_acceptor->GetInetAddr(),peeraddr);
    _ref.insert(std::make_pair(id, con)); 

    con->SetMessageCb(_message_cb);
    con->SetCloseCb(std::bind(&Server::closeConnection,this,std::placeholders::_1));
    con->SetEvents(EV_READ);

    if (_connect_cb) {
        _connect_cb(con);
    }
}

void Server::closeConnection(const ConnectionPtr &con) {
    con->SetEvents(EV_NONE);
    if (_close_cb) {
        _close_cb(con);
    }
}

void Server::removeConnection(const ConnectionPtr &con) {
    if (con->GetEvents() != EV_NONE) {
        closeConnection(con);
    }

    int64_t id = con->GetId();
    _ref.erase(id);
    con->Remove();
}

void Server::Start() {
    _acceptor->Listen();
    _acceptor->SetEvents(EV_READ);
    _evd->Run();
}


