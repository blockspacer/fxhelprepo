#include "stdafx.h"

#include <memory>
#include "BetNetworkHelper.h"
#include "BetGameDatabase.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_util.h"

namespace
{
    uint32 max_retry_break_seconds = 256;
}

BetNetworkHelper::BetNetworkHelper()
    : tcp_manager_(new TcpManager)
    , worker_thread_(new base::Thread("worker_thread"))
    , database_(new BetGameDatabase)
    , retry_break_seconds_(1)
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
    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    path = path.Append(L"betgame.db");
    return database_->Initialize(path.value());
}

void BetNetworkHelper::Finalize()
{
    worker_thread_->Stop();
    database_->Finalize();
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

void BetNetworkHelper::SetBetResultNotify(const base::Callback<void(const  BetResult&)>& callback)
{
    result_callback_ = callback;
}

void BetNetworkHelper::SetBetTimeNotify(const base::Callback<void(uint32 time)>& callback)
{
    time_callback_ = callback;
}

void BetNetworkHelper::EnterRoom(uint32 room_id)
{
    user_->SetNotify620(
        std::bind(&BetNetworkHelper::OnBetNotify, this, std::placeholders::_1));

    if (!user_->EnterRoomFopAlive(room_id,
        base::Bind(&BetNetworkHelper::ConnectionBreakCallback,
        base::Unretained(this), room_id)))
    {
        tips_callback_.Run(L"进入房间失败");
        return;
    }
    tips_callback_.Run(L"进入房间成功");
}

void BetNetworkHelper::OnBetNotify(const BetResult& bet_result)
{
    retry_break_seconds_ = 1;// 如果是正常运行的情况下，永远重置时间间隔倍数为1倍，即repeat_seconds
    if (bet_result.result == 0)
    {
        time_callback_.Run(bet_result.time);
        return;
    }
    
    auto it = result_map_.find(bet_result.result);
    if (it == result_map_.end())
    {
        return;
    }
    BetResult new_result = bet_result;
    new_result.display_result = it->second;
    result_callback_.Run(new_result);
    database_->InsertRecord(new_result);
}

void BetNetworkHelper::ConnectionBreakCallback(
    uint32 room_id)
{
    if (retry_break_seconds_ <= max_retry_break_seconds)
    {
        retry_break_seconds_ *= 2;
    }
    
    worker_thread_->message_loop()->PostDelayedTask(FROM_HERE,
        base::Bind(&BetNetworkHelper::EnterRoom,
        base::Unretained(this), room_id),
        base::TimeDelta::FromSeconds(retry_break_seconds_));
}
