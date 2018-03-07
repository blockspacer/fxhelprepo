#include "WebsocketClientControllerImpl.h"
#include "WebsocketClient.h"

#include "third_party/chromium/base/strings/string_number_conversions.h"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>



class websocket_endpoint {
public:
    websocket_endpoint() : m_next_id(0) {}
    ~websocket_endpoint() {}
    int connect(std::string const & uri) {}
    void close(int id, websocketpp::close::status::value code, std::string reason) {}
    void send(int id, std::string message) {}
    connection_metadata::ptr get_metadata(int id) const {}
private:
    int m_next_id;
};

WebsocketClientControllerImpl::WebsocketClientControllerImpl()
    :m_next_id(0)
{
}


WebsocketClientControllerImpl::~WebsocketClientControllerImpl()
{
}

bool WebsocketClientControllerImpl::Initialize()
{
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

    m_endpoint.init_asio();
    m_endpoint.start_perpetual();

    m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
    return true;
}

void WebsocketClientControllerImpl::Finalize()
{
    m_endpoint.stop_perpetual();

    for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
        if (it->second->get_status() != "Open") {
            // Only close open connections
            continue;
        }

        std::cout << "> Closing connection " << it->second->get_id() << std::endl;

        websocketpp::lib::error_code ec;
        m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
        if (ec) {
            std::cout << "> Error closing connection " << it->second->get_id() << ": "
                << ec.message() << std::endl;
        }
    }

    m_thread->join();
}

bool WebsocketClientControllerImpl::AddClient(
    AddClientCallback addcallback, const IpProxy& ipproxy,
    const std::string& ip, uint16 port, ClientCallback callback)
{
    //WebsocketClient* client = new WebsocketClient;
    //client->Connect();

    websocketpp::lib::error_code ec;

    std::string uri = "ws://"+ip+":"+base::UintToString(port);
    client::connection_ptr con = m_endpoint.get_connection(uri, ec);

    if (ec) {
        std::cout << "> Connect initialization error: " << ec.message() << std::endl;
        return -1;
    }

    int new_id = m_next_id++;
    connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id, con->get_handle(), uri);
    m_connection_list[new_id] = metadata_ptr;

    con->set_open_handler(websocketpp::lib::bind(
        &connection_metadata::on_open,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
        ));
    con->set_fail_handler(websocketpp::lib::bind(
        &connection_metadata::on_fail,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
        ));
    con->set_close_handler(websocketpp::lib::bind(
        &connection_metadata::on_close,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
        ));
    con->set_message_handler(websocketpp::lib::bind(
        &connection_metadata::on_message,
        metadata_ptr,
        websocketpp::lib::placeholders::_1,
        websocketpp::lib::placeholders::_2
        ));

    m_endpoint.connect(con);
    addcallback(true, new_id);

    return true;
}

void WebsocketClientControllerImpl::RemoveClient(WebsocketHandle handle)
{
    websocketpp::close::status::value code;
    std::string reason;

    websocketpp::lib::error_code ec;
    int id = handle;
    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        std::cout << "> No connection found with id " << id << std::endl;
        return;
    }

    m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);
    if (ec) {
        std::cout << "> Error initiating close: " << ec.message() << std::endl;
    }
}

bool WebsocketClientControllerImpl::Send(WebsocketHandle handle,
    const std::vector<uint8>& data, SendDataCallback callback)
{
    websocketpp::lib::error_code ec;
    int id = handle;
    con_list::iterator metadata_it = m_connection_list.find(id);
    //if (metadata_it == m_connection_list.end()) {
    //    std::cout << "> No connection found with id " << id << std::endl;
    //    return;
    //}

    std::string message(data.begin(), data.end());
    m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cout << "> Error sending message: " << ec.message() << std::endl;
        return false;
    }

    return true;
}
