#pragma once
#include <string>
#include <vector>
#include <map>
#include "third_party/chromium/base/basictypes.h"


struct SingerInfo
{
    uint32 roomid;
    uint32 userid;
    uint32 clanid;
    uint32 lastday;//最后一次上播离现在的天数，一个月按30天算
    uint32 fans;// 粉丝数量
    uint32 pictures;//个人相册照片数量
    std::string nickname;
    std::string richlevel;
    std::string starlevel;
    std::string roomurl;
    std::string userurl;
    std::string lastactiionString;
};

// 家族数据信息
struct FamilyInfo
{
    uint32 clanid;
    std::string clanname;
    std::string badgename;//家族简称
    uint32 clanroomid;
    uint64 cointotal;
    uint32 clanlevel;
    uint32 clanleaderuserid;
    std::string clanleadernickname;
    uint32 managercount;
    uint32 subleadercount;
    uint32 usercount;
    uint32 starcount;
    uint64 addtime;//unix time;
    bool iscompany;
};

// 本类功能是从全站找到很长时间没有上的主播
class SearchHelper
{
public:
    SearchHelper();
    ~SearchHelper();

    static void CurlInit();
    static void CurlCleanup();

    void Run();
    void SetFilter(uint32 lastactive,uint32 pagecount);
    bool GetExpiredFamilySingers(std::map<uint32, FamilyInfo>* familyInfoMap,
        std::vector<SingerInfo>* singerinfo);

    bool GetExpiredNormalSingers(std::vector<SingerInfo>* singerinfo);

private:

    bool GetUrlData(const std::string& url, std::string* response);
    bool GetAllFamilyInfo(std::map<uint32,FamilyInfo>* familyInfoMap);
    bool GetAllSingers(uint32 clanid, std::vector<std::string>* singerIdList);
    bool GetSingsInfos(const std::string& singerId, SingerInfo* singerinfo);
    bool WriteToFile(const std::map<uint32, FamilyInfo>& familyInfoMap,
        const std::vector<SingerInfo>& singerinfos);
    uint32 lastactive_;
    uint32 pagecount_;
};

