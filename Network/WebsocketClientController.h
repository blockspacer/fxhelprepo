#pragma once

#include <map>
#include <vector>
#include <functional>
#include "WebsocketDefine.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/memory/scoped_ptr.h"

#include "Network/IpProxy.h"

class WebsocketClient;
class WebsocketClientControllerImpl;

class WebsocketClientController
{
public:
    WebsocketClientController();
    ~WebsocketClientController();

    bool Initialize();
    void Finalize();

    bool AddClient(AddClientCallback addcallback, 
		ConnectBreakCallback connect_callback, const IpProxy& ipproxy,
		const std::string& ip, uint16 port, ClientCallback callback);

    void RemoveClient(WebsocketHandle handle);

    bool Send(WebsocketHandle handle, const std::vector<uint8>& data, SendDataCallback callback);

private:
    scoped_ptr<WebsocketClientControllerImpl> Impl_;

};

