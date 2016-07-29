#pragma once
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include "third_party/chromium/base/basictypes.h"
#include "SingleTcpClient.h"

#define ECHO_PORT   80
#define ECHO_SERVER "183.6.245.177"

class ClientController
{
public:
    ClientController();
    ~ClientController();

    bool Initialize();
    void Finalize();

    TCPHANDLE AddClient(const std::string&ip, uint16 port,
                   ConnectCallback connect_cb, DataReceiveCallback data_cb);
    void RemoveClient(TCPHANDLE handle);

private:
    void WorkerFunction();
    std::unique_ptr<std::thread> worker_thread_;
    bool exit_flag_ = false;

    struct event_base * event_base_ = 0;

    std::map<TCPHANDLE, SingleTcpClient*> tcp_client_map_;
};
