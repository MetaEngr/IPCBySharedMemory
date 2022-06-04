#pragma once

#include <queue>
#include <string>
#include <mutex>

namespace ipc
{
class MsgBuffer
{
public:
    explicit MsgBuffer(int id, const char* buf, int size);
    ~MsgBuffer();

    inline bool GetMsg(int& id, std::string &msg) const
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
    ShmMsgDataBuffer();
    ~ShmMsgDataBuffer();

    bool IsEmpty(void);
    size_t Size(void);
    void Push(int id, const char* buf, int size);
    void Pop(void);
    bool GetFront(int& id, std::string &msg);

private:
    std::queue<MsgBuffer*> msgQueue_;
    std::mutex mutex_;

private:
    ShmMsgDataBuffer(const ShmMsgDataBuffer&) = delete;
    ShmMsgDataBuffer& operator=(const ShmMsgDataBuffer&) = delete;
};

}  // namespace ipc
