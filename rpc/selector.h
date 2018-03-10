#ifndef POLLER_H
#define POLLER_H
#include <vector>
#include "rpc/channel.h"
#include <memory>
#include <map>
#include <poll.h>
using std::unique_ptr;
using std::vector;
using std::map;

/**
 * Event Demutiplexer
 */
class Selector {
    public:
        virtual int Select(int timeout,vector<Event> &)  = 0;
        virtual int Add(int fd,int events) = 0;
        virtual int Update(int, int) = 0;
        virtual int Remove(int) = 0;
    protected:
        EventDispatcher *_dispatcher;
};

class PollSelector : public Selector {
    public:
        virtual int Select(int timeout,vector<Event> &) override;
        virtual int Add(int fd,int events) override;
        virtual int Update(int fd,int events) override;
        virtual int Remove(int fd) override;
    private:
        vector<pollfd> _pollfds;        
        map<int,size_t> _fd_idx_map;
};

unique_ptr<Selector> MakeDefaultSelector(); 
#endif /* _POLLER_H */
