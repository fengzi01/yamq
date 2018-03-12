#ifndef _EVENT_HANDLER_H
#define _EVENT_HANDLER_H

class EventDispatcher;
class Socket;
class Channel;

const int EV_NONE = 0,EV_READ = 1,EV_WRITE = 2,EV_UNKNOWN = 4;
/* IO事件 */
struct Event {
    int revents;
    int fd;
};

/* 负责分发IO事件 */
class Channel {
    public:
        Channel(EventDispatcher *evd,int fd) : _fd(fd),_events(EV_NONE),_evd(evd){}
        virtual void HandleEvent(Event &) {}
/*
        virtual void OnReadable();
        virtual void OnWriteable();
        virtual void OnClose();
        virtual void OnError();
*/

        void Update();
        int Getfd() {return _fd;}

        /* 暂时这么简陋吧 */
        void SetEvents(int events) { _events = events;}
        int GetEvents() {return _events;}
    private:
        int _fd;
        int _events;
    protected:
        EventDispatcher *_evd;
};

class ConnectionChannel : public Channel {
};

class AcceptorChannel : public Channel {
};

class ConnectorChannel : public Channel {
};
#endif
