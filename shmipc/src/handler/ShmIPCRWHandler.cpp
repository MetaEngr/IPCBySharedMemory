#include "ShmIPCRWHandler.h"
#include <Windows.h>
#include <assert.h>
#include <iostream>
#include <thread>
#include "../ShmIPCDefine.h"
#include "../ShmMsgDataBuffer.h"
#include "../../include/IShmChannelNotify.h"
#include "../sharemem/SharedMemory.h"

namespace ipc
{
ShmIPCRWHandler::ShmIPCRWHandler() : msgReadBufferSize_(kSingleMsgBufferSize), msgBuffer_(new ShmMsgDataBuffer)
{
    msgReadBuffer_ = new char[msgReadBufferSize_];
    memset(msgReadBuffer_, 0, msgReadBufferSize_);
}

ShmIPCRWHandler::~ShmIPCRWHandler()
{
    this->Stop();

    if (nullptr != msgBuffer_)
    {
        delete msgBuffer_;
        msgBuffer_ = nullptr;
    }
    if (nullptr != msgReadBuffer_)
    {
        delete[] msgReadBuffer_;
        msgReadBuffer_ = nullptr;
    }
}

void ShmIPCRWHandler::SetSemaphore(HANDLE selfWrite, HANDLE selfRead, HANDLE otherWrite, HANDLE otherRead)
{
    assert(selfWrite != NULL && selfWrite != INVALID_HANDLE_VALUE);
    assert(selfRead != NULL && selfRead != INVALID_HANDLE_VALUE);

    assert(otherWrite != NULL && otherWrite != INVALID_HANDLE_VALUE);
    assert(otherRead != NULL && otherRead != INVALID_HANDLE_VALUE);

    hSemaphoreSelfRead_ = selfWrite;
    hSemaphoreSelfWrite_ = selfRead;
    hSemaphoreOtherRead_ = otherWrite;
    hSemaphoreOtherWrite_ = otherRead;
}

void ShmIPCRWHandler::AttachShm(shm::SharedMemory* shmWrite, shm::SharedMemory* shmRead)
{
    assert(nullptr != shmWrite);
    assert(nullptr != shmRead);

    shareMemoryWrite_ = shmWrite;
    shareMemoryRead_ = shmRead;
}

void ShmIPCRWHandler::SetNotify(IShmChannelNotify* notify)
{
    assert(nullptr != notify);

    notify_ = notify;
}

int ShmIPCRWHandler::SendMsg(int id, const char* buf, int size, bool synchronized)
{
    assert(kSingleMsgBufferSize >= size);
    assert(!synchronized);
    // if (synchronized)
    //{
    //    //  暂不支持同步
    //}

    msgBuffer_->Push(id, buf, size);

    // 允许自身写一次，上次对侧读完后若 msgBuffer_ 中无消息，则会再次等待可读信号
    ReleaseSemaphore(hSemaphoreSelfWrite_, 1, NULL);

    return 0;
}

void ShmIPCRWHandler::Start(void)
{
    std::thread th(&ShmIPCRWHandler::ReadWriteThreadFunc, this);
    th.detach();
}

void ShmIPCRWHandler::Stop(void)
{
    stopFlag_ = true;
}

void ShmIPCRWHandler::ReadWriteThreadFunc(void)
{
    HANDLE rwSemaphores[kSemaphoreObjectNum] = {
    hSemaphoreSelfRead_, hSemaphoreSelfWrite_
    };  // 0-读  1-写

    while (!stopFlag_)
    {
        DWORD res = 0xFFFFEEEE;
        res = WaitForMultipleObjects(kSemaphoreObjectNum, rwSemaphores, false, INFINITE);
        switch (res)
        {
        case WAIT_OBJECT_0:
        {
            // 对侧可读
            int readLen = shareMemoryRead_->Read(msgReadBuffer_, msgReadBufferSize_);
            ReleaseSemaphore(hSemaphoreOtherWrite_, 1, NULL);  // 对侧可写信号量资源数加一
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
        case (WAIT_OBJECT_0 + 1):
        {
            // 本侧可写
            int msgId = -1;
            std::string msg = "";
            if (!msgBuffer_->IsEmpty())
            {
                msgBuffer_->GetFront(msgId, msg);

                int writeSize = shareMemoryWrite_->Write(msg.c_str(), msg.size());
                //assert(writeSize == msg.size());  // 允许失败，此时对侧可能还没读，读完后对侧会重新开放可写信号
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
                ReleaseSemaphore(hSemaphoreOtherRead_, 1, NULL);  // 可读信号量资源数加一，写失败时让客户端再读一次
            }
            //else
            //{
            //    assert(false);  // 出错了
            //    fprintf(stderr, "ShmIPCWriter::Process(void) error!");
            //}
        }
        break;
        default:
            break;
        }
    }
}

}  // namespace ipc
