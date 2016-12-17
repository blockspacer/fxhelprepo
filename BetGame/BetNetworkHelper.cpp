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
    result_map_[31] = 8;
    result_map_[29] = 7;
    result_map_[27] = 2;
    result_map_[25] = 1;
    result_map_[30] = 6; // 确认
    result_map_[28] = 5;
    result_map_[26] = 4;
    result_map_[24] = 3;

    for (int i = 0; i < 8; i++)
    {
        distance_map_[i] = std::vector<uint32>();
    }

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

    InsertToCaculationMap(new_result);
    result_callback_.Run(new_result);
    database_->InsertRecord(new_result);
}

void BetNetworkHelper::InsertToCaculationMap(const BetResult& bet_result)
{
    if (caculation_map_.empty()) // 第一个数据
    {
        CaculationData data;
        data.bet_result = bet_result;
        for (int i = 0; i < 8; i++)
        {
            data.avg_distance[i] = 0;
            data.summary[i] = 0;// 累计期数
            data.sum_distance[i] = 0;;; // 统计总间隔
            data.distance[i] = 0;// 每个数间隔多少期没开
            data.max_distance[i] = 0;// 最大间隔
            data.mid_distance[i] = 0; // 间隔中位数
            data.avg_distance[i] = 0.0; // 平均间隔
            data.variance_distance[i] = 0.0;// 间隔方差
            data.frequence[i] = 0.0; // 频率 = 累计期数/开奖期数
        }
        caculation_map_[bet_result] = data;
        return;
    }

    const uint32& display_result = bet_result.display_result;
    auto caculate_data = caculation_map_.end();
    caculate_data--; // 最后一个
    const CaculationData& last_data = caculate_data->second;
    CaculationData new_data = last_data;
    new_data.index++;

    for (int i = 0; i < 8; i++)
    {
        if (i == display_result - 1)
        {
            if (new_data.distance[i] > last_data.max_distance[i])
            {
                new_data.max_distance[i] = new_data.distance[i];
            }
            new_data.summary[i]++;
            new_data.sum_distance[i] += last_data.distance[i];
            new_data.avg_distance[i] = (new_data.sum_distance[i]*1.0) / (new_data.summary[i]*1.0);
            // 记录间隔，计算中位数和方差
            distance_map_[i].push_back(last_data.distance[i]);
            new_data.mid_distance[i] = 0;
            new_data.variance_distance[i] = 0.0;
            new_data.distance[i] = 0; //当前开奖那个distance为0;
        }
        else
        {
            new_data.distance[i]++;
        }

        if (new_data.summary[i])// 避免除0错误
        {
            new_data.frequence[i] = (new_data.summary[i]*1.0)/(new_data.index*1.0);
        }
    }
    //BetResult bet_result;
    //uint32 index = 0; // 开奖期数，自定义从0开始
    //uint32 summary[8];// 累计期数
    //uint32 sum_distance[8];; // 统计总间隔
    //uint32 distance[8]; // 每个数间隔多少期没开
    //uint32 max_distance[8]; // 最大间隔
    //uint32 mid_distance[8]; // 间隔中位数
    //double avg_distance[8]; // 平均间隔
    //double variance_distance[8]; // 间隔方差
    //double frequence[8]; // 频率 = 累计期数/开奖期数
    LOG(INFO) << L"index(" << new_data.index << L") "<< L"bet_result = " << bet_result.display_result;
    for (int i = 0; i < 8; i++)
    {
        LOG(INFO) << L"result(" << i+1 << L") " << L"summary= " << new_data.summary[i]
            << L" sum_distance= " << new_data.sum_distance[i]
            << L" distance= " << new_data.distance[i]
            << L" max_distance= " << new_data.max_distance[i]
            //<< L" mid_distance= " << new_data.mid_distance[i]
            << L" avg_distance= " << new_data.avg_distance[i]
            //<< L" variance_distance= " << new_data.variance_distance[i]
            << L" frequence= " << new_data.frequence[i];
    }
    caculation_map_[bet_result] = new_data;
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
