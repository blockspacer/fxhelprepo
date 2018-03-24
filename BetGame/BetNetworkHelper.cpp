#include "stdafx.h"

#include <memory>
#include "BetNetworkHelper.h"
#include "BetGameDatabase.h"
#include "Network/WebsocketClientController.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_util.h"
#include "MathHelper.h"

namespace
{
    uint32 max_retry_break_seconds = 256;
}

BetNetworkHelper::BetNetworkHelper()
    : websocket_client_controller_(new WebsocketClientController)
    , worker_thread_(new base::Thread("worker_thread"))
    , database_(new BetGameDatabase)
    , retry_break_seconds_(1)
{
    result_map_[31] = 8; // ��֤
    result_map_[29] = 7; // ��֤
    result_map_[27] = 2; // ��֤
    result_map_[25] = 1; // ��֤
    result_map_[30] = 6; // ȷ��
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
    if (!websocket_client_controller_->Initialize())
        return false;

    user_->SetRoomServerIp("114.54.2.204");
    user_->SetWebsocketClientController(websocket_client_controller_.get());
    std::string errmsg;
    if (!user_->Login(account, password, "", &errmsg))
    {
        tips_callback_.Run(base::UTF8ToUTF16(errmsg));
        return false;
    }
    tips_callback_.Run(L"��¼�ɹ�");
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
        tips_callback_.Run(L"���뷿��ʧ��");
        return;
    }
    tips_callback_.Run(L"���뷿��ɹ�");
}

void BetNetworkHelper::OnBetNotify(const BetResult& bet_result)
{
    retry_break_seconds_ = 1;// ������������е�����£���Զ����ʱ��������Ϊ1������repeat_seconds
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
    if (caculation_map_.empty()) // ��һ������
    {
        CaculationData data;
        data.bet_result = bet_result;
        for (int i = 0; i < 8; i++)
        {
            data.avg_distance[i] = 0;
            data.summary[i] = 0;// �ۼ�����
            data.sum_distance[i] = 0;;; // ͳ���ܼ��
            data.distance[i] = 0;// ÿ�������������û��
            data.max_distance[i] = 0;// �����
            data.mid_distance[i] = 0; // �����λ��
            data.avg_distance[i] = 0.0; // ƽ�����
            data.standard_deviation[i] = 0.0;// �����׼��
            data.frequence[i] = 0.0; // Ƶ�� = �ۼ�����/��������
        }
        caculation_map_[bet_result] = data;
        return;
    }

    const uint32& display_result = bet_result.display_result;
    auto caculate_data = caculation_map_.end();
    caculate_data--; // ���һ��
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
            // ��¼�����������λ���ͼ����׼��
            distance_map_[i].push_back(last_data.distance[i]);
            new_data.mid_distance[i] = MathHelper::GetMid(distance_map_[i]);
            new_data.standard_deviation[i] = MathHelper::GetStandardDeviation(distance_map_[i], new_data.avg_distance[i]);

            if (new_data.distance[i] > new_data.max_distance[i])
            {
                LOG(INFO) << L"max_distance" << L"[" << i << L"] update(" << new_data.max_distance[i] << L") ";
                new_data.max_distance[i] = new_data.distance[i];
            }

            new_data.distance[i] = 0; //��ǰ�����Ǹ�distanceΪ0;
        }
        else
        {
            new_data.distance[i]++;
        }

        if (new_data.summary[i])// �����0����
        {
            new_data.frequence[i] = (new_data.summary[i] * 100.0) / (new_data.index*1.0);
        }
    }
    //BetResult bet_result;
    //uint32 index = 0; // �����������Զ����0��ʼ
    //uint32 summary[8];// �ۼ�����
    //uint32 sum_distance[8];; // ͳ���ܼ��
    //uint32 distance[8]; // ÿ�������������û��
    //uint32 max_distance[8]; // �����
    //uint32 mid_distance[8]; // �����λ��
    //double avg_distance[8]; // ƽ�����
    //double standard_deviation[8]; // �����׼��
    //double frequence[8]; // Ƶ�� = �ۼ�����/��������
    LOG(INFO) << L"index(" << new_data.index+1 << L") "<< L"bet_result = " << bet_result.display_result;
    for (int i = 0; i < 8; i++)
    {
        LOG(INFO) << L"result(" << i+1 << L") " << L"summary= " << new_data.summary[i]
            << L" sum_distance= " << new_data.sum_distance[i]
            << L" distance= " << new_data.distance[i]
            << L" max_distance= " << new_data.max_distance[i]
            //<< L" mid_distance= " << new_data.mid_distance[i]
            << L" avg_distance= " << base::DoubleToString(new_data.avg_distance[i])
            << L" standard_deviation= " << base::DoubleToString(new_data.standard_deviation[i])
            << L" frequence= " << base::DoubleToString(new_data.frequence[i]);
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
