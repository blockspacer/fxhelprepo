#pragma once

#include <map>
#include <vector>
#include <functional>
#include "WebsocketDefine.h"
#include "third_party/chromium/base/basictypes.h"

#include "Network/IpProxy.h"
#include "ConnectMetadata.h"

class WebsocketClient;

class WebsocketClientControllerImpl
{
public:

    WebsocketClientControllerImpl();
    ~WebsocketClientControllerImpl();

    bool Initialize();
    void Finalize();

    bool AddClient(AddClientCallback addcallback, const IpProxy& ipproxy,
        const std::string& ip, uint16 port, ClientCallback callback);

    void RemoveClient(WebsocketHandle handle);

    bool Send(WebsocketHandle handle, const std::vector<uint8>& data, SendDataCallback callback);

private:
    typedef std::map<int, connection_metadata::ptr> con_list;

    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    con_list m_connection_list;
    int m_next_id;

};

