#include "rpc/socket/connection.h"
#include "log/logging.h"
#include "unistd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include "rpc/event_dispatcher.h"

Connection::Connection(int64_t id,EventDispatcher *evd,int fd,const InetAddr &local_side,const InetAddr &remote_side):
    _id(id),
    _local_side(local_side),
    _remote_side(remote_side),
    _status(CONNECTING),
    _input_buf(new IoBuffer()),
    _output_buf(new IoBuffer())
{
    _fd = fd;
    _evd = evd;
}

Connection::~Connection() {}

void Connection::HandleRead() {
    size_t n = _input_buf->Retrieve(_fd);
    if ( n > 0 ) {
        if (_message_cb) {
            _message_cb(shared_from_this(),_input_buf.get());
        }
    } else if ( 0 == n ) {
        LOG(TRACE) << "connection is going to close fd = " << _fd;
        _status = CLOSING;
        if (_close_cb) {
            _close_cb(shared_from_this());
        }
        _status = CLOSED;
    }
}

void Connection::HandleWrite() {
    LOG(TRACE) << "Connection: fd = " << _fd << " is writeable";

	ssize_t n = ::send(_fd, _output_buf->Peek(), _output_buf->ReadableBytes(), MSG_NOSIGNAL);
	if (n > 0) {
        _output_buf->Seek(n);

		if (_output_buf->ReadableBytes() == 0) {
            DisableWrite();
		}
	} else {
        int serrno = errno;
        LOG(TRACE) << "connection write error. errno = " << serrno;
        HandleError();
	}
}

void Connection::Send(const char *data,size_t len) {
    _evd->RunInEvd(std::bind(&Connection::send,shared_from_this(),data,len));
}

void Connection::send(const char *data,size_t len) {
	if (_status != CONNECTED) {
		LOG(WARNING) << "disconnected, give up writing";
		return;
	}

	ssize_t nwritten = 0;
	size_t remaining = len;

	// if no data in output queue, writing directly
	if (!IsWritable() && _output_buf->ReadableBytes() == 0) {
		nwritten = ::send(_fd, static_cast<const char*>(data), len, MSG_NOSIGNAL);
		if (nwritten >= 0) {
			remaining = len - nwritten;
			if (remaining == 0) {
                LOG(TRACE) << "write complete";
			}
		} else {
			int serrno = errno;
			nwritten = 0;
            LOG(ERROR) << "SendInLoop write failed errno=" << serrno << " " << strerror(serrno);
            if (serrno == EPIPE || serrno == ECONNRESET) {
                HandleError();
                abort();
            }
		}
	}

	if (remaining > 0) {
		size_t old_len = _output_buf->ReadableBytes();

		//if (old_len + remaining >= high_water_mark_
		//        && old_len < high_water_mark_
		//        && high_water_mark_fn_) {
		//    loop_->QueueInLoop(std::bind(high_water_mark_fn_, shared_from_this(), old_len + remaining));
		//}

		_output_buf->Put(static_cast<const char*>(data) + nwritten, remaining);
		if (!IsWritable()) {
			EnableWrite();
		}
	}
}
