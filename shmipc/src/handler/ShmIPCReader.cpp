#include "ShmIPCReader.h"
#include <Windows.h>
#include <assert.h>
#include <iostream>
#include "../ShmIPCDefine.h"
#include "../../include/IShmChannelNotify.h"
#include "../sharemem/SharedMemory.h"

namespace ipc
{
ShmIPCReader::ShmIPCReader() : ThreadCtrl(), msgReadBufferSize_(kSingleMsgBufferSize)
{
    msgReadBuffer_ = new char[msgReadBufferSize_];
    memset(msgReadBuffer_, 0, msgReadBufferSize_);
}

ShmIPCReader::~ShmIPCReader()
{
    if (nullptr != msgReadBuffer_)
    {
        delete[] msgReadBuffer_;
        msgReadBuffer_ = nullptr;
    }
}

void ShmIPCReader::SetSemaphore(HANDLE semaphoreWrite, HANDLE semaphoreRead)
{
    assert(semaphoreWrite != NULL && semaphoreWrite != INVALID_HANDLE_VALUE);
    assert(semaphoreRead != NULL && semaphoreRead != INVALID_HANDLE_VALUE);

    hSemaphoreWrite_ = semaphoreWrite;
    hSemaphoreRead_ = semaphoreRead;
}

void ShmIPCReader::AttachShm(shm::SharedMemory* shm)
{
    assert(nullptr != shm);

    shareMemory_ = shm;
}

void ShmIPCReader::SetNotify(IShmChannelNotify* notify)
{
    assert(nullptr != notify);

    notify_ = notify;
}

void ShmIPCReader::Process(void)
{
    DWORD res = 0xFFFFEEEE;
    res = WaitForSingleObject(hSemaphoreRead_, INFINITE);
    switch (res)
    {
    case WAIT_OBJECT_0:
    {
        // 可读
        int readLen = shareMemory_->Read(msgReadBuffer_, msgReadBufferSize_);
        ReleaseSemaphore(hSemaphoreWrite_, 1, NULL);  // 可写信号量资源数加一
        if (readLen > 0)
        {
            std::cout << "读取数据： " << msgReadBuffer_ << std::endl;
            if (nullptr != notify_)
            {
                notify_->OnDataRecived(msgReadBuffer_, readLen);
            }
            memset(msgReadBuffer_, 0, readLen);  // 处理后清除数据
        }
    }
    break;
    default:
        break;
    }
}

}  // namespace ipc
