#include "rpc/event_dispatcher.h"
#include "rpc/channel.h"
#include "rpc/selector.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <poll.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h> // inet_addr
#include <stdlib.h>

const char * message = "hello world!";

class TestChannel : public Channel {
    public:
        TestChannel(EventDispatcher *evd,int fd) : Channel(evd,fd){}
    public:
        virtual void HandleEvent(Event &ev) {
            char buf[1024];
            size_t n = 0;
            printf("event type : 0x%02x\n",ev.revents);
            if (ev.revents &EV_READ) {
                // 可读事件
                n = ::read(Getfd(),buf,1024);
                buf[n+1] = '\0';
                printf("fd: %d,recv: %s\n",ev.fd,buf);
                //::close(Getfd()); 
                if ( 0 == n) {
                    printf("close request from server\n");
                    Remove();
                }
            }
            if (ev.revents & EV_WRITE) {
                write(Getfd(),message,strlen(message));
                printf("fd: %d,write: %s\n",ev.fd,message);
                SetEvents(EV_READ);
            } 
        }
};

#define BUFSIZE (4096)
#define REMOTE_IP "127.0.0.1"
#define REMOTE_PORT 8080

static int CreateSocketfd() {
    int client_fd;

      struct sockaddr_in remote_addr;

      memset(&remote_addr,0,sizeof(remote_addr));
      remote_addr.sin_family = AF_INET;
      remote_addr.sin_addr.s_addr = inet_addr(REMOTE_IP);
      remote_addr.sin_port = htons(REMOTE_PORT);

      // socket
      if ((client_fd = ::socket(AF_INET,SOCK_STREAM,0)) < 0) {
          perror("Socket Error");
          return -1;
      }

      // connect()时由系统随机生成一个IP和port
      if (::connect(client_fd,(struct sockaddr *)&remote_addr,sizeof(remote_addr)) < 0) {
          perror("Connect Error:");
          return -2;
      }

      return client_fd;
}

int main(int argc, char *argv[]) {
    EventDispatcher evd;
    int fd = CreateSocketfd();
    int fd2 = CreateSocketfd();
    TestChannel channel(&evd,fd2);
    unique_ptr<Channel> pChannel(new TestChannel(&evd,fd));
    channel.SetEvents(EV_WRITE);
    pChannel->SetEvents(EV_WRITE);

    evd.Start();
}
