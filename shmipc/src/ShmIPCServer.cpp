#include "../include/ShmIPCServer.h"
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
ShmIPCServer::ShmIPCServer(IShmChannelNotify* notify) :notify_(notify)
{
    assert(nullptr != notify);
    assert(kSingleMsgBufferSize <= kShareMemBufferSize);
}

ShmIPCServer::~ShmIPCServer()
{
    CleanUp();
}

void ShmIPCServer::CleanUp(void)
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

    CloseHandle(hSemaphoreClientWrite_);  // 销毁信号量
    CloseHandle(hSemaphoreClientRead_);  // 销毁信号量
    CloseHandle(hSemaphoreServerWrite_);  // 销毁信号量
    CloseHandle(hSemaphoreServerRead_);  // 销毁信号量
}

int ShmIPCServer::Init(void)
{
#pragma region Mutex
    HANDLE hMutex = NULL;
    // 创建互斥体，是其他进程等待互斥体
    std::wstring wsName = tool::CharToWchar(kShareMemMutexName, CP_ACP);
    hMutex = CreateMutex(NULL, TRUE, wsName.c_str());
    if (hMutex)
    {
        std::cout << "互斥量创建成功！" << std::endl;
    }
    else
    {
        std::cout << "互斥量创建失败！" << std::endl;
        return 1;
    }

#pragma endregion Mutex

    int ret = InitSemphore();
    if (0 == ret)
    {
        ret = InitSharedMemory();
    }
    if (0 == ret)
    {
        std::cout << "服务器已准备好！" << std::endl;
    }

    ReleaseMutex(hMutex);   // 离开互斥区
    // CloseHandle(hMutex);  // todo

    return ret;
}

int ShmIPCServer::InitSemphore(void)
{
#pragma region Semphore
    std::wstring wsReadCName = tool::CharToWchar(kSemaphoreClientReadName, CP_ACP);
    hSemaphoreClientRead_ = CreateSemaphore(NULL, 0, 10, wsReadCName.c_str());
    if (hSemaphoreClientRead_ == NULL)
    {
        std::cout << "客户端读信号量创建失败！" << std::endl;

        return 1;
    }
    // ReleaseSemaphore(hSemaphoreRead, 1, NULL);  //信号量资源数加一

    std::wstring wsWriteCName = tool::CharToWchar(kSemaphoreClientWriteName, CP_ACP);
    hSemaphoreClientWrite_ = CreateSemaphore(NULL, 0, 1, wsWriteCName.c_str());
    if (hSemaphoreClientWrite_ == NULL)
    {
        std::cout << "客户端写信号量创建失败！" << std::endl;

        return 1;
    }

    std::wstring wsReadSName = tool::CharToWchar(kSemaphoreServerReadName, CP_ACP);
    hSemaphoreServerRead_ = CreateSemaphore(NULL, 0, 10, wsReadSName.c_str());
    if (hSemaphoreServerRead_ == NULL)
    {
        std::cout << "服务器读信号量创建失败！" << std::endl;

        return 1;
    }
    // ReleaseSemaphore(hSemaphoreRead, 1, NULL);  //信号量资源数加一

    std::wstring wsWritesName = tool::CharToWchar(kSemaphoreServerWriteName, CP_ACP);
    hSemaphoreServerWrite_ = CreateSemaphore(NULL, 0, 1, wsWritesName.c_str());
    if (hSemaphoreServerWrite_ == NULL)
    {
        std::cout << "服务器写信号量创建失败！" << std::endl;

        return 1;
    }

    ReleaseSemaphore(hSemaphoreServerWrite_, 1, NULL);  // 服务端可写信号量资源数加一
    // 在重启后,允许客户端读一次,防止上次客户端异常退出未读到数据
    ReleaseSemaphore(hSemaphoreClientRead_, 1, NULL);  // 客户端可读信号量资源数加一

#pragma endregion Semphore

    return 0;
}

int ShmIPCServer::InitSharedMemory(void)
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

int ShmIPCServer::Start(void)
{
    if (nullptr == shmMsgHandler_)
    {
        shmMsgHandler_ = new ShmIPCRWHandler;
        shmMsgHandler_->SetSemaphore(hSemaphoreServerWrite_, hSemaphoreServerRead_,
            hSemaphoreClientWrite_, hSemaphoreClientRead_);
        shmMsgHandler_->SetNotify(notify_);
        shmMsgHandler_->AttachShm(shmServerWrite_, shmClientWrite_);
    }

    // 开启读写线程
    shmMsgHandler_->Start();

    return 0;
}

void ShmIPCServer::Stop(void)
{
    shmMsgHandler_->Stop();
}

int ShmIPCServer::SendMsg(int id, const char* buf, int size, bool synchronized)
{
    return shmMsgHandler_->SendMsg(id, buf, size, synchronized);
}

}  // namespace ipc
