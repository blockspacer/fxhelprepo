#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include <WinSock2.h>
#include <windows.h>
#include <functional>
#include <memory>

#include "event2\event.h"
#include "event2\bufferevent.h"
#include "event2\util.h"
#include "help_libevent_export_def.h"
namespace HelpLibevent
{
class TcpListener;
class HELP_LIBEVENT_API TcpClient : public std::enable_shared_from_this<TcpClient>
{
public:
    friend class TcpListener;
    typedef std::function<void (const char* buffer, const unsigned int length)> ReadCallback;
    typedef std::function<void (short events)> EventCallback;
    typedef std::function<void ()> WriteCallback;
    TcpClient(const char* addr, unsigned short port);
    TcpClient(bufferevent* bev, std::weak_ptr<TcpListener> listener);
    ~TcpClient();
public:
    bool Send(const void* data, const int& length);
    void SetCallback(EventCallback eventCb, ReadCallback readCb, WriteCallback writeCb);
    const bufferevent* GetBufferevent() const;
    std::weak_ptr<TcpClient> GetWeakPtr();
    const sockaddr_in GetAddr();
    int GetOutputBufferLength();
    int GetInputBufferLength();
private:
    void Close();
    static void OnReadCallback(bufferevent* bev, void* arg);
    static void OnEventCallback(bufferevent* bev, short events, void* arg);
    static void OnWrieteCallback(bufferevent* bev, void* arg);
    static unsigned __stdcall WorkFunction(void* arg);
private:
    bufferevent* buffevent_;
    sockaddr_in addr_;
    ReadCallback readCallback_;
    EventCallback eventCallback_;
    WriteCallback writeCallback_;
    event_base* base_;
    HANDLE thread_;
    bool connectError_;
    std::weak_ptr<TcpListener> listener_;
};
}
#endif