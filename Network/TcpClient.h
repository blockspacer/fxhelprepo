#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <WinSock.h>

#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/timer/timer.h"
#include "third_party/chromium/base/threading/thread.h"

// 提供TCP连接功能，发送数据，接收数据；
// 需要关联一个线程进去，运行收发数据
class Thread;

typedef SOCKET TcpHandle;

typedef void(*NotifyFunction)(void* privatedata, const std::vector<char>& data);
class TcpClient
    //:std::enable_shared_from_this<TcpClient>
{
public:
    TcpClient();
    ~TcpClient();

    bool Initialize();
    void Finalize();

    bool Connect(const std::string& ip, unsigned short port);
    bool Send(const std::vector<char>& data);
    bool Recv(std::vector<char>* buffer);

private:
    SOCKET socket_;
};


