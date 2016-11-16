#include "stdafx.h"

#include <memory>
#include "BetNetworkHelper.h"

#include "third_party/chromium/base/strings/utf_string_conversions.h"


BetNetworkHelper::BetNetworkHelper()
    :tcp_manager_(new TcpManager)
    ,worker_thread_(new base::Thread("worker_thread"))
{
    result_map_[31] = 1;
    result_map_[29] = 2;
    result_map_[27] = 7;
    result_map_[25] = 8;
    result_map_[30] = 3;
    result_map_[28] = 4;
    result_map_[26] = 5;
    result_map_[24] = 6;
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

    user_->SetRoomServerIp("114.54.2.204");
    user_->SetTcpManager(tcp_manager_.get());
    std::string errmsg;
    if (!user_->Login(account, password, "", &errmsg))
    {
        tips_callback_.Run(base::UTF8ToUTF16(errmsg));
        return false;
    }
    tips_callback_.Run(L"登录成功");
    return true;
}

void BetNetworkHelper::SetTipMessage(const base::Callback<void(const std::wstring&)>& callback)
{
    tips_callback_ = callback;
}

void BetNetworkHelper::SetBetResultNotify(const base::Callback<void(uint32)>& callback)
{
    result_callback_ = callback;
}

bool BetNetworkHelper::EnterRoom(uint32 room_id)
{
    user_->SetNotify620(std::bind(&BetNetworkHelper::OnBetNotify,
        this, std::placeholders::_1));
    if (!user_->EnterRoomFopAlive(room_id))
    {
        tips_callback_.Run(L"进入房间失败");
        return false;
    }
    tips_callback_.Run(L"进入房间成功");
    return true;
}

void BetNetworkHelper::OnBetNotify(const BetResult& bet_result)
{
    auto it = result_map_.find(bet_result.result);
    if (it == result_map_.end())
    {
        return;
    }
    
    result_callback_.Run(it->second);
}
