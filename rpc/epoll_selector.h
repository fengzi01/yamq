#pragma once

#include <vector>
#include "rpc/channel.h"
#include "rpc/selector.h"
#include <memory>
#include <map>

class EpollSelector : public Selector {
    public:
        EpollSelector(EventDispatcher *evd);
        virtual int Select(int timeout,vector<Event> &) override;
        virtual int Add(int fd,int events) override;
        virtual int Update(int fd,int events) override;
        virtual int Remove(int fd) override;
    private:
        int _epfd;
        vector<struct epoll_event> _events;
};
