#pragma once
#include <Windows.h>
#include "ThreadCtrl.h"

namespace shm
{
class SharedMemory;
}  // namespace shm

namespace ipc
{
class IShmChannelNotify;

class ShmIPCReader : public ThreadCtrl
{
public:
    ShmIPCReader();
    ~ShmIPCReader();

    // 参数设置
    void SetSemaphore(HANDLE semaphoreWrite, HANDLE semaphoreRead);
    void AttachShm(shm::SharedMemory* shm);
    void SetNotify(IShmChannelNotify* notify);

protected:
    void Process(void) override;

private:
    ShmIPCReader(const ShmIPCReader&) = delete;
    ShmIPCReader& operator=(const ShmIPCReader&) = delete;

private:
    // 读取一次消息的消息缓存,防止频繁创建
    const unsigned int msgReadBufferSize_;
    char* msgReadBuffer_ = nullptr;

    // 不管理生命周期
    HANDLE hSemaphoreWrite_ = NULL;  // 可写信号量
    HANDLE hSemaphoreRead_ = NULL;   // 可读信号量
    shm::SharedMemory* shareMemory_ = nullptr;  // 共享内存
    IShmChannelNotify* notify_ = nullptr;    // 通知回调
};

}  // namespace ipc
