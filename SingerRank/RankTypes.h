#pragma once
#include <vector>
#include <string>


#include "third_party/chromium/base/basictypes.h"

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
