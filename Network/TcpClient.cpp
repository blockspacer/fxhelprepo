#include <assert.h>
#include "TcpClient.h"
#include "EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

TcpClient::TcpClient()
    : socket_(0)
{
}

TcpClient::~TcpClient()
{
}

bool TcpClient::Initialize()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    return true;
}

void TcpClient::Finalize()
{
    if (socket_)
    {
        closesocket(socket_);
        socket_ = 0;
    }
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

    struct linger lig;
    lig.l_onoff = 0;
    lig.l_linger = 0;
    int iLen = sizeof(struct linger);
    setsockopt(socket_, SOL_SOCKET, SO_LINGER, (char *)&lig, iLen);
    int flag = 1;
    int len = sizeof(int);
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, len);
    int keepalivetime = 10;
    setsockopt(socket_, IPPROTO_TCP, SO_KEEPALIVE,
        (const char FAR *)&keepalivetime, sizeof(keepalivetime));
    rc = connect(socket_, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (SOCKET_ERROR == rc)
    {
        int error = WSAGetLastError();
        return false;
    }
    
    return true;
}

bool TcpClient::Recv(std::vector<char>* buffer)
{
    std::vector<char> temp(4096);
    int len = 0;
    fd_set rfdset;
    FD_ZERO(&rfdset);
    FD_SET(socket_, &rfdset);
    timeval timeout = { 0, 1000 };
    int ret = select(0, &rfdset, 0, 0, &timeout);
    if (ret == 0)//timeout
    {
        return true;
    }
    else if (ret < 0)
    {
        return false;
    }

    if (FD_ISSET(socket_, &rfdset))
    {
        len = recv(socket_, &temp[0], temp.size(), 0);
        if (SOCKET_ERROR == len)
        {
            return false;
        }
        else if (len>0)
        {
            //HandleData(*buffer, len);
            buffer->assign(temp.begin(), temp.begin()+len);
            return true;
        }
    }
    return false;
}

bool TcpClient::Send(const std::vector<char>& data)
{
    int len = send(socket_, data.data(), data.size(), 0);
    return len==data.size();
}