#include "rpc/socket/acceptor.h"
#include "rpc/event_dispatcher.h"
#include "log/logging.h"
#include <arpa/inet.h>  // inet_ntop
#include <unistd.h>

static int createAcceptSocketOrDie(InetAddr &addr)
{
    /**
     * AF_INET（又称 PF_INET）是 IPv4 网络协议的套接字类型，
	 * AF_INET6 则是 IPv6 的；
     * AF_UNIX 则是 Unix 系统本地通信。 
     * AF_INET 相比 AF_UNIX 更具通用性，因为 Windows 上有 AF_INET 而没有 AF_UNIX。
     */
    int sockfd = 0;
    if (addr.ip_type == ipv4) {
        sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    } else {
        sockfd = ::socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    }
    if (sockfd < 0)
    {
        LOG(FATAL) << "sockets::createNonblockingOrDie";
    }

    LOG(TRACE) << "Create acceptor fd  = " << sockfd;
    // bind
    int ret = 0;
    if (addr.ip_type == ipv4) {
        ret = ::bind(sockfd, (struct sockaddr *) &addr.ip_addr, sizeof(struct sockaddr));
    } else {
        ret = ::bind(sockfd, (struct sockaddr *) &addr.ip_addr, sizeof(struct sockaddr_in6));
    }
    if (ret < 0)
    {
        LOG(FATAL) << "sockets::bindOrDie";
        exit(-1);
    }
    return sockfd;
}

static int listenOrDie(int fd) {
    int ret = 0;
    if ((ret = ::listen(fd,SOMAXCONN)) < 0 ) {
        LOG(FATAL) << "listenOrDie fd = " << fd;
    }
    return ret;
}

static int acceptConnect(int sockfd, InetAddr *addr)
{
    socklen_t addr_len = static_cast<socklen_t>(sizeof addr->ip_addr);
    int connfd = ::accept4(sockfd, (struct sockaddr *)&addr->ip_addr,
            &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0)
    {
        int savedErrno = errno;
        LOG(ERROR) << "Socket::accept";
        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG(FATAL) << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG(FATAL) << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
    char buf[1024];
    if (addr->ip_type == ipv4) {
        fprintf(stderr,"New connection IPV4 ip = %s, port = %d, socket = %d\n",  
                inet_ntop(AF_INET, &addr->ip_addr.addr4.sin_addr, buf, sizeof(buf)), // IPv6  
                ntohs(addr->ip_addr.addr4.sin_port), connfd);
    } else {
        fprintf(stderr,"New connection IPV6 from %s, port %d, socket %d\n",  
                inet_ntop(AF_INET6, &addr->ip_addr.addr6.sin6_addr, buf, sizeof(buf)), // IPv6  
                ntohs(addr->ip_addr.addr6.sin6_port), connfd);
    }
    return connfd;
}

Acceptor::Acceptor(EventDispatcher *evd,const InetAddr &addr):
    _listening(false),
    _inet_addr(addr) {
    _evd = evd;
    _fd = createAcceptSocketOrDie(_inet_addr);
}

Acceptor::~Acceptor() {
    ::close(_fd);
}

void Acceptor::Listen() {
   listenOrDie(_fd); 
   _listening = true;
}

void Acceptor::OnRead() { 
    InetAddr peeraddr = {_inet_addr.ip_type};
    int newfd = acceptConnect(_fd,&peeraddr);
    if (_conn_cb) {
        _conn_cb(newfd,peeraddr);
        //_conn_cb();
    } else {
        ::close(newfd);
    }
}

