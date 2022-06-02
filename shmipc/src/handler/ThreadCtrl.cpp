#include "ThreadCtrl.h"
#include <iostream>

namespace ipc
{
ThreadCtrl::ThreadCtrl()
    : thread_(nullptr),
    pauseFlag_(false),
    stopFlag_(false),
    state_(Stoped)
{
}

ThreadCtrl::~ThreadCtrl()
{
    Stop();
}

ThreadCtrl::ENState ThreadCtrl::State(void) const
{
    return state_;
}

void ThreadCtrl::Start(void)
{
    if (nullptr != name_)
    {
        delete[] name_;
    }
    if (thread_ == nullptr)
    {
        thread_ = new std::thread(&ThreadCtrl::Run, this);
        pauseFlag_ = false;
        stopFlag_ = false;
        state_ = Running;
    }
}

void ThreadCtrl::Stop(void)
{
    if (thread_ != nullptr)
    {
        pauseFlag_ = false;
        stopFlag_ = true;
        condition_.notify_all();  // Notify one waiting thread, if there is one.
        thread_->join();  // wait for thread finished
        delete thread_;
        thread_ = nullptr;
        state_ = Stoped;
    }
}

void ThreadCtrl::Pause(void)
{
    if (thread_ != nullptr)
    {
        pauseFlag_ = true;
        state_ = Paused;
    }
}

void ThreadCtrl::Resume(void)
{
    if (thread_ != nullptr)
    {
        pauseFlag_ = false;
        condition_.notify_all();
        state_ = Running;
    }
}

void ThreadCtrl::SetName(const char* name, size_t len)
{
    if (nullptr != name_)
    {
        delete[] name_;
        name_ = nullptr;
    }
    if (len > 0)
    {
        name_ = new char[len + 1];
        memcpy_s(name_, (len+1), name, len);
        name_[len] = '\0';
    }
}

void ThreadCtrl::Run(void)
{
    std::cout << "enter thread:" << std::this_thread::get_id() << std::endl;

    while (!stopFlag_)
    {
        Process();
        if (pauseFlag_)
        {
            std::unique_lock<std::mutex> locker(mutex_);
            while (pauseFlag_)
            {
                condition_.wait(locker);  // Unlock mutex_ and wait to be notified
            }
            locker.unlock();
        }
    }
    pauseFlag_ = false;
    stopFlag_ = false;

    std::cout << "exit thread:" << std::this_thread::get_id() << std::endl;
}
}  // namespace ipc
