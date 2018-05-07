#include <arpa/inet.h>
#include "rpc/tcp/socket_utilties.h"
#include "rpc/tcp/server_multi.h"
#include "log/logging.h"
#include "rpc/event_dispatcher.h"
#include "rpc/http/http_server.h"
#include "rpc/http/http_request.h"
#include "rpc/http/http_response.h"

void OnRequest(const rpc::http::HttpRequest &req,rpc::http::HttpResponse *resp) {
    resp->setStatusCode(rpc::http::HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("text/html");
    resp->addHeader("Server", "Muduo");
    resp->setBody("<html><head><title>This is title</title></head>"
            "<body><h1>Hello</h1>"
            "</body></html>");
}

int main(int argc,char *argv[])
{
    //yamq::initLogging(argv[0]);
    InetAddr addr;
    addr.ip_type = inet_ipv4;
    addr.ip_addr.addr4.sin_family = AF_INET;
    addr.ip_addr.addr4.sin_port = htons(8081);
    addr.ip_addr.addr4.sin_addr.s_addr = INADDR_ANY;
    EventDispatcher evd;
    rpc::http::HttpServer server(&evd,addr);
    server.SetHttpCallback(OnRequest);
    server.Start();
    yamq::shutdownLogging();
    return 0;
}
