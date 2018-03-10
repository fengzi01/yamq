#ifndef _EVENT_HANDLER_H
#define _EVENT_HANDLER_H

class EventDispatcher;
class Socket;
class Channel;

/* IO事件 */
struct Event {
    const static int EV_READ = 1,EV_WRITE = 2,EV_UNKNOWN = 4;
    int revents;
    int fd;
    Channel *channel;
};

/* 负责分发IO事件 */
class Channel {
    public:
        virtual void HandleEvent(Event &);
        virtual void OnReadable();
        virtual void OnWriteable();
        virtual void OnClose();
        virtual void OnError();

        void Update();
        int Getfd() {return _fd;}

        /* 暂时这么简陋吧 */
        void SetRevents(int revents) { _revents = revents;}
        int GetRevents() {return _revents;}
    private:
        int _fd;
        int _revents;
        EventDispatcher *_evd;
};

class ConnectionChannel : public Channel {
};

class AcceptorChannel : public Channel {
};

class ConnectorChannel : public Channel {
};
#endif
