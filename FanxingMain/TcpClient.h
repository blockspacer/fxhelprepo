#pragma once
#include <string>
#include <vector>
#include <memory>
#include <WinSock2.h>

// 提供TCP连接功能，发送数据，接收数据；
// 需要关联一个线程进去，运行收发数据
class Thread;
class SocketWrapper;

class TcpClient
    //:std::enable_shared_from_this<TcpClient>
{
public:
    TcpClient();
    ~TcpClient();

    bool Connect(const std::string& ip, unsigned short port);
    bool Send(const std::vector<char>& data);

    static DWORD Recv(LPVOID lpParam);
    bool DoRecv();
    bool HandleData(const std::vector<char>& data, int len);
private:
    std::unique_ptr<Thread> thread_;
    //std::unique_ptr<SocketWrapper> socketWrapper_;
    SOCKET socket_;
};

