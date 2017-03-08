#pragma once

#include <string>
#include <vector>

#undef max
#undef min
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/callback.h"
#include "Network/TcpClientController.h"
#include "Network/User.h"

// 即时数据分析结果，每一期数据开出的时候都计算进去
struct CaculationData
{
    BetResult bet_result;
    uint32 periods = 0; // 开奖期数，自定义从0开始
    uint32 summary[8];// 累计中奖期数
    uint32 sum_distance[8];; // 统计总间隔
    uint32 distance[8]; // 每个数间隔多少期没开
    uint32 max_distance[8]; // 最大间隔
    uint32 mid_distance[8]; // 间隔中位数
    double avg_distance[8]; // 平均间隔
    double standard_deviation[8]; // 间隔标准差
    double frequence[8]; // 频率 = 累计期数/开奖期数
};

class BetGameDatabase;
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
    void SetBetResultNotify(const base::Callback<void(const  CaculationData&)>& callback);
    void SetBetTimeNotify(const base::Callback<void(uint32 time)>& callback);

private:
    void OnBetNotify(const  BetResult& bet_result);

    void InsertToCaculationMap(const BetResult& bet_result,
                               CaculationData* caculation_data);

    // 提供连接状态出错重连的功能
    void ConnectionBreakCallback(uint32 room_id);

    std::unique_ptr<TcpClientController> tcp_manager_;
    std::unique_ptr<User> user_;
    base::Callback<void(const std::wstring&)> tips_callback_;
    base::Callback<void(const  CaculationData&)> result_callback_;
    base::Callback<void(uint32)> time_callback_;
    scoped_ptr<base::Thread> worker_thread_;
    std::map<uint32, uint32> result_map_;
    std::unique_ptr<BetGameDatabase> database_;
    uint32 retry_break_seconds_; // 用来增加每批重试的间隔

    
    // 初始化时，拿到所有数据时先算一次以下值；每个结果出来也需要算出来的数据
    // 1. 每个数间隔多少期没开
    // 2. 每个数一共开多少次，开的概率是多少
    // 3. 基于每个数开的时候间隔值是多少，计算最大间隔值是多少，间隔值中位数是多少，平均间隔是多少，方差是多少
    std::map<BetResult, CaculationData> caculation_map_;

    std::map<uint32, std::vector<uint32>> distance_map_;// 每个数间隔次数的记录
};

