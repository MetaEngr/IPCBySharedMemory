#include "ShmMsgDataBuffer.h"
#include <assert.h>

namespace ipc
{
MsgBuffer::MsgBuffer(int id, const char* buf, int size)
{
    id_ = id;
    if (size <= 0)
    {
        assert(false || "msg buf is empty!");
        msg_ = nullptr;
    }
    else
    {
        msg_ = new std::string(buf, size);
    }
}

MsgBuffer::~MsgBuffer()
{
    if (nullptr != msg_)
    {
        delete msg_;
        msg_ = nullptr;
    }
    id_ = -1;
}


ShmMsgDataBuffer::ShmMsgDataBuffer()
{}

ShmMsgDataBuffer::~ShmMsgDataBuffer()
{
    while (!msgQueue_.empty())
    {
        MsgBuffer* buf = msgQueue_.front();
        msgQueue_.pop();
        delete buf;
    }
}

bool ShmMsgDataBuffer::IsEmpty(void)
{
    std::unique_lock<std::mutex> locker(mutex_);

    return msgQueue_.empty();
}

size_t ShmMsgDataBuffer::Size(void)
{
    std::unique_lock<std::mutex> locker(mutex_);

    return msgQueue_.size();
}

void ShmMsgDataBuffer::Push(int id, const char* buf, int size)
{
    std::unique_lock<std::mutex> locker(mutex_);

    return msgQueue_.push(new MsgBuffer(id, buf, size));
}

void ShmMsgDataBuffer::Pop(void)
{
    std::unique_lock<std::mutex> locker(mutex_);

    MsgBuffer* buf = msgQueue_.front();
    msgQueue_.pop();
    delete buf;
}

bool ShmMsgDataBuffer::GetFront(int& id, std::string &msg)
{
    std::unique_lock<std::mutex> locker(mutex_);

    if (msgQueue_.empty())
    {
        return false;
    }
    MsgBuffer* buf = msgQueue_.front();
    buf->GetMsg(id, msg);
    return true;
}

}  // namespace ipc
