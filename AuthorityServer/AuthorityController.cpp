#include "stdafx.h"
#include "AuthorityController.h"


AuthorityController::AuthorityController()
{
}


AuthorityController::~AuthorityController()
{
}

bool AuthorityController::Initialize()
{
    return true;
}
void AuthorityController::Finalize()
{

}

void AuthorityController::SetSendDataFunction(
    base::Callback<bool(bufferevent*, const std::vector<uint8>& data)> callback)
{
    send_data_callback_ = callback;
}

bool AuthorityController::Start()
{
    return true;
}

void AuthorityController::Stop()
{

}

bool AuthorityController::AddClient(bufferevent *bev, struct sockaddr& sock)
{
    auto find_result = client_map_.find(bev);
    if (find_result != client_map_.end())
        return false;

    scoped_ptr<AuthorityClientBussiness> client(new AuthorityClientBussiness());
    client->SetSendDataFunction(
        base::Bind(&AuthorityController::SendDataToClient,
        base::Unretained(this), bev));
    client_map_[bev] = client.Pass();
}

bool AuthorityController::RemoveClient(bufferevent *bev)
{
    return false;
}

bool AuthorityController::HandleMessage(
    bufferevent *bev, const std::vector<uint8>& data)
{
    if (send_data_callback_.is_null())
        return false;

    auto find_result = client_map_.find(bev);

    if (find_result == client_map_.end())
        return false;

    // 组包后，处理数据
    if (!find_result->second->HandleMessage(data))
        return false;

    return true;
}

bool AuthorityController::SendDataToClient(bufferevent* bev, 
                                           const std::vector<uint8>& data)
{
    return send_data_callback_.Run(bev, data);
}

//bool AuthorityController::AddUser();
//bool AuthorityController::RemoveUser();
//bool AuthorityController::QueryUser();
//bool AuthorityController::ModifyUser();
