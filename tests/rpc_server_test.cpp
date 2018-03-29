#include "rpc/socket/socket_utilties.h"
void OnConnection(ConnectionPtr con) {
    InetAddr addr = con->GetRemoteSide();
    int fd = con->Getfd();
    char buf[1024];
    fprintf(stderr,"User layer: new connection IPV4 ip = %s, port = %d, socket = %d\n",  
            inet_ntop(AF_INET, &addr.ip_addr.addr4.sin_addr, buf, sizeof(buf)), // IPv6  
            ntohs(addr.ip_addr.addr4.sin_port), fd);
}

void OnMessage(ConnectionPtr con,char *buff,int len) {
    buff[len+1] = '\0';
    InetAddr addr = con->GetRemoteSide();
    int fd = con->Getfd();
    char buf[1024];
    fprintf(stderr,"User layer: on message IPV4 ip = %s, port = %d, socket = %d\n",  
            inet_ntop(AF_INET, &addr.ip_addr.addr4.sin_addr, buf, sizeof(buf)), // IPv6  
            ntohs(addr.ip_addr.addr4.sin_port), fd);
    ::fprintf(stderr,"User layer: recv = [%s]\n",buff);
    (void)con;
    //con->Send(buf,len);
}

void HandleClose(ConnectionPtr con) {
    InetAddr addr = con->GetRemoteSide();
    int fd = con->Getfd();
    char buf[1024];
    fprintf(stderr,"User layer: on close IPV4 ip = %s, port = %d, socket = %d\n",  
            inet_ntop(AF_INET, &addr.ip_addr.addr4.sin_addr, buf, sizeof(buf)), // IPv6  
            ntohs(addr.ip_addr.addr4.sin_port), fd);

}

int main(int argc,char *argv[])
{
    yamq::initLogging(argv[0]);
    InetAddr addr;
    addr.ip_type = inet_ipv4;
    addr.ip_addr.addr4.sin_family = AF_INET;
    addr.ip_addr.addr4.sin_port = htons(8081);
    addr.ip_addr.addr4.sin_addr.s_addr = INADDR_ANY;

    EventDispatcher evd;

    Server server(&evd,addr);
    server.SetConnectCb(OnConnection);
    server.SetMessageCb(OnMessage);
    server.SetCloseCb(HandleClose);
    server.Start();

    yamq::shutdownLogging();
    return 0;
}
