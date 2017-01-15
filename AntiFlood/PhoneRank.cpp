#include "stdafx.h"
#include "PhoneRank.h"

#include <iostream>
#include <xutility>
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/time/time.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"

namespace
{

//baiduCode = 257 & cityName = 广州市&page = 1 & pageSize = 80 & platform = 5 & version = 6582$_fan_xing_$
//
//计算sing的方案
//cityName使用中文编码，不要转换成urlencode
//1. 所有参数以key - value方式放进map中
//2. 按key升序排列出key1 = value1&key2 = value2的格式的字符串str1
//3. 在str1后面紧跟$_fan_xing_$串，然后计算md5值
//4. 取md5的第8位到24位，一共是16位数据。

std::string GetSignFromMap(const std::map<std::string,std::string>& param_map)
{
    if (param_map.empty())
        return std::string("");

    std::string target;
    for (const auto& it : param_map)
    {
        target += it.first + "=" + it.second + "&";
    }
    target = target.substr(0, target.length() - 1);
    target += "$_fan_xing_$";
    std::string md5 = MakeMd5FromString(target);

    return md5.substr(8, 16);
}

}

PhoneRank::PhoneRank()
{
}

PhoneRank::~PhoneRank()
{
}

bool PhoneRank::GetCityRankInfos(uint32 roomid,
    const base::Callback<void(const std::wstring&)>& callback,
    std::wstring* display_msg)
{
    std::map<std::string, std::vector<CityInfo>> province_citys;
    bool result = GetCityInfos(&province_citys);

    NormalRoomInfo normal_room_info;
    if (!GetEnterRoomInfoByRoomId(roomid, &normal_room_info))
    {
        callback.Run(L"无法获取对应房间的主播信息");
        return false;
    }
    
    StarCard star_card;
    if (!GetStarCardByKugouId(normal_room_info.kugou_id, &star_card))
    {
        callback.Run(L"无法获取主播位置信息");
        return false;
    }
    
    std::string city_name = star_card.location;
    std::wstring w_city_name = base::UTF8ToWide(city_name);

    CityInfo city_info;
    bool found = false;
    for (const auto& province : province_citys)
    {
        for (const auto& it : province.second)
        {
            if (it.city_name.compare(city_name) == 0)
            {
                city_info = it;
                found = true;
                break;
            }
        }
        if (found)
            break;
    }

    if (!found)
    {
        std::wstring msg = L"在城市列表无法获取城市：" + base::UTF8ToWide(star_card.location);
        callback.Run(msg);
        return false;
    }

    std::vector<RankSingerInfo> rank_singer_infos;
    if (!GetRankSingerListByCity(city_info, &rank_singer_infos))
    {
        callback.Run(L"无法获取主播所在城市的主播列表");
        return false;
    }

    // 查看某个主播id的手机推荐排名
    uint32 singer_fanxing_id = normal_room_info.fanxing_id;
    uint32 rank_id = 0;
    uint32 count = 0;
    uint32 online_num = 0;
    RankSingerInfo singer_info;
    for (const auto& it : rank_singer_infos)
    {
        count++;
        if (it.userId == singer_fanxing_id)
        {
            rank_id = count;
            singer_info = it;
        }

        if (it.status) // status=1,电脑直播 status=2,手机直播
            online_num++;
    }

    std::wstring msg = L"排名/在线/" + w_city_name +L": ";
    msg += base::UintToString16(rank_id) + L"/";
    msg += base::UintToString16(online_num) + L"/";
    msg += base::UintToString16(rank_singer_infos.size());
    callback.Run(msg);

    *display_msg = msg;

    if (!rank_id)
    {
        callback.Run(L"在定位城市无法找到主播");
        return false;
    }

    if (!singer_info.status)
    {
        callback.Run(L"主播为未开播状态");
    }

    return true;
}

bool PhoneRank::GetSinglePageDataByCity(
    const QueryCityRankParam& query_city_rank_param,
    std::vector<RankSingerInfo>* rank_singer_infos,
    bool* has_next_page, uint32* online_number) const
{
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = "http://mo.fanxing.kugou.com/mfx/rank/cdn/room/cityLbs_v2/";
    request.queries["baiduCode"] = base::UintToString(query_city_rank_param.baidu_code);
    request.queries["cityName"] = UrlEncode(query_city_rank_param.city_name);
    //request.queries["cityName"] = query_city_rank_param.city_name;
    request.queries["page"] = base::UintToString(query_city_rank_param.page_number);
    request.queries["pageSize"] = base::UintToString(query_city_rank_param.page_size);
    request.queries["platform"] = base::UintToString(query_city_rank_param.platform);
    request.queries["sign"] = query_city_rank_param.sign;
    request.queries["version"] = base::UintToString(query_city_rank_param.version);;

    request.useragent = "繁星直播 3.0.5 rv:3.0.5.9 (iPhone; iPhone OS 8.4; zh_CN)";

    // 数据分析
    CurlWrapper curl_wrapper;
    HttpResponse http_reponse;
    if (!curl_wrapper.Execute(request, &http_reponse))
        return false;

    std::string content;
    content.assign(http_reponse.content.begin(),
        http_reponse.content.end());

    Json::Reader reader;
    Json::Value root(Json::objectValue);
    if (!reader.parse(content, root, false))
        return false;

    uint32 code = GetInt32FromJsonValue(root, "code");
    if (code != 0)
        return false;

    Json::Value defaultval(Json::objectValue);
    auto data = root.get("data", defaultval);
    if (!data.isObject())
    {
        return false;
    }

    auto cityRoom = data.get("cityRoom", defaultval);
    if (!cityRoom.isObject())
    {
        return false;
    }

    *has_next_page = !!GetInt32FromJsonValue(cityRoom, "hasNextPage");
    *online_number = GetInt32FromJsonValue(cityRoom, "onlineNum");

    auto singer_list = cityRoom.get("list", defaultval);
    if (!singer_list.isArray())
    {
        return false;
    }

    std::vector<RankSingerInfo> rank_singer_info_vector;
    for (const auto& singer_info_obj : singer_list)
    {
        auto members = singer_info_obj.getMemberNames();
        RankSingerInfo singer_info;
        for (const auto& member : members)
        {
            if (member.compare("activityPic") == 0)
            {
                singer_info.activityPic = singer_info_obj.get(member, defaultval).asString();
            }
            else if (member.compare("baiduCode") == 0)
            {
                singer_info.baiduCode = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("cityName") == 0)
            {
                singer_info.cityName = singer_info_obj.get(member, defaultval).asString();
            }
            else if (member.compare("company") == 0)
            {
                singer_info.company = singer_info_obj.get(member, defaultval).asString();
            }
            else if (member.compare("imgPath") == 0)
            {
                singer_info.imgPath = singer_info_obj.get(member, defaultval).asString();
            }
            else if (member.compare("isOriginal") == 0)
            {
                singer_info.isOriginal = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("kugouId") == 0)
            {
                singer_info.kugouId = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("labelName") == 0)
            {
                singer_info.labelName = singer_info_obj.get(member, defaultval).asString();
            }
            else if (member.compare("lastOnlineTime") == 0)
            {
                singer_info.lastOnlineTime = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("liveTitle") == 0)
            {
                singer_info.liveTitle = singer_info_obj.get(member, defaultval).asString();
            }
            else if (member.compare("nickName") == 0)
            {
                singer_info.nickName = singer_info_obj.get(member, defaultval).asString();
            }
            else if (member.compare("roomId") == 0)
            {
                singer_info.roomId = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("starLevel") == 0)
            {
                singer_info.starLevel = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("status") == 0)
            {
                singer_info.status = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("userId") == 0)
            {
                singer_info.userId = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("viewerNum") == 0)
            {
                singer_info.viewerNum = GetInt32FromJsonValue(singer_info_obj, member);
            }
        }
        rank_singer_info_vector.push_back(singer_info);
    }
    *rank_singer_infos = std::move(rank_singer_info_vector);
    return true;
}

bool PhoneRank::GetCityInfos(std::map<std::string, std::vector<CityInfo>>* province_citys) const
{
    std::map<std::string, std::string> param_map;
    param_map["platform"] = base::UintToString(5);
    param_map["version"] = base::UintToString(6582);
    std::string sign = GetSignFromMap(param_map);

    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = "http://mo.fanxing.kugou.com/mfx/logic/cdn/config/getProvinceCityList";
    request.queries = param_map;
    request.queries["sign"] = sign;

    // 数据分析
    CurlWrapper curl_wrapper;
    HttpResponse http_reponse;
    if (!curl_wrapper.Execute(request, &http_reponse))
        return false;

    std::string content;
    content.assign(http_reponse.content.begin(),
        http_reponse.content.end());

    Json::Reader reader;
    Json::Value root(Json::objectValue);
    if (!reader.parse(content, root, false))
        return false;

    uint32 code = GetInt32FromJsonValue(root, "code");
    if (code != 0)
        return false;

    Json::Value defaultval(Json::objectValue);
    auto data = root.get("data", defaultval);
    if (!data.isArray())
    {
        return false;
    }

    std::map<std::string, std::vector<CityInfo>> temp_map;
    for (const auto& province : data)
    {
        uint32 province_id = GetInt32FromJsonValue(province, "areaId");
        std::string province_name = province.get("areaName", "").asString();
        auto cityList = province.get("cityList", defaultval);
        if (!cityList.isArray())
        {
            return false;
        }
        std::vector<CityInfo> city_infos;
        for (const auto& city_obj : cityList)
        {
            auto members = city_obj.getMemberNames();
            CityInfo city_info;
            city_info.area_name = province_name;
            for (const auto& member : members)
            {
                if (member.compare("areaId") == 0)
                {
                    city_info.area_id = GetInt32FromJsonValue(city_obj, member);
                    assert(province_id == city_info.area_id);
                }
                else if (member.compare("cityCode") == 0)
                {
                    city_info.city_code = GetInt32FromJsonValue(city_obj, member);
                }
                else if (member.compare("cityName") == 0)
                {
                    city_info.city_name = city_obj.get(member, defaultval).asString();
                }
                else if (member.compare("fxCityId") == 0)
                {
                    city_info.fx_city_id = GetInt32FromJsonValue(city_obj, member);
                }
            }
            city_infos.push_back(city_info);
        }
        temp_map[province_name] = city_infos;
    }

    *province_citys = temp_map;
    return true;
}

bool PhoneRank::GetRankSingerListByCity(const CityInfo& city_info, 
    std::vector<RankSingerInfo>* rank_singer_infos) const
{
    //QueryCityRankParam query_city_rank_param;
    //query_city_rank_param.baidu_code = 257;
    //query_city_rank_param.city_name = base::WideToUTF8(L"广州市");
    //query_city_rank_param.page_size = 80;
    //query_city_rank_param.platform = 5; //6是ios,5是android
    //query_city_rank_param.version = 6582; // ios使用版本号是3059, android使用版本号是6582;

    QueryCityRankParam query_city_rank_param;
    query_city_rank_param.baidu_code = city_info.city_code;
    query_city_rank_param.city_name = city_info.city_name;
    query_city_rank_param.page_size = 80;
    query_city_rank_param.platform = 6; //6是ios,5是android
    query_city_rank_param.version = 3059; // ios使用版本号是3059, android使用版本号是6582;

    std::vector<RankSingerInfo> rank_singer_infos_part;
    bool has_next_page = true;
    uint32 page_number = 1;
    uint32 online_number = 0;
    while (has_next_page)
    {
        rank_singer_infos_part.clear();
        query_city_rank_param.page_number = page_number;
        std::map<std::string, std::string> param_map;
        param_map["baiduCode"] = base::UintToString(query_city_rank_param.baidu_code);
        param_map["cityName"] = query_city_rank_param.city_name;
        param_map["page"] = base::UintToString(query_city_rank_param.page_number);
        param_map["pageSize"] = base::UintToString(query_city_rank_param.page_size);
        param_map["platform"] = base::UintToString(query_city_rank_param.platform);
        param_map["version"] = base::UintToString(query_city_rank_param.version);
        std::string sign = GetSignFromMap(param_map);
        query_city_rank_param.sign = sign;

        if (!GetSinglePageDataByCity(query_city_rank_param, &rank_singer_infos_part,
            &has_next_page, &online_number))
        {
            return false;
        }
        rank_singer_infos->insert(rank_singer_infos->end(),
            rank_singer_infos_part.begin(), rank_singer_infos_part.end());

        page_number++;
    }

    return true;
}

bool PhoneRank::GetEnterRoomInfoByRoomId(uint32 roomid,
    NormalRoomInfo* normal_room_info) const
{
    std::map<std::string, std::string> param_map;
    param_map["platform"] = base::UintToString(5);
    param_map["version"] = base::UintToString(6582);
    param_map["roomId"] = base::UintToString(roomid);
    param_map["roomType"] = "0";// 未明确这个值的意义
    std::string sign = GetSignFromMap(param_map);

    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = "http://mo.fanxing.kugou.com/mfx/cdn/room/getEnterRoomInfo";
    request.queries = param_map;
    request.queries["sign"] = sign;

    // 数据分析
    CurlWrapper curl_wrapper;
    HttpResponse http_reponse;
    if (!curl_wrapper.Execute(request, &http_reponse))
        return false;

    std::string content;
    content.assign(http_reponse.content.begin(),
        http_reponse.content.end());

    Json::Reader reader;
    Json::Value root(Json::objectValue);
    if (!reader.parse(content, root, false))
        return false;

    uint32 code = GetInt32FromJsonValue(root, "code");
    if (code != 0)
        return false;

    Json::Value defaultval(Json::objectValue);
    auto data = root.get("data", defaultval);
    if (!data.isObject())
    {
        return false;
    }

    auto normalRoomInfo = data.get("normalRoomInfo", defaultval);
    if (!normalRoomInfo.isObject())
    {
        return false;
    }

    auto members = normalRoomInfo.getMemberNames();
    for (const auto& member : members)
    {
        if (member.compare("kugouId") == 0)
        {
            normal_room_info->kugou_id = GetInt32FromJsonValue(normalRoomInfo, member);
        }
        else if (member.compare("userId") == 0)
        {
            normal_room_info->fanxing_id = GetInt32FromJsonValue(normalRoomInfo, member);
        }
        //else if (member.compare("type") == 0)
        //{
        //    normal_room_info->type_ = GetInt32FromJsonValue(normalRoomInfo, member);
        //}
    }

    return true;
}

bool PhoneRank::GetStarCardByKugouId(uint32 kugouid,
    StarCard* star_card) const
{
    std::map<std::string, std::string> param_map;
    param_map["platform"] = base::UintToString(5);
    param_map["version"] = base::UintToString(6582);
    param_map["kugouId"] = base::UintToString(kugouid);
    std::string sign = GetSignFromMap(param_map);

    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = "http://mo.fanxing.kugou.com/mfx/cdn/room/getStarCard";
    request.queries = param_map;
    request.queries["sign"] = sign;

    // 数据分析
    CurlWrapper curl_wrapper;
    HttpResponse http_reponse;
    if (!curl_wrapper.Execute(request, &http_reponse))
        return false;

    std::string content;
    content.assign(http_reponse.content.begin(),
        http_reponse.content.end());

    Json::Reader reader;
    Json::Value root(Json::objectValue);
    if (!reader.parse(content, root, false))
        return false;

    uint32 code = GetInt32FromJsonValue(root, "code");
    if (code != 0)
        return false;

    Json::Value defaultval(Json::objectValue);
    auto data = root.get("data", defaultval);
    if (!data.isObject())
    {
        return false;
    }

    auto members = data.getMemberNames();
    for (const auto& member : members)
    {
        if (member.compare("location") == 0)
        {
            star_card->location = data.get("location", "").asString();
        }
    }

    return true;
}

