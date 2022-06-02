#include "SharedMemory.h"
#include <assert.h>
#include "../ShmIPCDefine.h"
#include "../StringTools.h"

namespace shm
{
SharedMemory::SharedMemory() {}
SharedMemory::~SharedMemory()
{
    CleanUp();
}

void SharedMemory::CleanUp(void)
{
    CloseHandle(shmMapFile_);
}

int SharedMemory::Open(const char* shmName, unsigned int shmSize)
{
    assert(nullptr != shmName);
    assert(shmSize > ipc::kShareMemBufferReserveSize);

    shmName_ = shmName;
    shmCapacity_ = shmSize + sizeof(unsigned int);  // 加 unsigned int 类型的写入大小标记
    int ret = OpenSharedMemory(shmName, shmCapacity_);
    if (0 != ret)
    {
        ret = CreateSharedMemory(shmName, shmCapacity_);
    }
    if (0 == ret)
    {
        char flag = ShmFlag::Initialized;  // 更新标志
        memcpy_s(shmAddr_, shmCapacity_, reinterpret_cast<void*>(&flag), sizeof(flag));  // 写1字节标志
    }
    return ret;
}

int SharedMemory::CreateSharedMemory(const char* shmName, unsigned int shmSize)
{
#pragma region SharedMemory
    std::wstring wsShmName = tool::CharToWchar(shmName, CP_ACP);
    // 创建内存共享区域
    shmMapFile_ = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE,          // read/write access
        0,                       // maximum object size (high-order DWORD)
        shmSize,                 // maximum object size (low-order DWORD)
        wsShmName.c_str());                // name of mapping object

    assert(shmMapFile_ != NULL);
    if (shmMapFile_ == NULL)
    {
        printf("Could not create file mapping object (%d).\n", GetLastError());
        return 1;
    }

    // 将内存映射到该进程
    shmAddr_ = (void*)MapViewOfFile(shmMapFile_,   // handle to map object
        FILE_MAP_ALL_ACCESS,  // read/write permission
        0,
        0,
        shmSize);

    assert(shmAddr_ != NULL);
    if (shmAddr_ == NULL)
    {
        printf("Could not map view of file (%d).\n", GetLastError());
        CloseHandle(shmMapFile_);
        return 1;
    }

#pragma endregion SharedMemory
    return 0;
}

int SharedMemory::OpenSharedMemory(const char* shmName, unsigned int shmSize)
{
#pragma region SharedMemory
    std::wstring wsShmName = tool::CharToWchar(shmName, CP_ACP);
    // 创建内存共享区域
    shmMapFile_ = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, wsShmName.c_str());
    // assert(shmMapFile_ != NULL);
    if (shmMapFile_ == NULL)
    {
        printf("Could not open file mapping object (%d).\n", GetLastError());
        return 1;
    }
    // 将内存映射到该进程
    shmAddr_ = (void*)MapViewOfFile(shmMapFile_,   // handle to map object
        FILE_MAP_ALL_ACCESS,  // read/write permission
        0,
        0,
        shmSize);

    // assert(shmAddr_ != NULL);
    if (shmAddr_ == NULL)
    {
        printf("Could not map view of file (%d).\n", GetLastError());
        CloseHandle(shmMapFile_);
        return 1;
    }
#pragma endregion SharedMemory

    return 0;
}

/////////////////////////////////////////////////////////////////

// 共享内存分布
// |--初始化标志--|------本次写入长度-------|---------本次写入消息缓存数据---------|
// |-flag(char)-|-writeLen(unsigned int)-|-writeBuf(bufsize * sizeof(char))-|

/////////////////////////////////////////////////////////////////

// 返回真正写入的大小
unsigned int SharedMemory::Write(const char* buf, unsigned int size)
{
    char flag = ShmFlag::Initialized;
    memcpy_s(reinterpret_cast<void*>(&flag), sizeof(flag), shmAddr_, sizeof(flag));  // 读取当前标志
    if (flag == ShmFlag::HasWrite)
    {
        return 0;  // 上次写入未被读取时直接返回
    }

    unsigned int len = size;
    int shmSize = shmCapacity_ - sizeof(len) - sizeof(flag);

    assert(shmSize >= static_cast<int>(len));
    if (shmSize < static_cast<int>(len))
    {
        fprintf(stderr, "out of shared memory!");
    }
    flag = ShmFlag::HasWrite;
    void* writeAddr = shmAddr_;
    memcpy_s(writeAddr, shmCapacity_, reinterpret_cast<void*>(&flag), sizeof(flag));  // 写1字节标志
    writeAddr = (reinterpret_cast<char*>(shmAddr_)) + sizeof(flag);
    memcpy_s(writeAddr, (shmCapacity_ - sizeof(flag)), reinterpret_cast<void*>(&len), sizeof(len));  // 写4字节长度
    writeAddr = (reinterpret_cast<char*>(writeAddr)) + sizeof(len);
    memcpy_s(writeAddr, shmSize, reinterpret_cast<const void*>(buf), len);  // write msg

    return len;
}
// 返回真正读取的大小
unsigned int SharedMemory::Read(char* buf, unsigned int size)
{
    char flag = ShmFlag::Initialized;
    void* readAddr = shmAddr_;
    memcpy_s(reinterpret_cast<void*>(&flag), sizeof(flag), readAddr, sizeof(flag));  // 读取当前标志
    if (flag != ShmFlag::HasWrite)
    {
        return 0;  // 未写入时直接返回
    }

    unsigned int len = 0;
    readAddr = (reinterpret_cast<char*>(readAddr)) + sizeof(flag);
    memcpy_s(reinterpret_cast<void*>(&len), sizeof(len), readAddr, sizeof(len));  // 获得写入大小

    int shmSize = shmCapacity_ - sizeof(len);
    assert(static_cast<int>(len) <= shmSize);
    if (static_cast<int>(len) > shmSize)
    {
        fprintf(stderr, "share memory error!");
        len = shmSize;
    }

    assert(len <= size);
    if (len > size)
    {
        fprintf(stderr, "more buffer size is need!");
        len = size;
    }

    readAddr = (reinterpret_cast<char*>(readAddr)) + sizeof(len);
    memcpy_s(buf, size, readAddr, len);  // read msg

    // 更新标志
    flag = ShmFlag::HasRead;
    memcpy_s(shmAddr_, shmCapacity_, reinterpret_cast<void*>(&flag), sizeof(flag));  // 写1字节标志
    return len;
}

}  // namespace shm
