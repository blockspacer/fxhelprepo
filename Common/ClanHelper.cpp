#include "ClanHelper.h"
#include <string>

#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/json/json.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"

ClanHelper::ClanHelper(int clanid)
	: clan_id_(clanid)
	,curlWrapper_(new CurlWrapper)
{
}

ClanHelper::~ClanHelper()
{
}

bool ClanHelper::GetAllClanSingers(std::vector<ClanSingerInfo>* clan_singers)
{
	if (!clan_singers)
	{
		return false;
	}

	std::vector<ClanSingerInfo> singers;
	uint32 total_count = 0;
	uint32 total_page = 0;
	bool result = GetPageClanSinger(1,&singers,&total_count,&total_page);
	clan_singers->insert(clan_singers->end(), singers.begin(), singers.end());
	
	for (uint32 page_num = 2; page_num <= total_page; page_num++)
	{
		singers.clear();
		result &= GetPageClanSinger(page_num,&singers,&total_count,&total_page);
		clan_singers->insert(clan_singers->end(), singers.begin(), singers.end());
	}

	return result;
}

bool ClanHelper::GetPageClanSinger(int page,std::vector<ClanSingerInfo>* clan_singers,
	uint32* total_count,uint32* total_page)
{
	std::string url = "http://visitor.fanxing.kugou.com/VServices/Clan.ClanServices.getClanStarListPaging/";
	url += base::IntToString(clan_id_) + "-" + base::IntToString(page) + "-12";
	HttpRequest request;
	request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
	request.url = url;
	request.referer = "http://fanxing.kugou.com/index.php?action=clan&id=" + base::IntToString(clan_id_);

	HttpResponse response;
	if (!curlWrapper_->Execute(request,&response))
	{
		return false;
	}

	std::string responsedata;
	responsedata.assign(response.content.begin(),response.content.end());
	if (responsedata.empty())
		return  false;

	std::string jsondata = PickJson(responsedata);

	Json::Reader reader;
	Json::Value rootdata(Json::objectValue);
	if (!reader.parse(jsondata,rootdata,false))
	{
		return false;
	}

	std::string errormsg;
	// 有必要检测status的值
	uint32 status = GetInt32FromJsonValue(rootdata,"status");
	if (status != 1)
	{
		std::string errorMsg = rootdata.get("errorcode","").asString();
		errormsg = errorMsg;
		std::wstring werrorMsg = base::UTF8ToWide(errorMsg);
		return false;
	}
	uint32 servertime = GetInt32FromJsonValue(rootdata,"servertime");
	Json::Value dataObject(Json::objectValue);
	dataObject = rootdata.get(std::string("data"),dataObject);
	if (dataObject.empty())
	{
		errormsg = "json parse error";
		return false;
	}

	*total_count = GetInt32FromJsonValue(dataObject,"count");
	*total_page = GetInt32FromJsonValue(dataObject,"totalPage");

	auto singer_list = dataObject.get("list",Json::Value());
	if (!singer_list.isArray())
	{
		return false;
	}

	for (const auto& item : singer_list)
	{
		ClanSingerInfo singer_info;
		singer_info.img_path = item.get("imgPath","").asString();
		singer_info.user_logo = item.get("userLogo","").asString();
		singer_info.kugou_id = item.get("kugouId", 0).asInt();
		singer_info.live_status = item.get("liveStatus",0).asInt();
		singer_info.nickname = item.get("nickName","").asString();
		singer_info.roomid = item.get("roomId",0).asInt();
		singer_info.star_level = item.get("starLevel",0).asInt();
		singer_info.fanxing_id = item.get("userId",0).asInt();
		singer_info.viewer_num = item.get("viewerNum",0).asInt();
		clan_singers->push_back(singer_info);
	}

	return true;
}
