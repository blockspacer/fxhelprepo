#include "stdafx.h"

#include <memory>
#include "BetNetworkHelper.h"


BetNetworkHelper::BetNetworkHelper()
    :tcp_manager_(new TcpManager)
    ,worker_thread_(new base::Thread("worker_thread"))
{
}

BetNetworkHelper::~BetNetworkHelper()
{
}

bool BetNetworkHelper::Initialize()
{
    return true;
}

void BetNetworkHelper::Finalize()
{
}

bool BetNetworkHelper::Login(const std::string& account, const std::string& password)
{
    CurlWrapper::CurlInit();
    worker_thread_->Start();
    user_.reset(new User);
    if (!tcp_manager_->Initialize())
        return false;

    //AuthorityHelper authorityHelper;
    //bool result = authorityHelper.Load(authority_.get());
    //assert(!authority_->serverip.empty());
    user_->SetRoomServerIp("114.54.2.204");
    user_->SetTcpManager(tcp_manager_.get());

    return true;
}

void BetNetworkHelper::SetBetResultNotify(const base::Callback<void(uint32)>& callback)
{
    result_callback_ = callback;
    return;
}

bool BetNetworkHelper::EnterRoom(uint32 room_id)
{
    user_->SetNotify620(std::bind(&BetNetworkHelper::OnBetNotify,
        this, std::placeholders::_1));
    user_->EnterRoomFopAlive(room_id);
    return false;
}

void BetNetworkHelper::OnBetNotify(const BetResult& bet_result)
{

}
