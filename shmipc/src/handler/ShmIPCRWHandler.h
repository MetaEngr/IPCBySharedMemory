#pragma once
#include <Windows.h>
#include <atomic>

namespace shm
{
class SharedMemory;
}  // namespace shm

namespace ipc
{
class ShmMsgDataBuffer;
class IShmChannelNotify;

constexpr const int kSemaphoreObjectNum = 2;  // 信号量对象数,读写各一个


class ShmIPCRWHandler
{
public:
    ShmIPCRWHandler();
    ~ShmIPCRWHandler();

    // 参数设置
    void SetSemaphore(HANDLE selfWrite, HANDLE selfRead, HANDLE otherWrite, HANDLE otherRead);
    void AttachShm(shm::SharedMemory* shmWrite, shm::SharedMemory* shmRead);
    void SetNotify(IShmChannelNotify* notify);

    // 发送数据 (暂不支持同步发送)
    int SendMsg(int id, const char* buf, int size, bool synchronized = false);

    void Start(void);
    void Stop(void);

protected:
    void ReadWriteThreadFunc(void);

private:
    ShmIPCRWHandler(const ShmIPCRWHandler&) = delete;
    ShmIPCRWHandler& operator=(const ShmIPCRWHandler&) = delete;

private:
    // 管理生命周期
    // 读取一次消息的消息缓存,防止频繁创建
    const unsigned int msgReadBufferSize_;
    char* msgReadBuffer_ = nullptr;
    std::atomic_bool stopFlag_ = false;   // 停止标识
    ShmMsgDataBuffer* msgBuffer_ = nullptr;  // 消息缓存

    // 不管理生命周期
    HANDLE hSemaphoreSelfRead_ = NULL;  // 本侧读
    HANDLE hSemaphoreSelfWrite_ = NULL;  // 本侧写
    HANDLE hSemaphoreOtherRead_ = NULL;  // 对侧读
    HANDLE hSemaphoreOtherWrite_ = NULL;  // 对侧写
    shm::SharedMemory* shareMemoryWrite_ = nullptr;  // 本侧写共享内存
    shm::SharedMemory* shareMemoryRead_ = nullptr;  // 读对侧共享内存
    IShmChannelNotify* notify_ = nullptr;    // 通知回调
};

}  // namespace ipc
