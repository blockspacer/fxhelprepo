#pragma once

#include <map>
#include <string>
#include <vector>
#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/bind.h"

class PhoneRank
{
public:
    PhoneRank();
    ~PhoneRank();

    bool GetCityRankInfos(uint32 roomid,
        const base::Callback<void(const std::wstring&)>& callback);

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
        uint32 area_id = 0; // ʡ��id
        std::string area_name = ""; // ʡ������
        uint32 city_code = 0; // ����id
        std::string city_name = "";
        uint32 fx_city_id = 0;
        std::string gaode_code; // �ߵµ�ͼ�Գ��еı�ţ�ʵ���ϵĵ绰����
    };

    struct NormalRoomInfo // ֻȡ�˲���Ŀǰ��Ϊ���õ���Ϣ
    {
        uint32 kugou_id = 0;
        //uint32 type_ = 0; // ������ֱ��״̬�ķ���
        uint32 fanxing_id = 0;
        std::string nickname = "";
    };

    struct StarCard // ֻȡ�˲���Ŀǰ��Ϊ���õ���Ϣ
    {
        std::string location; // ������λ�����ڳ���
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

    // ��ȡ�����б�
    bool GetSinglePageData(
        const std::string& url,
        const std::map<std::string, std::string>& param_map,
        std::vector<RankSingerInfo>* rank_singer_infos,
        bool* has_next_page, bool* all_online, uint32* doubleLiveFirst,
        uint32* mobileFromIndex) const;

    std::map<std::string, std::vector<CityInfo>> province_citys_;

    std::vector<RankSingerInfo> new_singers_rank_;

    std::vector<RankSingerInfo> beautiful_singers_rank_;
};

