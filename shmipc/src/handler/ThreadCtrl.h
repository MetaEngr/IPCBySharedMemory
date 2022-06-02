#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace ipc
{
class ThreadCtrl
{
public:
    ThreadCtrl();
    virtual ~ThreadCtrl();

    enum ENState
    {
        Stoped,     // /<停止状态，包括从未启动过和启动后被停止
        Running,    // /<运行状态
        Paused      // /<暂停状态
    };

    ENState State(void) const;
    void Start(void);
    void Stop(void);
    void Pause(void);
    void Resume(void);

    void SetName(const char* name, size_t len);

protected:
    // 线程循环内任务，不允许在此方法内写死循环
    virtual void Process(void) = 0;

private:
    ThreadCtrl(const ThreadCtrl&) = delete;
    ThreadCtrl& operator=(const ThreadCtrl&) = delete;

    // 线程任务
    void Run(void);

private:
    std::thread* thread_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic_bool pauseFlag_;   // /<暂停标识
    std::atomic_bool stopFlag_;   // /<停止标识
    ENState state_;

    char* name_ = nullptr;  // 线程名称
};
}  // namespace ipc
