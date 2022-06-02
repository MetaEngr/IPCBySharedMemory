#pragma once

namespace ipc
{
constexpr const int kSingleMsgBufferSize = 10240;  // 字节, 单条消息的最大长度
// 字节, 共享内存大小(100)
constexpr const int kShareMemBufferReserveSize = 100 + sizeof(char) + sizeof(unsigned int);
constexpr const int kShareMemBufferSize = kSingleMsgBufferSize + kShareMemBufferReserveSize;

// Global 对全局有效，要求权限较高
// constexpr const char* kShareMemServerWriteName = "Global\\MyFileMappingSWriteObject";  //指向服务端写共享内存的名字
// constexpr const char* kShareMemClientWriteName = "Global\\MyFileMappingCWriteObject";  //指向客户端写共享内存的名字
// constexpr const char* kShareMemMutexName = "Global\\MyMutexObject";              //指向互斥量的名字
// constexpr const char* kSemaphoreClientReadName = "Global\\MySemaphoreReadCObject";     //指向客户端读信号量的名字
// constexpr const char* kSemaphoreClientWriteName = "Global\\MySemaphoreWriteCObject";    //指向客户端写信号量的名字
// constexpr const char* kSemaphoreServerReadName = "Global\\MySemaphoreReadSObject";     //指向服务器读信号量的名字
// constexpr const char* kSemaphoreServerWriteName = "Global\\MySemaphoreWriteSObject";    //指向服务器写信号量的名字


// Local 对当前用户有效
constexpr const char* kShareMemServerWriteName = "Local\\MyFileMappingSWriteObject";  // 指向服务端写共享内存的名字
constexpr const char* kShareMemClientWriteName = "Local\\MyFileMappingCWriteObject";  // 指向客户端写共享内存的名字
constexpr const char* kShareMemMutexName = "Local\\MyMutexObject";              // 指向互斥量的名字
constexpr const char* kSemaphoreClientReadName = "Local\\MySemaphoreReadCObject";     // 指向客户端读信号量的名字
constexpr const char* kSemaphoreClientWriteName = "Local\\MySemaphoreWriteCObject";    // 指向客户端写信号量的名字
constexpr const char* kSemaphoreServerReadName = "Local\\MySemaphoreReadSObject";     // 指向服务器读信号量的名字
constexpr const char* kSemaphoreServerWriteName = "Local\\MySemaphoreWriteSObject";    // 指向服务器写信号量的名字

}  // namespace ipc
