#include "tcp_listener.h"
#include <process.h>
#include "event2/thread.h"
#include "tcp_client.h"

namespace HelpLibevent
{


TcpListener::TcpListener(const char* addr, unsigned short port)
    : listener_(nullptr)
    , base_(nullptr)
    , acceptCallback_(nullptr)
    , canceled_(false)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(addr);
}

TcpListener::~TcpListener()
{
    if (clients_.size() > 0)
    {
        clients_.clear();
    }
    if (listener_)
    {
        evconnlistener_free(listener_);
        listener_ = nullptr;
    }
    
    if (base_)
    {
        event_base_free(base_);
        base_ = nullptr;
    }
}

void TcpListener::SetAcceptCallback(AcceptCallback acceptCb)
{
    acceptCallback_ = acceptCb;
}

void TcpListener::OnAcceptCallabck(evconnlistener* listener,
    evutil_socket_t fd, sockaddr* addr, int socklen, void* arg)
{
    TcpListener* self = static_cast<TcpListener*>(arg);
    if (self)
    {
        self->OnAcceptCallabck(listener, fd, addr, socklen);
    }
}

void TcpListener::OnAcceptCallabck( evconnlistener* listener,
    evutil_socket_t fd, sockaddr* addr, int socklen )
{
    struct event_base *base = evconnlistener_get_base(listener_);
    struct bufferevent *bev = bufferevent_socket_new(base, fd,
        BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | 
        BEV_OPT_DEFER_CALLBACKS | BEV_OPT_UNLOCK_CALLBACKS);

    std::shared_ptr<TcpClient> client(new TcpClient(bev, GetWeakPtr()));

    clients_.push_back(client);
    if (acceptCallback_)
    {
        acceptCallback_(client);
    }
    
}

void EventLogCallback(int severity, const char *msg)
{
    OutputDebugStringA(msg);
}

void TcpListener::Start(
    std::function<void (bool success)> searchListenPortFinishedNotify)
{
    canceled_ = false;
    evthread_use_windows_threads();
    event_set_log_callback(EventLogCallback);
    base_ = event_base_new();
    if (base_)
    {
        unsigned short port = ntohs(addr_.sin_port);
        while ((listener_ = evconnlistener_new_bind(base_, OnAcceptCallabck,
            this, LEV_OPT_CLOSE_ON_FREE | 
            LEV_OPT_LEAVE_SOCKETS_BLOCKING | LEV_OPT_CLOSE_ON_EXEC,
            -1, (sockaddr*)&addr_, sizeof(addr_))) == nullptr)
        {
            addr_.sin_port = htons(++port);
            Sleep(50);
            if (canceled_)
                return;
        }
        if (searchListenPortFinishedNotify)
            searchListenPortFinishedNotify(true);
        if (canceled_)
            return;
        event_base_dispatch(base_);
    }
    else 
    {
        if (searchListenPortFinishedNotify)
            searchListenPortFinishedNotify(false);
    }
}

void TcpListener::Stop()
{
    canceled_ = true;
    if (base_)
        event_base_loopbreak(base_);
}

void TcpListener::OnEventCallback(std::shared_ptr<TcpClient> client,
                                  short events)
{
    if (events & (BEV_EVENT_READING|BEV_EVENT_WRITING|BEV_EVENT_EOF|
        BEV_EVENT_ERROR))
    {
        for (auto iter = clients_.begin(); iter != clients_.end(); ++iter)
        {
            if (iter->get() == client.get())
            {
                clients_.erase(iter);
                break;
            }
        }
    }
}

std::weak_ptr<TcpListener> TcpListener::GetWeakPtr()
{
    return shared_from_this();
}


unsigned short TcpListener::GetPort()
{
    return ntohs(addr_.sin_port);
}

}