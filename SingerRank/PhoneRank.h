#pragma once

#include <map>
#include <string>
#include <vector>
#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/threading/thread.h"
#include "third_party/chromium/base/single_thread_task_runner.h"
#include "third_party/chromium/base/memory/scoped_ptr.h"

typedef std::vector<std::wstring> RowData;
typedef std::vector<RowData> GridData;

class PhoneRank
{
public:
    PhoneRank();
    ~PhoneRank();

    bool Initialize(const base::Callback<void(const GridData&)>& singer_info_callback,
        const base::Callback<void(const std::wstring&)>& message_callback);

    void Finalize();

    void BreakRequest();

    void DoStop();

    bool GetCityRankInfos(uint32 roomid,
        const base::Callback<void(const std::wstring&)>& callback);

    bool InitCheckGroupSingers(bool beauty, bool newsinger);

    bool InitNewSingerRankInfos();
    bool GetNewSingerRankByRoomid(uint32 roomid, uint32* rank, uint32* all) const;

    bool InitBeautifulSingerRankInfos();
    bool GetBeautifulSingerRankByRoomid(uint32 roomid, uint32* rank, uint32* all) const;

private:
    struct RankSingerInfo
    {
        std::string activityPic = "";
        uint32 baiduCode = 0;
        std::string cityName = "";
        std::string company = "";
        std::string imgPath = "";
        uint32 isOriginal = 0;
        uint32 kugouId = 0;
        std::string labelName = "";
        uint32 lastLiveTime = 0;
        std::string liveTitle = "";
        std::string nickName = "";
        uint32 roomId = 0;
        uint32 starLevel = 0;
        uint32 status = 0;
        uint32 userId = 0;
        uint32 viewerNum = 0;
        double rankScore = 0.0;
        double score = 0.0;
    };

    struct CityInfo
    {
        uint32 area_id = 0; // 省份id
        std::string area_name = ""; // 省份名称
        uint32 city_code = 0; // 城市id
        std::string city_name = "";
        uint32 fx_city_id = 0;
        std::string gaode_code; // 高德地图对城市的编号，实际上的电话区号
    };

    struct NormalRoomInfo // 只取了部分目前认为有用的信息
    {
        uint32 kugou_id = 0;
        //uint32 type_ = 0; // 好像是直播状态的分类
        uint32 fanxing_id = 0;
        std::string nickname = "";
    };

    struct StarCard // 只取了部分目前认为有用的信息
    {
        std::string location; // 主播定位的所在城市
    };

    bool GetCityInfos(std::map<std::string, std::vector<CityInfo>>* province_citys) const;

    bool GetRankSingerListByCity(const CityInfo& city_info, std::vector<RankSingerInfo>* rank_singer_infos) const;
    bool GetSinglePageDataByCity(
        const std::map<std::string, std::string>& query_city_rank_param,
        std::vector<RankSingerInfo>* rank_singer_infos,
        bool* has_next_page, uint32* online_number) const;

    bool GetEnterRoomInfoByRoomId(uint32 roomid, 
        NormalRoomInfo* normal_room_info) const;

    bool GetStarCardByKugouId(uint32 kugouid, StarCard* star_card) const;

    // 获取新秀列表
    bool GetSinglePageData(
        const std::string& url,
        const std::map<std::string, std::string>& param_map,
        std::vector<RankSingerInfo>* rank_singer_infos,
        bool* has_next_page, bool* all_online, uint32* doubleLiveFirst,
        uint32* mobileFromIndex) const;

    base::Thread worker_thread_;
    scoped_refptr<base::SingleThreadTaskRunner> runner_;


    std::map<std::string, std::vector<CityInfo>> province_citys_;

    std::vector<RankSingerInfo> new_singers_rank_;

    std::vector<RankSingerInfo> beautiful_singers_rank_;

    base::Callback<void(const GridData&)> singer_info_callback_;
    base::Callback<void(const std::wstring&)> message_callback_;

    // 优化体验的参数
    bool break_all_request_;
};

