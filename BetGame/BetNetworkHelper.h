#pragma once

#include <string>
#include <vector>

#undef max
#undef min
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/callback.h"
#include "Network/User.h"

// ��ʱ���ݷ��������ÿһ�����ݿ�����ʱ�򶼼����ȥ
struct CaculationData
{
    BetResult bet_result;
    uint32 index = 0; // �����������Զ����0��ʼ
    uint32 summary[8];// �ۼ�����
    uint32 sum_distance[8];; // ͳ���ܼ��
    uint32 distance[8]; // ÿ�������������û��
    uint32 max_distance[8]; // �����
    uint32 mid_distance[8]; // �����λ��
    double avg_distance[8]; // ƽ�����
    double standard_deviation[8]; // �����׼��
    double frequence[8]; // Ƶ�� = �ۼ�����/��������
};

class BetGameDatabase;
class WebsocketClientController;
class BetNetworkHelper
{
public:
    BetNetworkHelper();
    ~BetNetworkHelper();

    bool Initialize();
    void Finalize();

    bool Login(const std::string& account, const std::string& password);
    void EnterRoom(uint32 room_id);

    void SetTipMessage(const base::Callback<void(const std::wstring&)>& callback);
    void SetBetResultNotify(const base::Callback<void(const  BetResult&)>& callback);
    void SetBetTimeNotify(const base::Callback<void(uint32 time)>& callback);

private:
    void OnBetNotify(const  BetResult& bet_result);

    void InsertToCaculationMap(const BetResult& bet_result);

    // �ṩ����״̬���������Ĺ���
    void ConnectionBreakCallback(uint32 room_id);

    std::unique_ptr<WebsocketClientController> websocket_client_controller_;
    std::unique_ptr<User> user_;
    base::Callback<void(const std::wstring&)> tips_callback_;
    base::Callback<void(const  BetResult&)> result_callback_;
    base::Callback<void(uint32)> time_callback_;
    scoped_ptr<base::Thread> worker_thread_;
    std::map<uint32, uint32> result_map_;
    std::unique_ptr<BetGameDatabase> database_;
    uint32 retry_break_seconds_; // ��������ÿ�����Եļ��

    
    // ��ʼ��ʱ���õ���������ʱ����һ������ֵ��ÿ���������Ҳ��Ҫ�����������
    // 1. ÿ�������������û��
    // 2. ÿ����һ�������ٴΣ����ĸ����Ƕ���
    // 3. ����ÿ��������ʱ����ֵ�Ƕ��٣����������ֵ�Ƕ��٣����ֵ��λ���Ƕ��٣�ƽ������Ƕ��٣������Ƕ���
    std::map<BetResult, CaculationData> caculation_map_;

    std::map<uint32, std::vector<uint32>> distance_map_;// ÿ������������ļ�¼
};

