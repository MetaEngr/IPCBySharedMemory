#pragma once

#ifdef DLL_SHM_IPC_EXPORT
#define SHM_IPC_EXPORT _declspec(dllexport)
#else
#define SHM_IPC_EXPORT _declspec(dllimport)
#endif

#include <Windows.h>

namespace shm
{
class SharedMemory;
}  // namespace shm

namespace ipc
{
class IShmChannelNotify;
class ShmIPCReader;
class ShmIPCWriter;

class SHM_IPC_EXPORT ShmIPCServer
{
public:
    explicit ShmIPCServer(IShmChannelNotify* notify);
    ~ShmIPCServer();

    // 初始化
    int Init(void);
    // 开启工作线程
    int Start(void);
    // 停止工作线程
    void Stop(void);
    // 发送客户端数据 (暂不支持同步发送)
    int SendMsg(int id, const char* buf, int size, bool synchronized = false);

private:
    ShmIPCServer(const ShmIPCServer&) = delete;
    ShmIPCServer& operator=(const ShmIPCServer&) = delete;

    int InitSemphore(void);
    int InitSharedMemory(void);
    void CleanUp(void);

private:
    // 信号量(约定:信号量由服务端创建，客户端只能打开不能创建)
    HANDLE hSemaphoreClientRead_ = NULL;
    HANDLE hSemaphoreClientWrite_ = NULL;
    HANDLE hSemaphoreServerRead_ = NULL;
    HANDLE hSemaphoreServerWrite_ = NULL;

    // 共享内存
    shm::SharedMemory* shmServerWrite_ = nullptr;  // server->client 方向消息
    shm::SharedMemory* shmClientWrite_ = nullptr;  // client->server 方向消息

    // 线程
    ShmIPCWriter* shmServerMsgWriter_ = nullptr;
    ShmIPCReader* shmClientMsgReader_ = nullptr;

    // 不管理生命周期
    IShmChannelNotify* notify_ = nullptr;
};
}  // namespace ipc
