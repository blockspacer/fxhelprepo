#ifndef _MULTICAST_LISTENER_H_
#define _MULTICAST_LISTENER_H_
#include <WinSock2.h>
#include <windows.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "event2/listener.h"
#include "event2/util.h"
#include "event2/bufferevent.h"
#include "event2/event_struct.h"
#include "help_libevent_export_def.h"
namespace HelpLibevent
{
    
class TcpClient;
class HELP_LIBEVENT_API MulticastListener : public std::enable_shared_from_this<MulticastListener>
{
public:
    typedef std::function<void (std::string str)> AcceptCallback;
    MulticastListener(unsigned short port, const char* maddr);
    ~MulticastListener();
    void SetAcceptCallback(AcceptCallback acceptCb);
    void Start(std::function<void (bool success)> searchListenPortFinishedNotify
        , const std::vector<sockaddr_in>& serverAddrList);
    void Stop(const std::vector<sockaddr_in>& ips);
    std::weak_ptr<MulticastListener> GetWeakPtr();
    //void OnEventCallback(std::shared_ptr<TcpClient> client, short events);
    unsigned short GetPort();
private:
    static void OnAcceptCallabck(evconnlistener* listener,
        int len, char* buf, void* arg);
    void OnAcceptCallabck(evconnlistener* listener,
        int len, char* buf);
    struct evconnlistener * MulticastListenerNew(
    struct event_base *base, udp_evconnlistener_cb cb, void *ptr, 
        unsigned flags, const std::vector<sockaddr_in>& serverAddrList, 
        sockaddr_in* multicastAddr);
private:
    event_base* base_;
    evconnlistener* listener_;
    evutil_socket_t socket_;
    AcceptCallback acceptCallback_;
    sockaddr_in addr_;
    sockaddr_in multicastAddr_;
    bool canceled_;
};
}
#endif //_MULTICAST_LISTENER_H_