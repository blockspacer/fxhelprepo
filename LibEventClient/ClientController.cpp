#include "stdafx.h"
#include "ClientController.h"

#include <signal.h>

#include "event2/event.h"
#include "event2/util.h"
#include "event2/thread.h"

namespace
{   
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

bool ClientController::Send(TCPHANDLE handle, const std::vector<uint8>& data, 
                            SendDataCallback callback)
{
    auto result = tcp_client_map_.find(handle);
    if (result == tcp_client_map_.end())
        return false;

    bool send_result =  result->second->Send(data);
    callback(send_result);
    return send_result;
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