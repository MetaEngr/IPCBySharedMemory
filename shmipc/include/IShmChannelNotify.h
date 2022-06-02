#pragma once

#ifdef DLL_SHM_IPC_EXPORT
#define SHM_IPC_EXPORT _declspec(dllexport)
#else
#define SHM_IPC_EXPORT _declspec(dllimport)
#endif

namespace ipc
{
class SHM_IPC_EXPORT IShmChannelNotify
{
public:
    IShmChannelNotify() {}
    virtual ~IShmChannelNotify() {}

    virtual void OnDataRecived(const char* buf, int size) = 0;
    virtual void OnWriteSuccess(int id) = 0;
    virtual void OnWriteFailed(int id) = 0;

private:
    IShmChannelNotify(const IShmChannelNotify&) = delete;
    IShmChannelNotify& operator=(const IShmChannelNotify&) = delete;
};

}  // namespace ipc
