#pragma once

#include <map>
#include <vector>
#include <functional>
#include "WebsocketDefine.h"
#include "third_party/chromium/base/basictypes.h"

#include "Network/IpProxy.h"
#include "ConnectMetadata.h"

class WebsocketClient;

class WebsocketClientController
{
public:
    typedef std::function<void(bool, WebsocketHandle)> AddClientCallback;
    typedef std::function<void(bool, const std::vector<uint8>&)> ClientCallback;
    typedef std::function<void(WebsocketHandle, bool)> SendDataCallback;

    WebsocketClientController();
    ~WebsocketClientController();

    bool Initialize();
    void Finalize();

    bool AddClient(AddClientCallback addcallback, const IpProxy& ipproxy,
        const std::string& ip, uint16 port, ClientCallback callback);

    void RemoveClient(WebsocketHandle handle);

    bool Send(WebsocketHandle handle, const std::vector<uint8>& data, SendDataCallback callback);

//private:
//    std::map<uint32, WebsocketClient*> websocket_client_map_;
//
private:
    typedef std::map<int, connection_metadata::ptr> con_list;

    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    con_list m_connection_list;
    int m_next_id;

};

