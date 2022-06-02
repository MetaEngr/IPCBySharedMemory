#include <stdio.h>
#include <assert.h>
#include <iostream>
#include "../shmipc/include/ShmIPCServer.h"
#include "../shmipc/include/IShmChannelNotify.h"

#ifdef _DEBUG
#ifdef _WIN64
#pragma comment(lib, "../x64/Debug/shmipc.lib")
#else
#pragma comment(lib, "../Win32/Debug/shmipc.lib")
#endif  // _WIN64
#else
#ifdef _WIN64
#pragma comment(lib, "../x64/Release/shmipc.lib")
#else
#pragma comment(lib, "../Win32/Release/shmipc.lib")
#endif  // _WIN64
#endif  // _DEBUG

class ShmChannelNotifyServer : public ipc::IShmChannelNotify
{
public:
    ShmChannelNotifyServer() {}
    ~ShmChannelNotifyServer() {}

    void OnDataRecived(const char* buf, int size) override
    {
        std::cout << "OnDataRecived: msg=" << buf << std::endl;
    }
    void OnWriteSuccess(int id) override
    {
        std::cout << "OnWriteSuccess: id=" << id << std::endl;
    }
    void OnWriteFailed(int id) override
    {
        std::cout << "OnWriteFailed: id=" << id << std::endl;
    }
};


void IpcServerTest()
{
    ShmChannelNotifyServer* notify = new ShmChannelNotifyServer;
    ipc::ShmIPCServer* server = new ipc::ShmIPCServer(notify);

    int ret = server->Init();
    if (0 != ret)
    {
        assert(false);
    }
    server->Start();

    int msgid = 0;
    while (true)
    {
        std::cout << "请输入待发送的数据:" << std::endl;
        char s[1024] = { 0 };
        int len = 0;
        while (true)
        {
            s[len++] = getchar();  // len = 0, then len + 1
            if (s[len - 1] == '\n')
                break;
        }
        server->SendMsg(msgid++, s, len);
        bool flag = false;
        if (flag)
        {
            break;
        }
    }
    server->Stop();
    delete server;
    delete notify;
}

int main(int argc, char* argv[])
{
    IpcServerTest();
    return 0;
}

