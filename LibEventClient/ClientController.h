#pragma once
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include "third_party/chromium/base/basictypes.h"
#include "SingleTcpClient.h"
#include "third_party/chromium/base/atomic_sequence_num.h"

#define ECHO_PORT   80
#define ECHO_SERVER "58.63.236.248"

class ClientController
{
public:
    ClientController();
    ~ClientController();

    bool Initialize();
    void Finalize();

    bool AddClient(const std::string&ip, uint16 port,
                   ConnectCallback connect_cb, DataReceiveCallback data_cb);
    void RemoveClient(TCPHANDLE handle);

    bool Send(TCPHANDLE handle, const std::vector<uint8>& data, SendDataCallback callback);

private:
    static void signal_cb(evutil_socket_t sock, short flags, void * args);

    void WorkerFunction();
    std::unique_ptr<std::thread> worker_thread_;
    bool exit_flag_ = false;

    struct event_base * event_base_ = 0;
    struct event* hup_event_ = 0;

    std::map<TCPHANDLE, SingleTcpClient*> tcp_client_map_;
    base::AtomicSequenceNumber sequencce_number_;
};

