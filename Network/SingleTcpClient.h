#pragma once
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include "third_party/libevent/include/event2/event.h"
#include "third_party/libevent/include/event2/util.h"
#include "third_party/libevent/include/event2/thread.h"
#include "third_party/chromium/base/basictypes.h"

typedef evutil_socket_t TCPHANDLE;

typedef std::function<void(bool, std::vector<uint8>&)> DataReceiveCallback;
typedef std::function<void(bool, TCPHANDLE)> ConnectCallback;


class SingleTcpClient
{
public:
    SingleTcpClient();
    ~SingleTcpClient();
    TCPHANDLE Connect(struct event_base * base, const std::string&ip, uint16 port,
                 ConnectCallback connect_cb, DataReceiveCallback data_cb);
    bool Send(const std::vector<uint8>& data);
    void Close();

private:
    static void write_cb(evutil_socket_t sock, short flags, void * args);
    static void read_cb(evutil_socket_t sock, short flags, void * args);

    struct event_base * base_;
    evutil_socket_t sock_;

    struct event *ev_read_;
    struct event *ev_write_;
    DataReceiveCallback data_receive_callback_;
    ConnectCallback connect_callback_;
};