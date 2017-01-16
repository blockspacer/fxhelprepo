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

TCPHANDLE SingleTcpClient::Connect(struct event_base * base,
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
    ev_read_ = event_new(base_, sock_, EV_READ, SingleTcpClient::read_cb, (void*)this);
    event_add(ev_write_, &tv);
    return sock_;
}

bool SingleTcpClient::Send(const std::vector<uint8>& data)
{
    int len = send(sock_, (char*)data.data(), data.size(), 0);
    return false;
}

void SingleTcpClient::Close()
{
    return;
}

// 通知可以进行数据发送
void SingleTcpClient::write_cb(evutil_socket_t sock, short flags, void * args)
{
    SingleTcpClient *client = (SingleTcpClient*)args;
    //int ret = send(sock, data.data(), data.length(), 0);
    bool result = true;
    if (EV_TIMEOUT & flags)
        result = false;

    if (client->connect_callback_)
    {
        client->connect_callback_(result);
    }
    event_add(client->ev_read_, 0);
}

void SingleTcpClient::read_cb(evutil_socket_t sock, short flags, void * args)
{
    SingleTcpClient *client = (SingleTcpClient*)args;
    char buf[128 + 1];
    int ret = recv(sock, buf, 128, 0);

    printf("read_cb, read %d bytes\n", ret);
    if (ret > 0)
    {
        buf[ret] = 0;
        printf("recv:%s\n", buf);
    }
    else if (ret == 0)
    {
        printf("read_cb connection closed\n");
        //event_base_loopexit(ec->base, NULL);
        return;
    }

    event_add(client->ev_read_, 0);
}
