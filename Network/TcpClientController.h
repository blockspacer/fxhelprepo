#pragma once
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include "third_party/chromium/base/basictypes.h"
#include "SingleTcpClient.h"
#include "Network/SingleTcpClient.h"
#include "Network/IpProxy.h"

#define ECHO_PORT   80
#define ECHO_SERVER "58.63.236.248"

class ClientController
{
public:
    typedef std::function<void(bool, SocketHandle)> AddClientCallback;
    typedef std::function<void(bool, const std::vector<uint8>&)> ClientCallback;
    typedef std::function<void(bool)> SendDataCallback;

    ClientController();
    ~ClientController();

    bool Initialize();
    void Finalize();

    // 带代理的接口，暂时未实现代理
    bool AddClient(AddClientCallback addcallback, const IpProxy& ipproxy,
        const std::string& ip, uint16 port, ClientCallback callback);

    void RemoveClient(SocketHandle handle);

    bool Send(SocketHandle handle, const std::vector<uint8>& data, SendDataCallback callback);

private:

    bool AddClient(const std::string&ip, uint16 port,
        ConnectCallback connect_cb, DataReceiveCallback data_cb);

    static void signal_cb(evutil_socket_t sock, short flags, void * args);

    void WorkerFunction();
    std::unique_ptr<std::thread> worker_thread_;
    bool exit_flag_ = false;

    struct event_base * event_base_ = 0;
    struct event* hup_event_ = 0;

    std::map<SocketHandle, SingleTcpClient*> tcp_client_map_;
};

