#ifndef _TCP_LISTENER_H_
#define _TCP_LISTENER_H_

#include <WinSock2.h>
#include <windows.h>
#include <memory>
#include <vector>
#include <functional>
#include "event2/listener.h"
#include "event2/util.h"
#include "event2/bufferevent.h"
#include "help_libevent_export_def.h"

namespace HelpLibevent
{
class TcpClient;
class HELP_LIBEVENT_API TcpListener : public std::enable_shared_from_this<TcpListener>
{
public:
    typedef std::function<void (std::shared_ptr<TcpClient> client)>
        AcceptCallback;
    TcpListener(const char* addr, unsigned short port);
    ~TcpListener();
    void SetAcceptCallback(AcceptCallback acceptCb);
    void Start(std::function<void (bool success)> searchListenPortFinishedNotify);
    void Stop();
    std::weak_ptr<TcpListener> GetWeakPtr();
    void OnEventCallback(std::shared_ptr<TcpClient> client, short events);
    unsigned short GetPort();
private:
    static void OnAcceptCallabck(evconnlistener* listener,
        evutil_socket_t fd, sockaddr* addr, int socklen, void* arg);
    void OnAcceptCallabck(evconnlistener* listener,
        evutil_socket_t fd, sockaddr* addr, int socklen);
private:
    event_base* base_;
    evconnlistener* listener_;
    std::vector<std::shared_ptr<TcpClient>> clients_;
    AcceptCallback acceptCallback_;
    sockaddr_in addr_;
    bool canceled_;
};
}
#endif