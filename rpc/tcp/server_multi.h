#pragma once
#include "rpc/tcp/acceptor.h"
#include "rpc/tcp/connection.h"
#include "base/std/Thread.h"
#include <unordered_map>
#include <atomic>
#include <assert.h>

// 主reactor + 多个子reactor范例
// 主reactor监听并选出一个reactor作为通信reactor
namespace multi {
// 处理IO操作的线程
class IoWorker {
    public:
        IoWorker();
        ~IoWorker();
        void Start();
        EventDispatcher *GetEvd();
    private:
        void threadFunc();

        EventDispatcher *_evd;
        std::atomic<bool> _running;
        std::unique_ptr<std2::Thread> _active;
};

class Server {
    public:
        using MessageCallback = Connection::MessageCallback;
        using ConnectCallback = Connection::ConnectCallback;
        using CloseCallback = Connection::CloseCallback;

        Server(EventDispatcher *evd,const InetAddr &addr,size_t worker_num = 0);
        ~Server();

        void SetConnectCb(ConnectCallback cb) {_connect_cb = cb;}
        void SetMessageCb(MessageCallback cb) {_message_cb = cb;}
        void SetCloseCb(CloseCallback cb) {_close_cb = cb;}

        void Start();
        const ConnectionPtr& GetConnection(int64_t id) {return _connections[id];}

        void SetWorkerNum(size_t num) { 
            assert(!_running);
            _worker_num = num;
        }
    private:
        void newConnect(int sockfd,const InetAddr &peeraddr);
        void closeConnect(const ConnectionPtr &);
        void removeConnect(const ConnectionPtr &);

        EventDispatcher * nextEvd();

        int64_t nextId();
        int64_t _next_id = 0;

        std::unordered_map<int64_t,ConnectionPtr> _connections; // hash 

        ConnectCallback _connect_cb;
        MessageCallback _message_cb;
        CloseCallback _close_cb;

        EventDispatcher *_evd;
        std::unique_ptr<Acceptor> _acceptor;

        std::vector<std::unique_ptr<IoWorker>> _workers;
        std::vector<EventDispatcher *> _evds;
        size_t _next_evd_id;

        std::atomic<bool> _running;
        size_t _worker_num;
};
} // multi
