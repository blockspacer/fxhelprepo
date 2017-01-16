#pragma once
#include <string>
#include <vector>
#include <thread>
#include <memory>

#include "third_party/chromium/base/basictypes.h"
#include "Network/TcpDefines.h"

struct event_base;
struct event;

class SingleTcpClient
{
public:
    SingleTcpClient();
    ~SingleTcpClient();
    SocketHandle Connect(struct event_base * base, const std::string&ip, uint16 port,
                 ConnectCallback connect_cb, DataReceiveCallback data_cb);

    bool Send(const std::vector<uint8>& data);
    void Close();

private:
    static void write_cb(intptr_t sock, short flags, void * args);
    static void read_cb(intptr_t sock, short flags, void * args);

    void OnConnect(intptr_t sock, short flags);
    void OnReceive(intptr_t sock, short flags);

    struct event_base * base_;
    intptr_t sock_;

    struct event *ev_read_;
    struct event *ev_write_;
    DataReceiveCallback data_receive_callback_;
    ConnectCallback connect_callback_;
};