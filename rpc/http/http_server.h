#pragma once

#include "rpc/tcp/server_multi.h"
#include "rpc/event_dispatcher.h"

namespace rpc {
namespace http {
class HttpRequest;
class HttpResponse;

class HttpServer {
    public:
        HttpServer(EventDispatcher *evd,const InetAddr &addr);
        typedef std::function<void (const HttpRequest &,HttpResponse *)> HttpCallback;
        void SetHttpCallback(const HttpCallback &cb) { _http_callback = cb; }
        void Start() {_tcp_server.Start();}
    private:
        void onConnection(const ConnectionPtr &conn);
        void onMessage(const ConnectionPtr &conn,IoBuffer *buf); 
        void onRequest(const ConnectionPtr &conn, const HttpRequest &);

        multi::Server _tcp_server;        
        // http请求callback
        HttpCallback _http_callback;
};

} // http
} // rpc
