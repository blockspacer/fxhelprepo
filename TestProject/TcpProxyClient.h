#pragma once
#include <memory>
#include <vector>
#include "third_party/chromium/base/basictypes.h"
class TcpClient;
class TcpProxyClient
{
public:
    TcpProxyClient();
    ~TcpProxyClient();

    void SetProxy(const std::string& ip, uint16 port);
    bool ConnectToProxy(const std::string& ip, uint16 port);
    bool Send(const std::vector<uint8>& data);
    bool Recv(std::vector<uint8>* buffer);

private:
    std::unique_ptr<TcpClient> tcpClient_;
    std::string proxyIp_;
    uint16 proxyPort_;
};

