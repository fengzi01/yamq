#include "rpc/tcp/server_multi.h"
#include "rpc/event_dispatcher.h"
#include <assert.h>
#include "log/logging.h"

namespace multi {
IoWorker::IoWorker():_running(false),
    _active(new std2::Thread(std::bind(&IoWorker::threadFunc,this))) {
}

IoWorker::~IoWorker() {
    assert(_running == true);
    _evd->Stop();
    _active->Join();
}

void IoWorker::threadFunc() {
    _evd = new EventDispatcher();
    _running = true;
    // loop
    _evd->Start();
    delete _evd;
    _evd = NULL;
}

EventDispatcher *IoWorker::GetEvd() {
    while (!_running) { }
    return _evd;
}

Server::Server(EventDispatcher *evd,const InetAddr &addr,size_t worker_num):
    _next_id(0),
    _evd(evd),
    _acceptor(new Acceptor(evd,addr)),
    _next_evd_id(0),
    _running(false),
    _worker_num(worker_num)
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

EventDispatcher *Server::nextEvd() {
    assert(_running);
    EventDispatcher *evd = _evd;
    if (!_evds.empty()) {
        evd = _evds[_next_evd_id];
        ++_next_evd_id;
        if (_next_evd_id >= _evds.size()) {
            _next_evd_id = 0;
        }
    }
    return evd;
}

void Server::newConnect(int sockfd,const InetAddr &peeraddr) {
    int64_t id = nextId();
    EventDispatcher *evd = nextEvd();
    ConnectionPtr con = std::make_shared<Connection>(id,evd,sockfd,_acceptor->GetInetAddr(),peeraddr);
    con->SetMessageCb(_message_cb);
    con->SetCloseCb(std::bind(&Server::closeConnect,this,std::placeholders::_1));

    evd->RunInEvd(std::bind(&Connection::establish,con));

    _connections[id] = con;

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
    LOG(TRACE) << "starting";
    assert(!_running);
    for (size_t i = 0; i < _worker_num; ++i) {
        LOG(TRACE) << "new worker";
        auto worker = std::unique_ptr<IoWorker>(new IoWorker());
        _evds.push_back(worker->GetEvd());
        _workers.push_back(std::move(worker));
    }
    _acceptor->Listen();
    _evd->Start();
    _running = true;
    LOG(TRACE) << "started";
}

} // multi
