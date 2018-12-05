#include "stdafx.h"
#include "PhoneRank.h"

#include <iostream>
#include <xutility>
#include <iterator>
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/time/time.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "CitySingersManager.h"
#include "common.h"

PhoneRank::PhoneRank()
    : break_all_request_(false)
    ,city_manager_(new CitySingersManager())
{
}

PhoneRank::~PhoneRank()
{
}

bool PhoneRank::Initialize(base::SingleThreadTaskRunner* runner, 
    const base::Callback<void(uint32, bool, const RowData&)>& singer_info_callback,
    const base::Callback<void(const std::wstring&)>& message_callback)
{
    singer_info_callback_ = singer_info_callback;
    message_callback_ = message_callback;

    runner_ = runner;
    city_manager_->Initialize(runner);

    return true;
}

void PhoneRank::Finalize()
{
    BreakRequest();
    runner_->PostTask(FROM_HERE,
        base::Bind(base::IgnoreResult(&PhoneRank::DoStop), base::Unretained(this)));
}

void PhoneRank::BreakRequest()
{
    break_all_request_ = true;
}

void PhoneRank::DoStop()
{
    DCHECK(runner_->RunsTasksOnCurrentThread());
    // �ϵ���������
}

void PhoneRank::InitCheckGroupSingers(bool beauty, bool newsinger)
{
    beauty_ = beauty;
    newsinger_ = newsinger;
}

void PhoneRank::RetriveSingerRankResult(uint32 roomid)
{
    if (!runner_->RunsTasksOnCurrentThread())
    {
        runner_->PostTask(FROM_HERE,
            base::Bind(base::IgnoreResult(&PhoneRank::RetriveSingerRankResult),
                base::Unretained(this), roomid));
        return;
    }

    DCHECK(runner_->RunsTasksOnCurrentThread());

    // �Ȼ�ȡ����������Ϣ

    // ��ȡͬ������������ó��е���Ϣ���Ǳ��θ��¹��������
	RankSingerInfo rank_singer_info;
	uint32 city_rank_id = 0;
	uint32 city_online_num = 0;
	uint32 city_all_count = 0;
	if (!DoGetCityRankInfos(roomid, &rank_singer_info, &city_rank_id,
		&city_online_num, &city_all_count))
	{
		
	}

    // ��ȡ������������������Ů����ȫվ����
	bool find_result = false;
    uint32 new_rank = 0;
    uint32 new_all = new_singers_rank_.size();
    for (auto& singerinfo : new_singers_rank_)
    {
        new_rank++;
        if (singerinfo.roomId == roomid)
        {
            rank_singer_info = singerinfo;
			find_result = true;
            break;
        }
    }

	if (!find_result)
		new_rank = 0;

    // ��ȡŮ��������������������¦ȫվ����
    uint32 beauty_rank = 0;
    uint32 beauty_all = beautiful_singers_rank_.size();
    for (auto& singerinfo : beautiful_singers_rank_)
    {
        beauty_rank++;
        if (singerinfo.roomId == roomid)
        {
            rank_singer_info = singerinfo;
			find_result = true;
            break;
        }
    }

	if (!find_result)
		beauty_rank = 0;

    RowData row_data;
    row_data.push_back(base::UTF8ToWide(rank_singer_info.nickName));
    row_data.push_back(base::UTF8ToWide(rank_singer_info.cityName));
	row_data.push_back(base::UintToString16(city_rank_id) + L"/" + 
		base::UintToString16(city_online_num) +L"/" + 
		base::UintToString16(city_all_count));
    row_data.push_back(base::UTF8ToWide(rank_singer_info.labelName));
    row_data.push_back(base::UintToString16(rank_singer_info.viewerNum));
    row_data.push_back(base::UintToString16(new_rank) + L"/" + base::UintToString16(new_all));
    row_data.push_back(base::UintToString16(beauty_rank) + L"/" + base::UintToString16(beauty_all));

	bool result = !rank_singer_info.nickName.empty();
	singer_info_callback_.Run(roomid, result, row_data);
}

bool PhoneRank::InitNewSingerRankInfos()
{
    if (!runner_->RunsTasksOnCurrentThread())
    {
        runner_->PostTask(FROM_HERE,
            base::Bind(base::IgnoreResult(&PhoneRank::InitNewSingerRankInfos), 
            base::Unretained(this)));
        return true;
    }

    DCHECK(runner_->RunsTasksOnCurrentThread());
    new_singers_rank_.clear();

    uint32 doubleLiveFirst = 0;
    uint32 mobileFromIndex = 0;
    uint32 pageSize = 80;
    uint32 pcFromIndex = 0;
    uint32 platform = kPlatform;
    uint32 type_t = 3;
    uint32 version = kVersion;
    
    bool has_next_page = true;
    bool all_online = true;
    uint32 page_num = 0;
    while (has_next_page && all_online)
    {
        std::map<std::string, std::string> param_map;
        param_map["doubleLiveFirst"] = base::UintToString(doubleLiveFirst);
        param_map["mobileFromIndex"] = base::UintToString(mobileFromIndex);
        param_map["pageSize"] = base::UintToString(pageSize);
        param_map["pcFromIndex"] = base::UintToString(pcFromIndex);
        param_map["platform"] = base::UintToString(platform);
        param_map["type"] = base::UintToString(type_t);
        param_map["version"] = base::UintToString(version);
        std::string sign = GetSignFromMap(param_map);
        param_map["sign"] = sign;

        std::string url = "http://gzacshow.kugou.com/mfanxing-home/cdn/room/live_list_by_group/v02";

        std::vector<RankSingerInfo> rank_singer_infos;
        if (!GetSinglePageData(url, param_map, &rank_singer_infos,
            &has_next_page, &all_online, &mobileFromIndex,
            &pcFromIndex))
        {
            std::wstring message = L"��ȡ����������[ " + base::UintToString16(++page_num) + L" ]ҳ ʧ��";
            message_callback_.Run(message);
            message = L"������������XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
            return false;
        }

        std::wstring message = L"��ȡ����������[ " + base::UintToString16(++page_num) + L" ]ҳ �ɹ�";
        message_callback_.Run(message);
        new_singers_rank_.insert(new_singers_rank_.end(),
            rank_singer_infos.begin(), rank_singer_infos.end());
    }

    std::wstring message = L"��ȡ�����������,һ�� " + base::UintToString16(new_singers_rank_.size()) +L" ������";
    message_callback_.Run(message);

    return true;
}

bool PhoneRank::GetNewSingerRankByRoomid(uint32 roomid, uint32* rank, uint32* all) const
{
    uint32 count = 0;
    *all = new_singers_rank_.size();
    for (auto& singerinfo : new_singers_rank_)
    {
        count++;
        if (singerinfo.roomId == roomid)
        {
            *rank = count;
            RowData row_data;
            row_data.push_back(base::UTF8ToWide(singerinfo.nickName));
            row_data.push_back(base::UTF8ToWide(singerinfo.cityName));
            row_data.push_back(base::UTF8ToWide(singerinfo.labelName));
            row_data.push_back(base::UintToString16(singerinfo.viewerNum));
            std::wstring str_rank = base::UintToString16(count) + L"/" + base::UintToString16(*all);
            row_data.push_back(str_rank);
            singer_info_callback_.Run(roomid, true, row_data);
            return true;
        }
    }
    return false;
}

bool PhoneRank::InitBeautifulSingerRankInfos()
{
    if (!runner_->RunsTasksOnCurrentThread())
    {
        runner_->PostTask(FROM_HERE,
            base::Bind(base::IgnoreResult(&PhoneRank::InitBeautifulSingerRankInfos),
            base::Unretained(this)));
        return true;
    }

    DCHECK(runner_->RunsTasksOnCurrentThread());
    beautiful_singers_rank_.clear();

    uint32 doubleLiveFirst = 0;
    uint32 mobileFromIndex = 0;
    uint32 pageSize = 80;
    uint32 pcFromIndex = 0;
    uint32 platform = kPlatform;
    uint32 cId = 20;
    uint32 version = kVersion;

    bool has_next_page = true;
    bool all_online = true;
    uint32 page_num = 0;
    while (has_next_page && all_online)
    {
        std::map<std::string, std::string> param_map;
        param_map["doubleLiveFirst"] = base::UintToString(doubleLiveFirst);
        param_map["mobileFromIndex"] = base::UintToString(mobileFromIndex);
        param_map["pageSize"] = base::UintToString(pageSize);
        param_map["pcFromIndex"] = base::UintToString(pcFromIndex);
        param_map["platform"] = base::UintToString(platform);
        param_map["cId"] = base::UintToString(cId);
        param_map["version"] = base::UintToString(version);
        std::string sign = GetSignFromMap(param_map);
        param_map["sign"] = sign;

        std::string url = "http://gzacshow.kugou.com/mfanxing-home/cdn/room/live_list_by_group_v3";

        std::vector<RankSingerInfo> rank_singer_infos;
        if (!GetSinglePageData(url, param_map, &rank_singer_infos,
            &has_next_page, &all_online, &mobileFromIndex,
            &pcFromIndex))
        {
            std::wstring message = L"��ȡŮ��������[ " + base::UintToString16(++page_num) + L" ]ҳ ʧ��";
            message_callback_.Run(message);
            message = L"������������XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
            return false;
        }

        std::wstring message = L"��ȡŮ��������[ " + base::UintToString16(++page_num) + L" ]ҳ �ɹ�";
        message_callback_.Run(message);
        beautiful_singers_rank_.insert(beautiful_singers_rank_.end(),
            rank_singer_infos.begin(), rank_singer_infos.end());
    }

    std::wstring message = L"��ȡŮ���������,һ�� " + base::UintToString16(beautiful_singers_rank_.size()) + L" ������";
    message_callback_.Run(message);

    return true;
}

bool PhoneRank::GetBeautifulSingerRankByRoomid(uint32 roomid, uint32* rank, uint32* all) const
{
    // FIX ME: �ж��̷߳��ʱ����ķ��գ���ʱ����
    // DCHECK(runner_->RunsTasksOnCurrentThread());
    uint32 count = 0;
    *all = beautiful_singers_rank_.size();
    for (auto& singerinfo : beautiful_singers_rank_)
    {
        count++;
        if (singerinfo.roomId == roomid)
        {
            *rank = count;
            return true;
        }

    }
    return false;
}

bool PhoneRank::GetCityRankInfos(uint32 roomid)
{
    if (!runner_->RunsTasksOnCurrentThread())
    {
        runner_->PostTask(FROM_HERE,
            base::Bind(base::IgnoreResult(&PhoneRank::GetCityRankInfos),
            base::Unretained(this), roomid));
        return true;
    }

    DCHECK(runner_->RunsTasksOnCurrentThread());

	RankSingerInfo singer_info;
	uint32 rank_id = 0;
	uint32 online_num = 0;
	uint32 all_count = 0;
	if (!DoGetCityRankInfos(roomid, &singer_info,&rank_id,&online_num, &all_count))
	{
		return false;
	}
	
    std::wstring msg = L"����/����/" + base::UTF8ToWide(singer_info.cityName) +L": ";
    msg += base::UintToString16(rank_id) + L"/";
    msg += base::UintToString16(online_num) + L"/";
    msg += base::UintToString16(all_count);
    message_callback_.Run(msg);

    if (!rank_id)
    {
        message_callback_.Run(L"�ڶ�λ�����޷��ҵ�����");
        return false;
    }

    if (!singer_info.status)
        message_callback_.Run(L"����Ϊδ����״̬");

    return true;
}

bool PhoneRank::DoGetCityRankInfos(uint32 roomid, RankSingerInfo* singer_info,
	uint32* rank_id, uint32* online_num, uint32* all_count)
{
	DCHECK(runner_->RunsTasksOnCurrentThread());
	NormalRoomInfo normal_room_info;
	if (!GetEnterRoomInfoByRoomId(roomid, &normal_room_info))
	{
		message_callback_.Run(L"�޷���ȡ��Ӧ�����������Ϣ");
		return false;
	}

	StarCard star_card;
	if (!GetStarCardByKugouId(normal_room_info.kugou_id, &star_card))
	{
		message_callback_.Run(L"�޷���ȡ����λ����Ϣ");
		return false;
	}

	std::string city_name = star_card.location;

	CityInfo city_info;
	if (!city_manager_->FindCity(city_name, &city_info))
	{
		std::wstring msg = L"�ڳ����б��޷���ȡ���У�" + base::UTF8ToWide(star_card.location);
		message_callback_.Run(msg);
		return false;
	}

	std::vector<RankSingerInfo> rank_singer_infos;
	if (!city_manager_->GetRankSingerListByCity(city_info, &rank_singer_infos))
	{
		message_callback_.Run(L"�޷���ȡ�������ڳ��е������б�");
		return false;
	}

	// �鿴ĳ������id���ֻ��Ƽ�����
	uint32 singer_fanxing_id = normal_room_info.fanxing_id;
	uint32 singer_kugou_id = normal_room_info.kugou_id;
	//uint32 rank_id = 0;
	uint32 count = 0;
	//uint32 online_num = 0;

	for (const auto& it : rank_singer_infos)
	{
		count++;
		if (it.userId == singer_fanxing_id)
		{
			*rank_id = count;
			*singer_info = it;
		}

		if (it.userId == singer_kugou_id)
		{
			*rank_id = count;
			*singer_info = it;
		}

		if (it.status) // status=1,����ֱ�� status=2,�ֻ�ֱ��
			(*online_num)++;
	}
	*all_count = rank_singer_infos.size();
	return true;
}

bool PhoneRank::GetEnterRoomInfoByRoomId(uint32 roomid,
    NormalRoomInfo* normal_room_info) const
{
    DCHECK(runner_->RunsTasksOnCurrentThread());
    std::map<std::string, std::string> param_map;
    param_map["platform"] = base::UintToString(kPlatform);
    param_map["version"] = base::UintToString(kVersion);
    param_map["roomId"] = base::UintToString(roomid);
    param_map["roomType"] = "0";// δ��ȷ���ֵ������
    std::string sign = GetSignFromMap(param_map);

    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = "http://mo.fanxing.kugou.com/mfx/cdn/room/getEnterRoomInfo";
    request.queries = param_map;
    request.queries["sign"] = sign;

    // ���ݷ���
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

    auto normalRoomInfo = data.get("normalRoomInfo", defaultval);
    if (!normalRoomInfo.isObject())
        return false;

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
    DCHECK(runner_->RunsTasksOnCurrentThread());
    std::map<std::string, std::string> param_map;
    param_map["platform"] = base::UintToString(6);
    param_map["version"] = base::UintToString(3302);
    param_map["kugouId"] = base::UintToString(kugouid);
    std::string sign = GetSignFromMap(param_map);

    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = "http://mo.fanxing.kugou.com/mfx/cdn/room/getStarCard";
    request.queries = param_map;
    request.queries["sign"] = sign;

    // ���ݷ���
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

    auto members = data.getMemberNames();
    for (const auto& member : members)
    {
        if (member.compare("location") == 0)
            star_card->location = data.get("location", "").asString();
    }

    return true;
}

bool PhoneRank::GetSinglePageData(
    const std::string& url,
    const std::map<std::string, std::string>& param_map,
    std::vector<RankSingerInfo>* rank_singer_infos,
    bool* has_next_page, bool* all_online, uint32* mobileFromIndex,
    uint32* pcFromIndex) const
{
    DCHECK(runner_->RunsTasksOnCurrentThread());
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.queries = param_map;

    request.useragent = "�ṷֱ�� 3.9.1 rv:3.9.1.0 (iPhone; iOS 10.3.3; zh_CN)";

    // ���ݷ���
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

    *has_next_page = !!GetInt32FromJsonValue(data, "hasNextPage");
    *mobileFromIndex = GetInt32FromJsonValue(data, "mobileFromIndex");
    *pcFromIndex = GetInt32FromJsonValue(data, "pcFromIndex");

    auto singer_list = data.get("list", defaultval);
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
            else if (member.compare("lastLiveTime") == 0)
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
                if (singer_info.status == 0)
                    *all_online = false;
            }
            else if (member.compare("userId") == 0)
            {
                singer_info.userId = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("viewerNum") == 0)
            {
                singer_info.viewerNum = GetInt32FromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("rankScore") == 0)
            {
                singer_info.rankScore = GetDoubleFromJsonValue(singer_info_obj, member);
            }
            else if (member.compare("score") == 0)
            {
                singer_info.score = GetDoubleFromJsonValue(singer_info_obj, member);
            }
        }
        rank_singer_info_vector.push_back(singer_info);
    }
    *rank_singer_infos = std::move(rank_singer_info_vector);
    return true;
}

