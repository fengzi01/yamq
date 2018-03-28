#include <iostream>
#include <memory>

#include "rpc/event_dispatcher.h"
#include "rpc/selector.h"
#include "rpc/socket/acceptor.h"
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "log/logging.h"

void onConnection(int fd, const InetAddr &peeraddr) {
    InetAddr addr = peeraddr;
    char buf[1024];
    fprintf(stderr,"New connection IPV4 ip = %s, port = %d, socket = %d\n",  
            inet_ntop(AF_INET, &addr.ip_addr.addr4.sin_addr, buf, sizeof(buf)), // IPv6  
            ntohs(addr.ip_addr.addr4.sin_port), fd);
    close(fd);
}

int main(int argc,char *argv[])
{
    //yamq::initLogging(argv[0]);
    InetAddr addr;
    addr.ip_type = inet_ipv4;
    addr.ip_addr.addr4.sin_family = AF_INET;
    addr.ip_addr.addr4.sin_port = htons(8080);
    addr.ip_addr.addr4.sin_addr.s_addr = INADDR_ANY;
    EventDispatcher evd;

    Acceptor acceptor(&evd,addr);
    LOG(TRACE) << "acceptor";
    acceptor.SetConnectCb(onConnection);
    acceptor.SetEvents(EV_READ);
    acceptor.Listen();
    LOG(TRACE) << "acceptor";

    evd.Start();
    //yamq::shutdownLogging();
    return 0;
}
