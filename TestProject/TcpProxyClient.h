#pragma once
#include <memory>
#include <vector>
#include "third_party/chromium/base/basictypes.h"
class TcpClient;
class MyTcpProxyClient
{
public:
    MyTcpProxyClient();
    ~MyTcpProxyClient();

    void SetProxy(const std::string& ip, uint16 port);
    bool ConnectToSocks4Proxy(const std::string& ip, uint16 port);
    bool ConnectToSocks5Proxy(const std::string& ip, uint16 port);
    bool Send(const std::vector<uint8>& data);
    bool Recv(std::vector<uint8>* buffer);

private:
    std::unique_ptr<TcpClient> tcpClient_;
    std::string proxyIp_;
    uint16 proxyPort_;
};
//
//void TcpProxyClientTest()
//{
//    MyTcpProxyClient proxyClient;
//    proxyClient.SetProxy("61.177.248.202", 1080);
//    proxyClient.ConnectToSocks4Proxy("114.54.2.205", 843);
//}

