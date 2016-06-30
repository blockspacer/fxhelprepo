#pragma once
#include <memory>
#include <vector>
#include "third_party/chromium/base/basictypes.h"
#include "IpProxy.h"
#include "TcpClient.h"

typedef SOCKET SocketHandle;
class SocketWrapper
{
public:
    SocketWrapper();
    ~SocketWrapper();

    bool Create();
    SocketHandle GetSocketHandle() const;
    bool Connect(const std::string& ip, unsigned short port);
    bool Send(const std::vector<char>& data);
    bool Recv(std::vector<char>* buffer);
    void Close();

private:
    SocketHandle socket_;
};

class TcpProxyClient
{
public:
    TcpProxyClient();
    ~TcpProxyClient();

    void SetProxy(const IpProxy& ipproxy);
    bool Connect(const std::string& ip, uint16 port);
    SocketHandle GetSocketHandle() const;
    bool Send(const std::vector<uint8>& data);
    bool Recv(std::vector<uint8>* buffer);
    void Close();

private:
    bool ConnectToSocks4Proxy(const std::string& ip, uint16 port);
    bool ConnectToSocks5Proxy(const std::string& ip, uint16 port);
    bool ConnectToServer(const std::string& ip, uint16 port);

    SocketWrapper socketWrapper_;
    //std::string proxyIp_;
    //uint16 proxyPort_;
    IpProxy ipproxy_;
};

