#include "ShmIPCWriter.h"
#include <Windows.h>
#include <assert.h>
#include <iostream>
#include "../ShmIPCDefine.h"
#include "../ShmMsgDataBuffer.h"
#include "../../include/IShmChannelNotify.h"
#include "../sharemem/SharedMemory.h"

namespace ipc
{
ShmIPCWriter::ShmIPCWriter() : ThreadCtrl(), msgBuffer_(new ShmMsgDataBuffer)
{}

ShmIPCWriter::~ShmIPCWriter()
{
    if (nullptr != msgBuffer_)
    {
        delete msgBuffer_;
        msgBuffer_ = nullptr;
    }
}

void ShmIPCWriter::SetSemaphore(HANDLE semaphoreWrite, HANDLE semaphoreRead)
{
    assert(semaphoreWrite != NULL && semaphoreWrite != INVALID_HANDLE_VALUE);
    assert(semaphoreRead != NULL && semaphoreRead != INVALID_HANDLE_VALUE);

    hSemaphoreWrite_ = semaphoreWrite;
    hSemaphoreRead_ = semaphoreRead;
}

void ShmIPCWriter::AttachShm(shm::SharedMemory* shm)
{
    assert(nullptr != shm);

    shareMemory_ = shm;
}

void ShmIPCWriter::SetNotify(IShmChannelNotify* notify)
{
    assert(nullptr != notify);

    notify_ = notify;
}

int ShmIPCWriter::SendMsg(int id, const char* buf, int size, bool synchronized)
{
    assert(kSingleMsgBufferSize >= size);
    assert(!synchronized);
    // if (synchronized)
    //{
    //    //  暂不支持同步
    //}

    msgBuffer_->Push(id, buf, size);

    this->Resume();  // 唤醒线程
    return 0;
}

void ShmIPCWriter::Process(void)
{
    if (msgBuffer_->IsEmpty())
    {
        this->Pause();  // 消息队列为空时，挂起线程，等待唤醒
        return;
    }

    DWORD res = 0xFFFFEEEE;
    res = WaitForSingleObject(hSemaphoreWrite_, INFINITE);
    switch (res)
    {
    case WAIT_OBJECT_0:
    {
        // 可写
        int msgId = -1;
        std::string msg = "";
        if (!msgBuffer_->IsEmpty())
        {
            msgBuffer_->GetFront(msgId, msg);

            int writeSize = shareMemory_->Write(msg.c_str(), msg.size());
            assert(writeSize == msg.size());
            if (writeSize != msg.size())
            {
                fprintf(stderr, "ShmIPCWriter::Process(void) error! writeSize != msg.size().");
                notify_->OnWriteFailed(msgId);
            }
            else
            {
                msgBuffer_->Pop();
                std::cout << "写入数据： " << msg << std::endl;
                if (nullptr != notify_ && msgId >= 0)
                {
                    notify_->OnWriteSuccess(msgId);
                }
            }
            ReleaseSemaphore(hSemaphoreRead_, 1, NULL);  // 可读信号量资源数加一，写失败时让客户端再读一次
        }
        else
        {
            assert(false);  // 出错了
            fprintf(stderr, "ShmIPCWriter::Process(void) error!");
        }
    }
    break;
    default:
        break;
    }
}

}  // namespace ipc
