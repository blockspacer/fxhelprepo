#include "stdafx.h"
#include "AuthorityController.h"


AuthorityController::AuthorityController()
    :control_thread_(nullptr)
{
}


AuthorityController::~AuthorityController()
{
}

bool AuthorityController::Initialize()
{
    control_thread_.reset(new base::Thread("controller_thread"));
    control_thread_->Start();
    runner_ = control_thread_->message_loop_proxy();
    return true;
}
void AuthorityController::Finalize()
{
    control_thread_->Stop();
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

bool AuthorityController::AddClient(bufferevent *bev, const struct sockaddr& sock)
{
    return runner_->PostTask(FROM_HERE, base::Bind(&AuthorityController::DoAddClient, 
                      base::Unretained(this), bev, sock));
}

void AuthorityController::DoAddClient(bufferevent *bev, const struct sockaddr& sock)
{
    auto find_result = client_map_.find(bev);
    if (find_result != client_map_.end())
        return;

    scoped_ptr<AuthorityClientBussiness> client(new AuthorityClientBussiness());
    client->SetSendDataFunction(
        base::Bind(&AuthorityController::SendDataToClient,
        base::Unretained(this), bev));
    client_map_[bev] = client.Pass();
}

bool AuthorityController::RemoveClient(bufferevent *bev)
{
    runner_->PostTask(FROM_HERE, base::Bind(&AuthorityController::DoRemoveClient,
        base::Unretained(this), bev));

    return false;
}

void AuthorityController::DoRemoveClient(bufferevent *bev)
{

}

bool AuthorityController::HandleMessage(
    bufferevent *bev, const std::vector<uint8>& data)
{
    if (send_data_callback_.is_null())
        return false;

    runner_->PostTask(FROM_HERE, base::Bind(&AuthorityController::DoHandleMessage,
        base::Unretained(this), bev, data));

    return true;
}

void AuthorityController::DoHandleMessage(
    bufferevent *bev, const std::vector<uint8>& data)
{
    auto find_result = client_map_.find(bev);

    if (find_result == client_map_.end())
        return;

    if (!find_result->second->HandleMessage(data))
    {
        // 无法处理消息，要终结这条连接
    }
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
