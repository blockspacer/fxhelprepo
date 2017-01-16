#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <WinSock.h>

#include "IpProxy.h"
//#include "TcpClient.h"
#include "TcpProxyClient.h"

#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/timer/timer.h"
#include "third_party/chromium/base/threading/thread.h"

class TcpManager
{
public:

    typedef std::function<void(bool, SocketHandle)> AddClientCallback;
    typedef std::function<void(bool, const std::vector<uint8>&)> ClientCallback;
    typedef std::function<void(bool)> SendDataCallback;

    TcpManager();
    ~TcpManager();

    static void AddRef() {}
    static void Release() {}

    bool Initialize();
    void Finalize();

    bool AddClient(AddClientCallback addcallback, const IpProxy& ipproxy,
        const std::string& ip, uint16 port, ClientCallback callback);
    void RemoveClient(SocketHandle handle);
    bool Send(SocketHandle handle, const std::vector<char>& data, SendDataCallback callback);
private:

    void DoAddClient(std::shared_ptr<TcpProxyClient> client, 
        AddClientCallback addcallback, ClientCallback callback);
    void DoRemoveClient(SocketHandle handle);
    void DoSend(SocketHandle handle, const std::vector<char>& data, SendDataCallback callback);
    void DoRecv();
    void DoRemoveAllClient();
    std::map<SocketHandle, ClientCallback> callbacks_;

    std::map<SocketHandle, std::pair<std::shared_ptr<TcpProxyClient>,ClientCallback>> newcallbacks_;

    bool stopflag = false;
    base::Thread baseThread_;
};

//
//SocketHandle g_handle;
//void tcpclientcallback(TcpManager* tcpmanager, bool result, SocketHandle handle)
//{
//    if (!result)
//        return;
//    g_handle = handle;
//    std::string str = "<policy-file-request/>";
//    std::vector<char> data;
//    data.assign(str.begin(), str.end());
//    data.push_back(0);
//    tcpmanager->Send(handle, data);
//}
//
//void clientcallback(bool result, const std::vector<char>& data)
//{
//    std::string str(data.begin(), data.end());
//    std::cout << str;
//}
//
//bool TcpManagerTest()
//{
//    TcpManager tcpmanager;
//    tcpmanager.Initialize();
//
//    tcpmanager.AddClient(
//        std::bind(tcpclientcallback, &tcpmanager, std::placeholders::_1, std::placeholders::_2),
//        "114.54.2.204", 843, clientcallback);
//
//    Sleep(10000);
//    tcpmanager.RemoveClient(g_handle);
//    while (1);
//}


