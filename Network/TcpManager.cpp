#include "TcpManager.h"
#include <set>
#include <assert.h>

#include "EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

TcpManager::TcpManager()
    :baseThread_("TcpManagerThread")
{

}
TcpManager::~TcpManager()
{

}

bool TcpManager::Initialize()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    return baseThread_.Start();
}

void TcpManager::Finalize()
{
    stopflag = true;

    if (baseThread_.IsRunning())
    {
        baseThread_.message_loop_proxy()->PostTask(
            FROM_HERE, base::Bind(&TcpManager::DoRemoveAllClient, this));
        baseThread_.Stop();
    }
}

bool TcpManager::AddClient(AddClientCallback addcallback, 
    const IpProxy& ipproxy, const std::string& ip, uint16 port,
    ClientCallback callback)
{
    return baseThread_.message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&TcpManager::DoAddClient, this, addcallback, ipproxy,
                   ip, port, callback));
}

void TcpManager::RemoveClient(TcpHandle handle)
{
    baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&TcpManager::DoRemoveClient, this, handle));
}

bool TcpManager::Send(TcpHandle handle, const std::vector<char>& data)
{
    return baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&TcpManager::DoSend, this, handle, data));
}

void TcpManager::DoAddClient(AddClientCallback addcallback, const IpProxy& ipproxy,
    const std::string& ip, uint16 port, ClientCallback callback)
{

    std::shared_ptr<TcpProxyClient> client(new TcpProxyClient);
    client->SetProxy(ipproxy);
    
    if (!client->Connect(ip, port))
    {
        addcallback(false, 0);
        return;
    }
    
    auto sock = client->GetSocketHandle();
    newcallbacks_[sock] = std::make_pair(client,callback);
    addcallback(true, sock);

    baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
                                               base::Bind(&TcpManager::DoRecv, this));

    //int rc = 0;
    //SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //if (sock == INVALID_SOCKET)
    //{
    //    assert(false);
    //    addcallback(false, sock);
    //    return;
    //}

    //SOCKADDR_IN serverAddr;
    //serverAddr.sin_family = AF_INET;
    //serverAddr.sin_port = htons(port);
    //serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    //struct linger lig;
    //lig.l_onoff = 0;
    //lig.l_linger = 0;
    //int iLen = sizeof(struct linger);
    //setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&lig, iLen);
    //int flag = 1;
    //int len = sizeof(int);
    //setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, len);
    //int keepalivetime = 10;
    //setsockopt(sock, IPPROTO_TCP, SO_KEEPALIVE,
    //    (const char FAR *)&keepalivetime, sizeof(keepalivetime));
    //rc = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    //if (SOCKET_ERROR == rc)
    //{
    //    int error = WSAGetLastError();
    //    addcallback(false, sock);
    //    return;
    //}
    //callbacks_[sock] = callback;
    //addcallback(true, sock);
    //baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
    //                                           base::Bind(&TcpManager::DoRecv, this));
}

void TcpManager::DoRemoveClient(TcpHandle handle)
{
    auto result = newcallbacks_.find(handle);
    if (result == newcallbacks_.end())
        return;

    closesocket(result->first);
    newcallbacks_.erase(result);
}

void TcpManager::DoSend(TcpHandle handle, const std::vector<char>& data)
{
    int len = send(handle, data.data(), data.size(), 0);
    if (len<0)
    {
        int errorcode = WSAGetLastError();
        LOG(ERROR) << base::IntToString(errorcode);
    }
    
    assert(len == data.size());
}

void TcpManager::DoRecv()
{
    do 
    {
        int len = 0;
        fd_set rfdset;
        FD_ZERO(&rfdset);
        for (const auto& newcallback : newcallbacks_)
        {
            SOCKET sock = newcallback.first;
            FD_SET(sock, &rfdset);
        }

        timeval timeout = { 0, 1 };
        int ret = select(0, &rfdset, 0, 0, &timeout);
        if (ret == 0)//timeout
        {
            break;;
        }
        else if (ret == -1)
        {
            int errorcode = WSAGetLastError();
            LOG(ERROR) << base::IntToString(errorcode);
            break;
        }

        std::set<SocketHandle> eraseset;
        for (const auto& callback : newcallbacks_)
        {
            std::vector<uint8> buffer;
            SOCKET sock = callback.first;
            if (FD_ISSET(sock, &rfdset))
            {
                if (!callback.second.first->Recv(&buffer))
                {
                    int errorcode = WSAGetLastError();
                    LOG(ERROR) << base::IntToString(errorcode);
                    callback.second.second(false, buffer);
                    eraseset.insert(callback.first);
                    continue;
                }
                else if (buffer.size() > 0)
                {
                    callback.second.second(true, buffer);
                }
            }
        }
        for (auto eraseit : eraseset)
        {
            newcallbacks_.erase(eraseit);
        }
    }
    while (0);

    if (stopflag)
        return;

    //if (callbacks_.empty()) // 没有任务的情况下不要轮询了
    //    return;

    baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
                                               base::Bind(&TcpManager::DoRecv, this));
}

void TcpManager::DoRemoveAllClient()
{
    newcallbacks_.clear();
}