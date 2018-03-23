#include "rpc/timer/timer_queue_rbtree.h"
#include "timerfd.h"
#include "log/logging.h"
#include "rpc/event_dispatcher.h"
#include <functional>

TimerQueueRbtree::TimerQueueRbtree(EventDispatcher *evd):_own_epoch(Clock::GetNowTicks()) {
    _tree = ::rbtree_create(NULL);
    _fd = ::createTimerfd();
    _evd = evd;
}

TimerQueueRbtree::~TimerQueueRbtree() {
    ::rbtree_destroy(_tree);
    ::closeTimerfd(_fd);
}

int TimerQueueRbtree::AddTimer(uint64_t time,uint64_t interval,TimerCallback cb) {
    Timer *timer = new Timer();
    
    //timer->id = -1;
    timer->cb = cb;
    timer->index = -1;
    timer->elapse = interval;
    timer->interval = interval;
    timer->time = time;
    timer->attach = nullptr;

    timer->id = nextId();
    _ref[timer->id] = timer;

    // 保证在Loop线程中更新
    _evd->RunInLoop(std::bind(&TimerQueueRbtree::addTimer,this,timer));
    return timer->id;
}

void TimerQueueRbtree::addTimer(Timer *timer) {
    uint64_t now = Clock::GetNowTicks() - _own_epoch;
    timer->expire = now + timer->time;
    auto n = ::rbtree_insert(_tree,timer->expire,timer);
    timer->attach = n;
    if (::rbtree_min(_tree)->key >= timer->expire) {
        ::resetTimerfd(_fd,_own_epoch + timer->expire);
    } else {
        // FIXME
    }
}

void TimerQueueRbtree::CancelTimer(int timer_id) {
    // 保证在Loop线程中更新
    _evd->RunInLoop(std::bind(&TimerQueueRbtree::cancelTimer,this,timer_id));  
}

bool TimerQueueRbtree::cancelTimer(int id) {
    auto timer = static_cast<Timer *>(_ref[id]);
    if (nullptr == timer) {
        return false;
    }
    auto n = static_cast<rbtree_node_t *>(timer->attach);
    if (n != nullptr) {
        ::rbtree_delete(_tree,n);
    }
    delete timer;
    return true;
}

void TimerQueueRbtree::PerTick() {
    uint64_t now = Clock::GetNowTicks() - _own_epoch;
    std::vector<Timer *> expired_timers;
    rbtree_node_t *n = nullptr;
    Timer *timer = nullptr;
    for(;;) {
        n = ::rbtree_min(_tree);
        if (n == _tree->nil) break;
        timer = static_cast<Timer *>(n->val);
        if (timer->expire < now) {
            expired_timers.push_back(timer);
            ::rbtree_delete(_tree,n);
            timer->attach = nullptr;
        } else {
            break;
        }
    }
    for ( Timer *node : expired_timers) {
        if (node->elapse > 0) {
            --(node->elapse);
            node->cb();
            if (node->elapse > 0) {
                addTimer(node);
            } else {
                delete node;
            }
        } else {
            delete node;
        }
    }
}

int64_t TimerQueueRbtree::WaitTimeUsec() const {
    rbtree_node_t *n = nullptr;
    Timer *timer = nullptr;
    n = ::rbtree_min(_tree);
    if (n == _tree->nil) return -1;
    timer = static_cast<Timer *>(n->val);
    return timer->expire;
}

void TimerQueueRbtree::HandleRead() {
    uint64_t  now = Clock::GetNowTicks();
    ::readTimerfd(_fd,now); 
    PerTick();
    int64_t expire = WaitTimeUsec();
    if (expire > 0) {
        ::resetTimerfd(_fd,_own_epoch + expire);
    } else {
        // FIXME
        LOG(TRACE) << "NO TIMER!";
    }
}

