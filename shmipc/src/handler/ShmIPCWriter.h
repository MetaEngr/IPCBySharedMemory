#pragma once
#include <Windows.h>
#include "ThreadCtrl.h"

namespace shm
{
class SharedMemory;
}  // namespace shm

namespace ipc
{
class ShmMsgDataBuffer;
class IShmChannelNotify;

class ShmIPCWriter : public ThreadCtrl
{
public:
    ShmIPCWriter();
    ~ShmIPCWriter();

    // 参数设置
    void SetSemaphore(HANDLE semaphoreWrite, HANDLE semaphoreRead);
    void AttachShm(shm::SharedMemory* shm);
    void SetNotify(IShmChannelNotify* notify);

    // 发送数据 (暂不支持同步发送)
    int SendMsg(int id, const char* buf, int size, bool synchronized = false);

protected:
    void Process(void) override;

private:
    ShmIPCWriter(const ShmIPCWriter&) = delete;
    ShmIPCWriter& operator=(const ShmIPCWriter&) = delete;

private:
    // 管理生命周期
    ShmMsgDataBuffer* msgBuffer_ = nullptr;  // 消息缓存

    // 不管理生命周期
    HANDLE hSemaphoreWrite_ = NULL;  // 可写信号量
    HANDLE hSemaphoreRead_ = NULL;   // 可读信号量
    shm::SharedMemory* shareMemory_ = nullptr;  // 共享内存
    IShmChannelNotify* notify_ = nullptr;    // 通知回调
};

}  // namespace ipc
