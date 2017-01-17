#include "stdafx.h"
#include "SingleTcpClient.h"
#include "assert.h"
#include <signal.h>

#include "event2/event.h"
#include "event2/util.h"
#include "event2/thread.h"

namespace
{
static evutil_socket_t make_tcp_socket()
{
    int on = 1;
    evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    assert(sock != INVALID_SOCKET);

    evutil_make_socket_nonblocking(sock);
#ifdef WIN32
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&on, sizeof(on));
#else
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));
#endif

    return sock;
}

}
SingleTcpClient::SingleTcpClient()
{
}


SingleTcpClient::~SingleTcpClient()
{
}

SocketHandle SingleTcpClient::Connect(struct event_base * base,
                              const std::string&ip, uint16 port,
                              ConnectCallback connect_cb, DataReceiveCallback data_cb)
{
    base_ = base;
    connect_callback_ = connect_cb;
    data_receive_callback_ = data_cb;
    sock_ = make_tcp_socket();
    SOCKADDR_IN serverAddr;
    struct event * ev_write = 0;
    struct event * ev_read = 0;
    struct timeval tv = { 10, 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    memset(serverAddr.sin_zero, 0x00, 8);
    connect(sock_, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    ev_write_ = event_new(base_, sock_, EV_WRITE, SingleTcpClient::write_cb, (void*)this);
    ev_read_ = event_new(base_, sock_, EV_READ | EV_PERSIST, SingleTcpClient::read_cb, (void*)this);
    event_add(ev_write_, &tv);
    //event_add(ev_read_, &tv);
    return sock_;
}

bool SingleTcpClient::Send(const std::vector<uint8>& data)
{
    int len = send(sock_, (char*)data.data(), data.size(), 0);
    if (len < 0)
    {
        int errorcode = WSAGetLastError();
    }
    
    return (len == data.size());
}

void SingleTcpClient::Close()
{
    closesocket(sock_);
    event_del(ev_read_);
    return;
}

// 通知可以进行数据发送
void SingleTcpClient::write_cb(intptr_t sock, short flags, void * args)
{
    SingleTcpClient *client = (SingleTcpClient*)args;
    client->OnConnect(sock, flags);
}

void SingleTcpClient::read_cb(intptr_t sock, short flags, void * args)
{
    SingleTcpClient *client = (SingleTcpClient*)args;
    client->OnReceive(sock, flags);
}

void SingleTcpClient::OnConnect(intptr_t sock, short flags)
{
    bool result = true;
    if (flags == EV_WRITE)
    {
        int err;
        int len = sizeof(err);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, (char FAR *)(&err), &len);
        if (err)
        {
            printf("connect return ev_write, but check failed\n");
            this->Close();
            result = false;
        }
        else
        {
            result = true;
            event_add(ev_read_, 0);
            printf("connect return ev_write, check ok\n");
        }
    }
    else
    {  // timeout  
        printf("connect return failed, for timeout\n");
        this->Close();
        result = false;
    }

    connect_callback_(result, sock);
}

void SingleTcpClient::OnReceive(intptr_t sock, short flags)
{
    char buf[4096];
    int ret = recv(sock, buf, 4096, 0);
    std::vector<uint8> data;

    printf("read_cb, read %d bytes\n", ret);

    if (ret == 0)
    {
        printf("read_cb connection closed\n");
        this->Close();
        event_del(ev_read_);
        data_receive_callback_(false, data);
        return;
    }
    else if (ret < 0)
    {
        printf("read_cb connection error\n");
        this->Close();
        event_del(ev_read_);
        data_receive_callback_(false, data);
        return;
    }

    // 正常处理数据
    printf("recv data");
    data.assign(buf, buf+ret);
    data_receive_callback_(true, data);
}
