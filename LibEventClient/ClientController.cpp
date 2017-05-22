#include "stdafx.h"
#include "ClientController.h"

#include <signal.h>

#include "event2/event.h"
#include "event2/util.h"
#include "event2/thread.h"

#include "MessageDefine.h"
#include "MessagePackage.h"

#undef max
#undef min
#include "third_party/chromium/base/time/time.h"

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

    Header header;
    header.protocol_version = 1;
    header.header_length = sizeof(Header);
    header.total_length = sizeof(Header) + data.size();
    header.sequence_number = sequencce_number_.GetNext();
    header.ack_number = 0;
    header.timestamp = static_cast<uint32>(base::Time::Now().ToDoubleT());
    header.header_crc32 = 0; // ÔÝÊ±²»¼ì²é

    std::vector<uint8> send_package;
    
    uint8* header_stream = new uint8[sizeof(Header)];
    uint8* temp_addr = header_stream;
    if (!header.Write(&temp_addr, sizeof(Header)))
        return false;

    send_package.assign(header_stream, header_stream + sizeof(Header));
    send_package.insert(send_package.end(), data.begin(), data.end());
    delete[] header_stream;

    bool send_result = result->second->Send(send_package);
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