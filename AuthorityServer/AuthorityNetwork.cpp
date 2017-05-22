#include "stdafx.h"
#include "AuthorityNetwork.h"
#include "AuthorityController.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

#include<stdio.h>  
#include<string.h>  

#include "event2/event.h"
#include <signal.h>
#include "event2/thread.h"



AuthorityNetwork::AuthorityNetwork()
    :network_thread_(nullptr)
    , notify_callback_(nullptr)
{
}


AuthorityNetwork::~AuthorityNetwork()
{
}

bool AuthorityNetwork::Initialize(AuthorityController* controller)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    (void)WSAStartup(wVersionRequested, &wsaData);

    authority_controller_ = controller;
    authority_controller_->SetSendDataFunction(
        base::Bind(&AuthorityNetwork::SendDataToClient,
        base::Unretained(this)));

    network_thread_.reset(new base::Thread("network_thread"));
    return true;
}

void AuthorityNetwork::Finalize()
{
    if (network_thread_->IsRunning())
        Stop();
}

void AuthorityNetwork::SetNotify(
    std::function<void(const std::wstring& message)> callback)
{
    notify_callback_ = callback;
}

bool AuthorityNetwork::Start()
{
    network_thread_->Start();
    network_thread_->message_loop_proxy()->PostTask(
        FROM_HERE, base::Bind(&AuthorityNetwork::DispatchFunction,
        base::Unretained(this)));
    notify_callback_(L"启动业务");

    return true;
}

void AuthorityNetwork::Stop()
{
    exit_flag_ = true;
    event_active(hup_event_, EV_SIGNAL, 1);
    event_base_loopexit(event_base_, NULL);

    network_thread_->Stop();
    notify_callback_(L"停止业务");
}

void AuthorityNetwork::DispatchFunction()
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(9999);

    evthread_use_windows_threads();
    event_base_ = event_base_new();
    evthread_make_base_notifiable(event_base_);
    hup_event_ = evsignal_new(event_base_, SIGINT, signal_cb, this);
    event_add(hup_event_, NULL);

    notify_callback_(L"创建监听");
    evconnlistener *listener
        = evconnlistener_new_bind(event_base_, &AuthorityNetwork::listener_cb, this,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
        10, (struct sockaddr*)&sin,
        sizeof(struct sockaddr_in));

    notify_callback_(L"启动派发流程");
    event_base_dispatch(event_base_);

    evconnlistener_free(listener);
    event_base_free(event_base_);
    exit_flag_ = true;
    notify_callback_(L"退出监听流程");
    return ;
}

bool AuthorityNetwork::SendDataToClient(
    bufferevent *bev, const std::vector<uint8>& data)
{
    //char reply[] = "I has read your data";
    bufferevent_write(bev, (const void*)(&data[0]), data.size());
    return true;
}

//一个新客户端连接上服务器了  
//当此函数被调用时，libevent已经帮我们accept了这个客户端。该客户端的
//文件描述符为fd
void AuthorityNetwork::listener_cb(evconnlistener *listener, evutil_socket_t fd,
struct sockaddr *sock, int socklen, void *arg)
{
    LOG(INFO)<<"accept a client " << fd;

    AuthorityNetwork* network = reinterpret_cast<AuthorityNetwork*>(arg);
    event_base *base = network->event_base_;
    auto notify_callback = network->notify_callback_;
    std::wstring wstr = L"accept a client";
    notify_callback(wstr);


    //为这个客户端分配一个bufferevent  
    bufferevent *bev = bufferevent_socket_new(base, fd,
                                              BEV_OPT_CLOSE_ON_FREE);
    
    // 后续要加上对write_cb的处理
    bufferevent_setcb(bev, AuthorityNetwork::socket_read_cb, socket_write_cb,
                      AuthorityNetwork::socket_event_cb, (void*)(network));
    bufferevent_enable(bev, EV_READ | EV_PERSIST);

    // 记录连接端的信息
    network->authority_controller_->AddClient(bev, *sock);
}

void AuthorityNetwork::socket_read_cb(bufferevent *bev, void *arg)
{
    AuthorityNetwork* network = reinterpret_cast<AuthorityNetwork*>(arg);
    event_base *base = network->event_base_;
    auto notify_callback = network->notify_callback_;

    //char msg[4096];
    char msg[8];
    std::vector<uint8> data;

    size_t len = bufferevent_read(bev, msg, sizeof(msg) - 1);
    while (len)
    {
        data.insert(data.end(), msg, msg + len);
        len = bufferevent_read(bev, msg, sizeof(msg) - 1);
    }

    std::string str(data.begin(), data.end());
    std::wstring wstr = L"server read the data:";
    wstr += base::UTF8ToWide(str);
    notify_callback(wstr);

    // 扔给业务线程去处理客户发送过来的数据
    network->authority_controller_->HandleMessage(bev, data);
}

void AuthorityNetwork::socket_write_cb(bufferevent *bev, void *arg)
{
    AuthorityNetwork* network = reinterpret_cast<AuthorityNetwork*>(arg);
    event_base *base = network->event_base_;
    auto notify_callback = network->notify_callback_;

    std::wstring wstr = L"I has write your data";
    notify_callback(wstr);
}


void AuthorityNetwork::socket_event_cb(bufferevent *bev, short events, void *arg)
{
    AuthorityNetwork* network = reinterpret_cast<AuthorityNetwork*>(arg);
    event_base *base = network->event_base_;
    auto notify_callback = network->notify_callback_;
    auto authority_controller = network->authority_controller_;

    if (events & BEV_EVENT_EOF)
    {
        printf("connection closed\n");
        notify_callback(L"connection closed");
    }
    else if (events & BEV_EVENT_ERROR)
    {
        printf("some other error\n");
        notify_callback(L"some other error");
    }

    authority_controller->RemoveClient(bev);

    //这将自动close套接字和free读写缓冲区  
    bufferevent_free(bev);
}

void AuthorityNetwork::signal_cb(intptr_t sock, short flags, void * args)
{
    AuthorityNetwork* network = reinterpret_cast<AuthorityNetwork*>(args);
    event_base *base = network->event_base_;
    auto notify_callback = network->notify_callback_;

    notify_callback(L"signal_cb call!");
}
