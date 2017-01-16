#pragma once
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include "event2/event.h"
#include "event2/util.h"
#include "event2/thread.h"
#include "third_party/chromium/base/basictypes.h"

typedef evutil_socket_t SocketHandle;

typedef std::function<void(bool, std::vector<uint8>&)> DataReceiveCallback;
typedef std::function<void(bool, SocketHandle)> ConnectCallback;
typedef std::function<void(bool)> SendDataCallback;

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
    static void write_cb(evutil_socket_t sock, short flags, void * args);
    static void read_cb(evutil_socket_t sock, short flags, void * args);

    void OnConnect(evutil_socket_t sock, short flags);
    void OnReceive(evutil_socket_t sock, short flags);

    struct event_base * base_;
    evutil_socket_t sock_;

    struct event *ev_read_;
    struct event *ev_write_;
    DataReceiveCallback data_receive_callback_;
    ConnectCallback connect_callback_;
};