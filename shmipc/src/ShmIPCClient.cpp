#include "../include/ShmIPCClient.h"
#include <Windows.h>
#include <assert.h>
#include <iostream>
#include <string>
#include "../include/IShmChannelNotify.h"
#include "sharemem/SharedMemory.h"
#include "ShmIPCDefine.h"
#include "handler/ShmIPCRWHandler.h"
#include "StringTools.h"


namespace ipc
{
ShmIPCClient::ShmIPCClient(IShmChannelNotify* notify) :notify_(notify)
{
    assert(nullptr != notify);
    assert(kSingleMsgBufferSize <= kShareMemBufferSize);
}

ShmIPCClient::~ShmIPCClient()
{
    CleanUp();
}

void ShmIPCClient::CleanUp(void)
{
    // 唤醒所有线程
    ReleaseSemaphore(hSemaphoreClientWrite_, 1, NULL);
    ReleaseSemaphore(hSemaphoreClientRead_, 1, NULL);
    ReleaseSemaphore(hSemaphoreServerWrite_, 1, NULL);
    ReleaseSemaphore(hSemaphoreServerRead_, 1, NULL);

    if (nullptr != shmMsgHandler_)
    {
        shmMsgHandler_->Stop();
        delete shmMsgHandler_;
        shmMsgHandler_ = nullptr;
    }
    if (nullptr != shmServerWrite_)
    {
        delete shmServerWrite_;
        shmServerWrite_ = nullptr;
    }
    if (nullptr != shmClientWrite_)
    {
        delete shmClientWrite_;
        shmClientWrite_ = nullptr;
    }

    // 一旦不再需要，注意一定要用 CloseHandle 关闭互斥体句柄。如对象的所有句柄都已关闭，那么对象也会删除
    CloseHandle(hSemaphoreClientWrite_);  // 销毁信号量
    CloseHandle(hSemaphoreClientRead_);  // 销毁信号量
    CloseHandle(hSemaphoreServerWrite_);  // 销毁信号量
    CloseHandle(hSemaphoreServerRead_);  // 销毁信号量
}

int ShmIPCClient::Init(void)
{
    // 查询所有，根据kShareMemMutexName 打开互斥量
#pragma region Mutex
    int ret = 1;
    HANDLE hMutex = INVALID_HANDLE_VALUE;
    do
    {
        std::wstring wsName = tool::CharToWchar(kShareMemMutexName, CP_ACP);
        hMutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, wsName.c_str());
        if (hMutex)
        {
            std::cout << "互斥量打开成功！" << std::endl;
        }
        else
        {
            ret = 1;
            std::cout << "互斥量打开失败！" << std::endl;
            break;
        }

        std::cout << "等待... ... " << std::endl;
        DWORD res = 0xFFFFEEEE;
        res = WaitForSingleObject(hMutex, 20000);
        switch (res)
        {
        case WAIT_OBJECT_0:
            ret = 0;
            std::cout << "收到信号... ... " << std::endl;
            break;
        case WAIT_TIMEOUT:
            std::cout << "超时没有收到... ... " << std::endl;
            break;
        case WAIT_ABANDONED:
            std::cout << "另一个进程意外终止... ... " << std::endl;
            break;
        default:
            break;
        }
#pragma endregion Mutex

        if (0 == ret)
        {
            ret = InitSemphore();
        }
        if (0 == ret)
        {
            ret = InitSharedMemory();
        }
        if (0 == ret)
        {
            std::cout << "客户端已识别到服务器已准备好！" << std::endl;
        }

        ReleaseMutex(hMutex);   // 离开互斥区
    } while (false);

    CloseHandle(hMutex);
    return ret;
}

int ShmIPCClient::InitSemphore(void)
{
#pragma region Semphore
    std::wstring wsReadCName = tool::CharToWchar(kSemaphoreClientReadName, CP_ACP);
    hSemaphoreClientRead_ = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, wsReadCName.c_str());
    if (hSemaphoreClientRead_ == NULL)
    {
        std::cout << "客户端读信号量打开失败！" << std::endl;
        return 1;
    }

    std::wstring wsWriteCName = tool::CharToWchar(kSemaphoreClientWriteName, CP_ACP);
    hSemaphoreClientWrite_ = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, wsWriteCName.c_str());
    if (hSemaphoreClientWrite_ == NULL)
    {
        std::cout << "客户端写信号量打开失败！" << std::endl;
        return 1;
    }

    std::wstring wsWriteSName = tool::CharToWchar(kSemaphoreServerWriteName, CP_ACP);
    hSemaphoreServerWrite_ = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, wsWriteSName.c_str());
    if (hSemaphoreServerWrite_ == NULL)
    {
        std::cout << "服务器写信号量打开失败！" << std::endl;
        return 1;
    }

    std::wstring wsReadSName = tool::CharToWchar(kSemaphoreServerReadName, CP_ACP);
    hSemaphoreServerRead_ = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, wsReadSName.c_str());
    if (hSemaphoreServerRead_ == NULL)
    {
        std::cout << "服务器读信号量打开失败！" << std::endl;
        return 1;
    }
    ReleaseSemaphore(hSemaphoreClientWrite_, 1, NULL);  // 客户端可写信号量资源数加一
    // 在重启后,允许服务端读一次,防止上次服务端异常退出未读到数据
    ReleaseSemaphore(hSemaphoreServerRead_, 1, NULL);  // 服务端可读信号量资源数加一

#pragma endregion Semphore

    return 0;
}

int ShmIPCClient::InitSharedMemory(void)
{
    int ret = 0;
    if (nullptr == shmServerWrite_)
    {
        shmServerWrite_ = new shm::SharedMemory();
        ret = shmServerWrite_->Open(kShareMemServerWriteName, kShareMemBufferSize);
    }
    if (0 != ret)
    {
        return ret;
    }
    if (nullptr == shmClientWrite_)
    {
        shmClientWrite_ = new shm::SharedMemory();
        ret = shmClientWrite_->Open(kShareMemClientWriteName, kShareMemBufferSize);
    }
    return ret;
}

int ShmIPCClient::Start(void)
{
    if (nullptr == shmMsgHandler_)
    {
        shmMsgHandler_ = new ShmIPCRWHandler;
        shmMsgHandler_->SetSemaphore(hSemaphoreClientWrite_, hSemaphoreClientRead_,
            hSemaphoreServerWrite_, hSemaphoreServerRead_);
        shmMsgHandler_->SetNotify(notify_);
        shmMsgHandler_->AttachShm(shmClientWrite_, shmServerWrite_);
    }

    // 开启消息读写线程
    shmMsgHandler_->Start();

    return 0;
}

void ShmIPCClient::Stop(void)
{
    shmMsgHandler_->Stop();
}

int ShmIPCClient::SendMsg(int id, const char* buf, int size, bool synchronized)
{
    return shmMsgHandler_->SendMsg(id, buf, size, synchronized);
}
}  // namespace ipc
