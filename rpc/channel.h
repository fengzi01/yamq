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
        Channel(EventDispatcher *evd,int fd) : _fd(fd),_events(EV_NONE),_evd(evd),_attached(false){}
        virtual void HandleEvent(Event &) = 0;
/*
        virtual void OnReadable();
        virtual void OnWriteable();
        virtual void OnClose();
        virtual void OnError();
*/

        void Update();
        void Remove();
        int Getfd() {return _fd;}

        /* 暂时这么简陋吧 */
        void SetEvents(int events) { _events = events;Update();}
        int GetEvents() {return _events;}

        bool Attached() {return _attached;}
    private:
        int _fd;
        int _events;
        EventDispatcher *_evd;
        bool _attached;
};

class ConnectionChannel : public Channel {
};

class AcceptorChannel : public Channel {
};

class ConnectorChannel : public Channel {
};
#endif
