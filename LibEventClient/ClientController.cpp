#include "stdafx.h"
#include "ClientController.h"

#include <signal.h>

#include "event2/event.h"
#include "event2/util.h"
#include "event2/thread.h"

namespace
{   
std::string data = R"(GET /9AoX9ROWdLBGpj3Uk2T1_Q==/1380986614829967.jpg?param=138y138 HTTP/1.1
Host: p1.music.126.net
Connection: keep-alive
Accept: image/webp,image/*,*/*;q=0.8
User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.110 Safari/537.36
Accept-Encoding: gzip, deflate
Accept-Language: en-US,en;q=0.8

)";

    struct echo_context
    {
        struct event_base *base;
        struct event *event_write;
        struct event *event_read;
    };
}

ClientController::ClientController()
{
}


ClientController::~ClientController()
{
}

bool ClientController::Initialize()
{
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);

    (void)WSAStartup(wVersionRequested, &wsaData);
    worker_thread_.reset(new std::thread(std::bind(&ClientController::WorkerFunction, this)));

    return true;
}

void ClientController::Finalize()
{
    exit_flag_ = true;
    event_active(hup_event_, EV_SIGNAL, 1);
    event_base_loopexit(event_base_, NULL);

    if (worker_thread_)
        worker_thread_->join();
}

bool ClientController::AddClient(const std::string&ip, uint16 port,
                                 ConnectCallback connect_cb, 
                                 DataReceiveCallback data_cb)
{
    SingleTcpClient* client = new SingleTcpClient;
    TCPHANDLE handle = client->Connect(event_base_, ip, port, connect_cb, data_cb);
    if (handle <= 0)
        return false;

    tcp_client_map_.erase(handle);
    tcp_client_map_[handle] = client;
    return true;
}

void ClientController::RemoveClient(TCPHANDLE handle)
{
    auto result = tcp_client_map_.find(handle);
    if (result != tcp_client_map_.end())
    {
        delete result->second;
        tcp_client_map_.erase(result);
    }
}

void ClientController::signal_cb(evutil_socket_t sock, short flags, void * args)
{

}

void ClientController::WorkerFunction()
{
    evthread_use_windows_threads();
    event_base_ = event_base_new();
    evthread_make_base_notifiable(event_base_);
    hup_event_ = evsignal_new(event_base_, SIGINT, signal_cb, NULL);
    event_add(hup_event_, NULL);
    event_base_dispatch(event_base_);
    event_base_free(event_base_);
}