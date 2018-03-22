#include "rpc/timer/timer_queue_rbtree.h"
#include "timerfd.h"
#include "log/logging.h"

TimerQueueRbtree::TimerQueueRbtree(EventDispatcher *evd):_own_epoch(Clock::GetNowTicks()) {
    _tree = ::rbtree_create(NULL);
    _fd = createTimerfd();
    _evd = evd;
}

TimerQueueRbtree::~TimerQueueRbtree() {
    rbtree_destroy(_tree);
    closeTimerfd(_fd);
}

int TimerQueueRbtree::AddTimer(uint64_t time,uint64_t interval,TimerCallback cb) {
    uint64_t now = Clock::GetNowTicks();
    uint64_t expire = now - _own_epoch + time;
    Timer *timer = new Timer();
    
    int id = nextId();
    timer->id = id;
    timer->expire = expire;
    timer->cb = cb;
    timer->index = -1;
    timer->elapse = interval;
    timer->interval = interval;
    timer->time = time;

    auto n = ::rbtree_insert_node(_tree,timer->expire,timer);
    _ref[id] = n;
    LOG(TRACE) << "INSERT n.key = " << n->key;
    LOG(TRACE) << "ADD TIMER " << timer->id << " ela = " << timer->elapse;
    timer = static_cast<Timer *>(::rbtree_min_node(_tree)->val);
    resetTimerfd(_fd,_own_epoch + timer->expire);
    return id;
}

bool TimerQueueRbtree::CancelTimer(int id) {
    rbtree_node_t *n = static_cast<rbtree_node_t *>(_ref[id]);
    if (nullptr == n) {
        return false;
    }
    ::rbtree_del_node(_tree,n);
    Timer *timer = static_cast<Timer *>(n->val);
    return true;
}

void TimerQueueRbtree::PerTick() {
    uint64_t now = Clock::GetNowTicks() - _own_epoch;
    std::vector<Timer *> expired;
    rbtree_node_t *n = nullptr;
    Timer *timer = nullptr;
    for(;;) {
        n = ::rbtree_min_node(_tree);
        LOG(TRACE) << "UPDATE n.key = " << n->key;
        if (n == _tree->nil) break;
        timer = static_cast<Timer *>(n->val);
        LOG(TRACE) << "timer->expire = " << timer->expire << " now = " << now;
        if (timer->expire < now) {
            expired.push_back(timer);
            ::rbtree_del_node(_tree,n);
        } else {
            break;
        }
    }
    LOG(TRACE) << "expired.size = " << expired.size();
    for ( auto node : expired) {
        if (node->elapse > 0) {
            --(node->elapse);
            node->cb();
        }
        if (node->elapse > 0) {
            // FIXME
            LOG(TRACE) << "ADD TIMER " << node->id << " ela = " << node->elapse;
            //AddTimer(node->time,node->elapse,node->cb);
        }
    }
}

int64_t TimerQueueRbtree::WaitTimeUsec() {
    rbtree_node_t *n = nullptr;
    Timer *timer = nullptr;
    n = ::rbtree_min_node(_tree);
    if (n == _tree->nil) return -1;
    timer = static_cast<Timer *>(n->val);
    return timer->expire;
}

void TimerQueueRbtree::HandleRead() {
    uint64_t  now = Clock::GetNowTicks();
    readTimerfd(_fd,now); 
    PerTick();
    int64_t expire = WaitTimeUsec();
    if (expire > 0) {
        resetTimerfd(_fd,_own_epoch + expire);
    } else {
        // FIXME
        LOG(TRACE) << "NO TIMER!";
    }
}

void TimerQueueRbtree::Start() {
    SetEvents(EV_READ);
}

