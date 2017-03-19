#pragma once

#include <map>
#include <functional>

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "third_party/chromium/base/memory/scoped_ptr.h"
#include "third_party/chromium/base/threading/thread.h"

#include "event2/util.h"
#include "event2/bufferevent.h"
#include "event2/listener.h"

class AuthorityController;

class AuthorityNetwork
{
public:
    AuthorityNetwork();
    ~AuthorityNetwork();

    bool Initialize(AuthorityController* controller);
    void Finalize();

    void SetNotify(std::function<void(const std::wstring& message)> callback);

    bool Start();
    void Stop();

private:
    void DispatchFunction();

    bool SendDataToClient(bufferevent *bev, const std::vector<uint8>& data);
    static void listener_cb(evconnlistener *listener, evutil_socket_t fd,
                     struct sockaddr *sock, int socklen, void *arg);
    static void socket_read_cb(bufferevent *bev, void *arg);
    static void socket_write_cb(bufferevent *bev, void *arg);
    static void socket_event_cb(bufferevent *bev, short events, void *arg);
    static void signal_cb(intptr_t sock, short flags, void * args);

    struct event_base * event_base_ = 0;
    struct event* hup_event_ = 0;
    bool exit_flag_ = false;
    scoped_ptr<base::Thread> network_thread_;

    AuthorityController* authority_controller_;

    std::function<void(const std::wstring&)> notify_callback_;

    //std::map<int, bufferevent *> handle_bev_map_;
};

