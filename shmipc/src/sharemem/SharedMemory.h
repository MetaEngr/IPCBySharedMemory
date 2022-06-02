#pragma once

#include <Windows.h>
#include <string>

namespace shm
{
class SharedMemory
{
    enum ShmFlag
    {
        Initialized = 0,  // 初始化
        HasWrite,         // 已写入
        HasRead,          // 已读取
    };

public:
    SharedMemory();
    ~SharedMemory();

    // 先尝试打开已存在的共享内存，若不存在则创建
    int Open(const char* shmName, unsigned int shmSize);
    // 写共享内存, 返回真正写入的大小
    unsigned int Write(const char* buf, unsigned int size);
    // 读共享内存, 返回真正读取的大小
    unsigned int Read(char*buf, unsigned int size);

private:
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    // 创建共享内存
    int CreateSharedMemory(const char* shmName, unsigned int shmSize);
    // 打开一个已存在的共享内存
    int OpenSharedMemory(const char* shmName, unsigned int shmSize);
    // 清理
    void CleanUp(void);

private:
    // 共享内存文件句柄
    HANDLE shmMapFile_ = NULL;
    void* shmAddr_ = NULL;

    // 记录信息
    std::string shmName_ = "";
    unsigned int shmCapacity_ = 0;
};

}  // namespace shm
