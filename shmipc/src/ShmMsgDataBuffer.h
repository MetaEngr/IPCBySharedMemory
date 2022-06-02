#pragma once

#include <queue>
#include <string>

namespace ipc
{
class MsgBuffer
{
public:
    MsgBuffer(int id, const char* buf, int size)
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

    ~MsgBuffer()
    {
        if (nullptr != msg_)
        {
            delete msg_;
            msg_ = nullptr;
        }
        id_ = -1;
    }

    bool GetMsg(int& id, std::string &msg) const
    {
        id = id_;
        msg = *msg_;
        return true;
    }

private:
    int id_ = -1;
    std::string* msg_ = nullptr;
};

class ShmMsgDataBuffer
{
public:
    ShmMsgDataBuffer() {}
    ~ShmMsgDataBuffer()
    {
        while (!msgQueue_.empty())
        {
            MsgBuffer* buf = msgQueue_.front();
            msgQueue_.pop();
            delete buf;
        }
    }

    inline bool IsEmpty(void) const { return msgQueue_.empty(); }
    inline size_t Size(void) const { return msgQueue_.size(); }
    inline void Push(int id, const char* buf, int size) {
        return msgQueue_.push(new MsgBuffer(id, buf, size));
    }
    void Pop(void)
    {
        MsgBuffer* buf = msgQueue_.front();
        msgQueue_.pop();
        delete buf;
    }
    bool GetFront(int& id, std::string &msg)
    {
        if (msgQueue_.empty())
        {
            return false;
        }
        MsgBuffer* buf = msgQueue_.front();
        buf->GetMsg(id, msg);
        return true;
    }

private:
    std::queue<MsgBuffer*> msgQueue_;

private:
    ShmMsgDataBuffer(const ShmMsgDataBuffer&) = delete;
    ShmMsgDataBuffer& operator=(const ShmMsgDataBuffer&) = delete;
};

}  // namespace ipc
