#include <assert.h>
#include "TcpClient.h"
#include "Thread.h"
#include "EncodeHelper.h"

TcpClient::TcpClient()
    : socket_(0)
    , notify_(nullptr)
    , privateData_(nullptr)
{
    thread_.reset(new Thread);
    thread_->Init(TcpClient::Recv, this);
}

TcpClient::~TcpClient()
{  
    thread_->Stop();
    notify_ = nullptr;
    privateData_ = nullptr;
    closesocket(socket_);
}

void TcpClient::SetNotify(NotifyFunction notify, void* privatedata)
{
    notify_ = notify;
    privateData_ = privatedata;
}

bool TcpClient::Connect(const std::string& ip, unsigned short port)
{
    int rc = 0;
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == INVALID_SOCKET)
    {
        assert(false);
        return false;
    }

    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    int keepalivetime = 10;
    setsockopt(socket_, IPPROTO_TCP, SO_KEEPALIVE,
        (const char FAR *)&keepalivetime, sizeof(keepalivetime));
    rc = connect(socket_, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (SOCKET_ERROR == rc)
    {
        assert(false);
        return false;
    }
    
    thread_->Start();

    return true;
}

DWORD TcpClient::Recv(LPVOID lpParam)
{
    TcpClient* tcpClient = (TcpClient*)(lpParam);
    return tcpClient->DoRecv();
}

// 这里简单的以阻塞模式解决问题
bool TcpClient::DoRecv()
{
    std::vector<char> buffer(4096);

    int len = 0;
    while (1)
    {
        len = recv(socket_, &buffer[0], buffer.size(), 0);
        if (len>0)
        {
            HandleData(buffer, len);
        }
    }

    return true;
}

bool TcpClient::HandleData(const std::vector<char>& data, int len)
{
    //std::string strdata = BinToAnsiHex(&data[0], len);
    //printf(strdata.c_str());
    if (notify_ && privateData_)
    {
        std::vector<char> outdata;
        outdata.assign(&data[0], &data[0] + len);
        notify_(privateData_, outdata);
    }
    return true;
}

bool TcpClient::Send(const std::vector<char>& data)
{
    int len = send(socket_, data.data(), data.size(), 0);
    return len==data.size();
}