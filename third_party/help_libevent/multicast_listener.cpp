#include "multicast_listener.h"
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <process.h>
#include "event2/thread.h"
#include "event2/event-config.h"
#include "event-internal.h"
#include "evthread-internal.h"
namespace HelpLibevent
{

MulticastListener::MulticastListener(unsigned short port,
    const char* maddr)
    : listener_(nullptr)
    , base_(nullptr)
    , acceptCallback_(nullptr)
    , canceled_(false)
    , socket_(0)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr("0.0.0.0");

    multicastAddr_.sin_family = AF_INET;
    multicastAddr_.sin_port = 0;
    multicastAddr_.sin_addr.s_addr = inet_addr(maddr);
    
}

MulticastListener::~MulticastListener()
{
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

void MulticastListener::SetAcceptCallback(AcceptCallback acceptCb)
{
    acceptCallback_ = acceptCb;
}

void MulticastListener::OnAcceptCallabck(evconnlistener* listener, 
    int len, char* buf, void* arg)
{
    MulticastListener* self = static_cast<MulticastListener*>(arg);
    if (self)
    {
        self->OnAcceptCallabck(listener, len, buf);
    }
}

void MulticastListener::OnAcceptCallabck(evconnlistener* listener, 
    int len, char* buf)
{
    std::string str = buf;
    if (acceptCallback_)
    {
        acceptCallback_(str);
    }
}


void MulticastListener::Start(
    std::function<void (bool success)> searchListenPortFinishedNotify,
    const std::vector<sockaddr_in>& serverAddrList)
{
    canceled_ = false;
    evthread_use_windows_threads();
    //event_set_log_callback(EventLogCallback);
    base_ = event_base_new();
    if (base_)
    {
        unsigned short port = ntohs(addr_.sin_port);
        for (unsigned short n = 0; n < 5; n++)
        {
            listener_ = MulticastListenerNew(base_, OnAcceptCallabck, this, 
                LEV_OPT_CLOSE_ON_FREE|LEV_OPT_LEAVE_SOCKETS_BLOCKING|
                LEV_OPT_CLOSE_ON_EXEC|LEV_OPT_REUSEABLE,
                serverAddrList, &multicastAddr_);
            if (listener_ != nullptr)
                break;
            addr_.sin_port = htons(++port);
            Sleep(50);
            if (canceled_)
                return;
        }
        if (nullptr == listener_)
            return;
        
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

void MulticastListener::Stop(const std::vector<sockaddr_in>& ips)
{
    canceled_ = true;
    if (socket_)
    {
        for (auto ip = ips.begin(); ip != ips.end(); ip++)
        {
            struct ip_mreq mreq;
            mreq.imr_interface = ip->sin_addr;
            mreq.imr_multiaddr = multicastAddr_.sin_addr;
            setsockopt(socket_, IPPROTO_IP, 
                IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq)); 
        }
    }
    socket_ = 0;
    if (base_)
    {
        event_base_loopbreak(base_);
    }
}

std::weak_ptr<MulticastListener> MulticastListener::GetWeakPtr()
{
    return shared_from_this();
}


unsigned short MulticastListener::GetPort()
{
    return ntohs(addr_.sin_port);
}

struct evconnlistener * MulticastListener::MulticastListenerNew(
    struct event_base *base, udp_evconnlistener_cb cb, void *ptr, 
    unsigned flags, const std::vector<sockaddr_in>& serverAddrList, 
    sockaddr_in* multicastAddr)
{
    struct evconnlistener *listener;
    evutil_socket_t sock_fd;
    int flag = 1;

    /* Create endpoint */
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        return NULL;
    }

    if (evutil_make_socket_nonblocking(sock_fd) < 0) 
    {
        evutil_closesocket(sock_fd);
        return NULL;
    }

    if (flags & LEV_OPT_CLOSE_ON_EXEC) 
    {
        if (evutil_make_socket_closeonexec(sock_fd) < 0) 
        {
            evutil_closesocket(sock_fd);
            return NULL;
        }
    }

    if (flags & LEV_OPT_REUSEABLE) 
    {
//         if (evutil_make_listen_socket_reuseable(sock_fd) < 0) 
//         {
//             evutil_closesocket(sock_fd);
//             return NULL;
//         }
        int one = 1;
        setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &one,
            (ev_socklen_t)sizeof(one));
    }

    /* Bind */
    if (bind(sock_fd, (struct sockaddr *)&addr_, sizeof(addr_)) < 0) 
    {
        evutil_closesocket(sock_fd);
        return NULL;
    } 

    /* Set multicast */
    flag = 0;
    if (setsockopt(sock_fd, IPPROTO_IP, 
        IP_MULTICAST_LOOP, (char*)&flag, sizeof(flag)) < 0) 
    {
        evutil_closesocket(sock_fd);
        return NULL;
    }

    for (auto ip = serverAddrList.begin(); ip != serverAddrList.end(); ip++)
    {
        struct ip_mreq mreq;
        mreq.imr_interface = ip->sin_addr;
        mreq.imr_multiaddr = multicastAddr->sin_addr;
        if (setsockopt(sock_fd, IPPROTO_IP, 
            IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) 
        {
            evutil_closesocket(sock_fd);
            return NULL;
        }
    }

    listener = multicast_listener_new(base, sock_fd, cb, ptr, flags);
    if (!listener) {
        evutil_closesocket(sock_fd);
        return NULL;
    }

    socket_ = sock_fd;

    return listener;

}

}