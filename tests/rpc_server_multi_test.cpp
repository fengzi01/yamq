#include <arpa/inet.h>
#include "rpc/tcp/socket_utilties.h"
#include "rpc/tcp/server_multi.h"
#include "log/logging.h"
#include "rpc/event_dispatcher.h"

void OnConnection(ConnectionPtr con) {
    InetAddr addr = con->GetRemoteSide();
    int fd = con->Getfd();
    char buf[1024];
    fprintf(stderr,"User layer: new connection IPV4 ip = %s, port = %d, socket = %d\n",  
            inet_ntop(AF_INET, &addr.ip_addr.addr4.sin_addr, buf, sizeof(buf)), // IPv6  
            ntohs(addr.ip_addr.addr4.sin_port), fd);
}

void OnMessage(ConnectionPtr con,IoBuffer *buff) {
    InetAddr addr = con->GetRemoteSide();
    int fd = con->Getfd();
    char buf[1024];
    fprintf(stderr,"User layer[%d]: on message IPV4 ip = %s, port = %d, socket = %d\n",  
            std2::this_thread::GetTid(),
            inet_ntop(AF_INET, &addr.ip_addr.addr4.sin_addr, buf, sizeof(buf)), // IPv6  
            ntohs(addr.ip_addr.addr4.sin_port), fd);
    std::string str(buff->GetAllAsString());
    ::fprintf(stderr,"User layer[%d]: recv = [%s]\n",std2::this_thread::GetTid(),str.c_str());
    con->Send(str.c_str(),str.length());
}

void HandleClose(ConnectionPtr con) {
    InetAddr addr = con->GetRemoteSide();
    int fd = con->Getfd();
    char buf[1024];
    fprintf(stderr,"User layer[%d]: on close IPV4 ip = %s, port = %d, socket = %d\n",  
            std2::this_thread::GetTid(),
            inet_ntop(AF_INET, &addr.ip_addr.addr4.sin_addr, buf, sizeof(buf)), // IPv6  
            ntohs(addr.ip_addr.addr4.sin_port), fd);

}

int main(int argc,char *argv[])
{
//    yamq::initLogging(argv[0]);
    InetAddr addr;
    addr.ip_type = inet_ipv4;
    addr.ip_addr.addr4.sin_family = AF_INET;
    addr.ip_addr.addr4.sin_port = htons(8081);
    addr.ip_addr.addr4.sin_addr.s_addr = INADDR_ANY;

    EventDispatcher evd;

    multi::Server server(&evd,addr,2);
    server.SetConnectCb(OnConnection);
    server.SetMessageCb(OnMessage);
    server.SetCloseCb(HandleClose);
    server.Start();

    yamq::shutdownLogging();
    return 0;
}
