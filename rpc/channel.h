#ifndef _EVENT_HANDLER_H
#define _EVENT_HANDLER_H

class EventDispatcher;
class Socket;
class Channel;

const int EV_NONE = 0x00,EV_READ = 0x01,EV_WRITE = 0x02,EV_UNKNOWN = 0x04,EV_ERROR = 0x08, EV_CLOSE = 0x10;
/* IO事件 */
struct Event {
    int revents;
    int fd;

    bool Readable() {
        return revents & EV_READ;
    }
};

/* 负责分发IO事件 */
class Channel {
    public:
        Channel(EventDispatcher *evd,int fd) : _fd(fd),_events(EV_NONE),_evd(evd),_attached(false){}
        Channel() {}
        virtual void HandleEvent(Event &);
        
        virtual void HandleRead() {}
        virtual void HandleWrite() {}
        virtual void HandleClose() {}
        virtual void HandleError() {}

        void Update();
        void Remove();
        int Getfd() {return _fd;}

        /* 暂时这么简陋吧 */
        void SetEvents(int events) { _events = events;Update();}
        int GetEvents() {return _events;}

        bool Attached() {return _attached;}

        /* event dispatcher expose, beause maybe use AddTimer or etc. */
        EventDispatcher * GetEvd() {return _evd;}
    protected:
        int _fd;
        int _events;
        EventDispatcher *_evd;
        bool _attached;
};

#endif
