#include "WebsocketClientController.h"
#include "WebsocketClient.h"

#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "Network/WebsocketClientControllerImpl.h"

WebsocketClientController::WebsocketClientController()
    :Impl_(new WebsocketClientControllerImpl())
{
}


WebsocketClientController::~WebsocketClientController()
{
}

bool WebsocketClientController::Initialize()
{
    return Impl_->Initialize();
}

void WebsocketClientController::Finalize()
{
    Impl_->Finalize();
}

bool WebsocketClientController::AddClient(
    AddClientCallback addcallback, const IpProxy& ipproxy,
    const std::string& ip, uint16 port, ClientCallback callback)
{
    return Impl_->AddClient(addcallback, ipproxy, ip, port, callback);
}

void WebsocketClientController::RemoveClient(WebsocketHandle handle)
{
    return Impl_->RemoveClient(handle);
}

bool WebsocketClientController::Send(WebsocketHandle handle,
    const std::vector<uint8>& data, SendDataCallback callback)
{
    return Impl_->Send(handle, data, callback);
}
