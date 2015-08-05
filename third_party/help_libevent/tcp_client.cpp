#include "tcp_client.h"
#include "tcp_listener.h"
#include "event2\thread.h"
#include "event2\buffer.h"
#include <process.h>
#include <memory>
namespace HelpLibevent
{
TcpClient::TcpClient(const char* addr, unsigned short port)
    : eventCallback_(nullptr)
    , readCallback_(nullptr)
    , writeCallback_(nullptr)
    , thread_(INVALID_HANDLE_VALUE)
    , listener_()
    , buffevent_(nullptr)
    , connectError_(false)
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(addr);

    evthread_use_windows_threads();
    base_ = event_base_new();

    buffevent_ = bufferevent_socket_new(base_, -1, 
        BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | 
        BEV_OPT_DEFER_CALLBACKS | BEV_OPT_UNLOCK_CALLBACKS);
    bufferevent_setcb(buffevent_, OnReadCallback, OnWrieteCallback, 
        OnEventCallback, this);
    int reuslt = bufferevent_enable(buffevent_, EV_READ|EV_WRITE);
    thread_ = (HANDLE)_beginthreadex(nullptr, 0, WorkFunction, this, 0, nullptr);
}

TcpClient::TcpClient(bufferevent* bev, std::weak_ptr<TcpListener> listener)
    : eventCallback_(nullptr)
    , readCallback_(nullptr)
    , thread_(INVALID_HANDLE_VALUE)
    , base_(nullptr)
    , buffevent_(nullptr)
    , listener_(listener)
    , connectError_(false)
{
    evutil_socket_t fd = bufferevent_getfd(bev);
    
    int length = sizeof(sockaddr_in);
    getpeername(fd, (sockaddr*)&addr_, &length);

    buffevent_ = bev;
    bufferevent_setcb(bev, OnReadCallback, OnWrieteCallback, 
        OnEventCallback, this);
    bufferevent_enable(buffevent_, EV_READ|EV_WRITE);
}

TcpClient::~TcpClient()
{
    Close();
}

bool TcpClient::Send(const void* data, const int& length)
{
    if (connectError_)
        return false;
    int result = bufferevent_write(buffevent_, data, length);
    return result == 0;
}

void TcpClient::OnReadCallback(bufferevent* bev, void* arg)
{
    TcpClient* self = static_cast<TcpClient*>(arg);
    evbuffer *input = bufferevent_get_input(self->buffevent_);
    size_t inputLength = evbuffer_get_length(input);
    std::unique_ptr<char> buffer(new char[inputLength+1]);
    memset(buffer.get(), 0, inputLength+1);
    evbuffer_remove(input, buffer.get(), inputLength);

    if (self && self->readCallback_)
    {
        self->readCallback_(buffer.get(), inputLength);
    }
}

void TcpClient::OnEventCallback(bufferevent* bev, short events, void* arg)
{
    if (events & BEV_EVENT_READING)
    {
        //cout << "OnEventCallback BEV_EVENT_READING" << endl;
    }
    else if (events & BEV_EVENT_WRITING)
    {
        //cout << "OnEventCallback BEV_EVENT_WRITING" << endl;
    }
    else if (events & BEV_EVENT_EOF)
    {
        //cout << "OnEventCallback BEV_EVENT_EOF" << endl;
    }
    else if (events & BEV_EVENT_ERROR)
    {
        //DWORD error = EVUTIL_SOCKET_ERROR();
        //cout << "OnEventCallback BEV_EVENT_ERROR" << endl;
    }
    else if (events & BEV_EVENT_TIMEOUT)
    {
        //cout << "OnEventCallback BEV_EVENT_TIMEOUT" << endl;
    }
    else if (events & BEV_EVENT_CONNECTED)
    {
        //cout << "OnEventCallback BEV_EVENT_CONNECTED" << endl;
    }
    TcpClient* self = static_cast<TcpClient*>(arg);
    if (self && self->eventCallback_)
    {
        self->eventCallback_(events);

        auto listener = self->listener_.lock();
        if (listener)
        {
            listener->OnEventCallback(self->shared_from_this(), events);
        }
    }
}

void TcpClient::OnWrieteCallback(bufferevent* bev, void* arg)
{
    //printf("OnWrieteCallback\n");
    TcpClient* self = static_cast<TcpClient*>(arg);
    if (self && self->writeCallback_)
    {
        self->writeCallback_();
    }
}

unsigned __stdcall TcpClient::WorkFunction(void* arg)
{
    TcpClient* self = static_cast<TcpClient*>(arg);
    if (self)
    {
        if (bufferevent_socket_connect(self->buffevent_, (sockaddr*)&self->addr_,
            sizeof(self->addr_)) < 0)
        {
            self->connectError_ = true;
            return -1;
        }
        event_base_dispatch(self->base_);
    }
    return 0;
}

void TcpClient::SetCallback(EventCallback eventCb, 
    ReadCallback readCb, WriteCallback writeCb)
{
    eventCallback_ = eventCb;
    readCallback_ = readCb;
    writeCallback_ = writeCb;
}

const bufferevent* TcpClient::GetBufferevent() const
{
    return buffevent_;
}

std::weak_ptr<TcpClient> TcpClient::GetWeakPtr()
{
    return shared_from_this();
}

const sockaddr_in TcpClient::GetAddr()
{
    return addr_;
}

void TcpClient::Close()
{
    if (thread_ != INVALID_HANDLE_VALUE)
    {
        while (WAIT_TIMEOUT == WaitForSingleObject(thread_, 10))
        {
            event_base_loopbreak(base_);
        }
        CloseHandle(thread_);
        thread_ = INVALID_HANDLE_VALUE;
    }
    if (buffevent_)
    {
        bufferevent_free(buffevent_);
        buffevent_ = nullptr;
    }
    if (base_)
    {
        event_base_free(base_);
        base_ = nullptr;
    }
}

int TcpClient::GetOutputBufferLength()
{
    if (buffevent_)
    {
        return evbuffer_get_length(bufferevent_get_output(buffevent_));
    }
    return -1;
}

int TcpClient::GetInputBufferLength()
{
    if (buffevent_)
    {
        return evbuffer_get_length(bufferevent_get_input(buffevent_));
    }
    return -1;
}

}