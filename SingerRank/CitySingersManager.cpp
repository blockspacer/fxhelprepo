#include "stdafx.h"
#include "CitySingersManager.h"
#include "common.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"

CitySingersManager::CitySingersManager()
{
}

CitySingersManager::~CitySingersManager()
{
}

bool CitySingersManager::Initialize(base::SingleThreadTaskRunner* runner)
{
    runner_ = runner;

    runner_->PostTask(FROM_HERE,
        base::Bind(base::IgnoreResult(&CitySingersManager::GetCityInfos), base::Unretained(this)));

    return true;;
}

void CitySingersManager::Finalize()
{
    return;
}

bool CitySingersManager::GetCityInfos()
{
    DCHECK(runner_->RunsTasksOnCurrentThread());
    std::map<std::string, std::string> param_map;
    param_map["platform"] = base::UintToString(6);
    param_map["version"] = base::UintToString(3910);
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
        return false;

    std::map<std::string, std::vector<CityInfo>> temp_map;
    for (const auto& province : data)
    {
        uint32 province_id = GetInt32FromJsonValue(province, "areaId");
        std::string province_name = province.get("areaName", "").asString();
        auto cityList = province.get("cityList", defaultval);
        if (!cityList.isArray())
            return false;

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
                else if (member.compare("gaodeCode") == 0)
                {
                    city_info.gaode_code = city_obj.get(member, defaultval).asString();
                }
            }
            city_infos.push_back(city_info);
        }
        temp_map[province_name] = city_infos;
    }

    province_citys_ = temp_map;
    return true;
}

bool CitySingersManager::FindCity(const std::string& city_name, CityInfo* city_info)
{
    DCHECK(runner_->RunsTasksOnCurrentThread());
    std::wstring w_city_name = base::UTF8ToWide(city_name);
    if (w_city_name.size() > 2)
        w_city_name = w_city_name.substr(0, 2);

    for (const auto& province : province_citys_)
    {
        for (const auto& it : province.second)
        {
            std::wstring temp = base::UTF8ToWide(it.city_name);
            if (temp.size() > 2)
                temp = temp.substr(0, 2);
            if (temp.compare(w_city_name) == 0)
            {
                *city_info = it;
                return true;
            }
        }
    }

    return false;
}

void CitySingersManager::ClearCityRankCache()
{
    DCHECK(runner_->RunsTasksOnCurrentThread());
    city_singers_rank_.clear();
}

bool CitySingersManager::GetRankSingerListByCity(
    const CityInfo& city_info, std::vector<RankSingerInfo>* rank_singer_infos) const
{
    DCHECK(runner_->RunsTasksOnCurrentThread());

    auto result = city_singers_rank_.find(city_info.city_name);
    if (result != city_singers_rank_.end())
    {
        *rank_singer_infos = result->second;
        return true;
    }

    std::vector<RankSingerInfo> rank_singer_infos_part;
    bool has_next_page = true;
    uint32 page_number = 1;
    uint32 online_number = 0;
    while (has_next_page)
    {
        rank_singer_infos_part.clear();
        std::map<std::string, std::string> param_map;
        param_map["gaodeCode"] = city_info.gaode_code;
        param_map["cityName"] = city_info.city_name;
        param_map["page"] = base::UintToString(page_number);
        param_map["pageSize"] = base::UintToString(kPagesize);
        param_map["platform"] = base::UintToString(kPlatform);
        param_map["version"] = base::UintToString(kVersion);
        std::string sign = GetSignFromMap(param_map);
        param_map["sign"] = sign;

        if (!GetSinglePageDataByCity(param_map, &rank_singer_infos_part,
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


bool CitySingersManager::GetSinglePageDataByCity(
    const std::map<std::string, std::string>& query_city_rank_param,
    std::vector<RankSingerInfo>* rank_singer_infos,
    bool* has_next_page, uint32* online_number) const
{
    DCHECK(runner_->RunsTasksOnCurrentThread());
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = "http://mo.fanxing.kugou.com/mfx/rank/cdn/room/cityLbs_v2/";
    request.queries = query_city_rank_param;
    request.useragent = base::WideToUTF8(kUseragent);

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
        return false;

    auto cityRoom = data.get("cityRoom", defaultval);
    if (!cityRoom.isObject())
        return false;

    *has_next_page = !!GetInt32FromJsonValue(cityRoom, "hasNextPage");
    *online_number = GetInt32FromJsonValue(cityRoom, "onlineNum");

    auto singer_list = cityRoom.get("list", defaultval);
    if (!singer_list.isArray())
        return false;

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
                singer_info.lastLiveTime = GetInt32FromJsonValue(singer_info_obj, member);
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
