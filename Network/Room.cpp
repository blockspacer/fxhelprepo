#include "Room.h"
#include "CurlWrapper.h"
#include "MessageNotifyManager.h"
#include "CookiesHelper.h"
#include "EncodeHelper.h"

#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/json/json.h"

namespace
{
    bool MakePostdata(const std::map<std::string, std::string>& postmap,
        std::vector<uint8>* postdata)
    {
        if (postmap.empty())
            return false;

        std::string temp;
        bool first = true;
        for (const auto& param : postmap)
        {
            if (first)
                first = false;
            else
                temp += "&";

            temp += param.first + "=" + UrlEncode(param.second);
        }
        postdata->assign(temp.begin(), temp.end());
        return true;
    }


    //计算sing的方案
    //cityName使用中文编码，不要转换成urlencode
    //1. 所有参数以key - value方式放进map中
    //2. 按key升序排列出key1 = value1&key2 = value2的格式的字符串str1
    //3. 在str1后面紧跟$_fan_xing_$串，然后计算md5值
    //4. 取md5的第8位到24位，一共是16位数据。

    std::string GetSignFromMap(const std::map<std::string, std::string>& param_map)
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
Room::Room(uint32 roomid)
    :roomid_(roomid)
    , curlWrapper_(new CurlWrapper)
    , messageNotifyManager_(new MessageNotifyManager)
    , cookiesHelper_(new CookiesHelper)
{
    
}

Room::~Room()
{

}

bool Room::Initialize(const scoped_refptr<base::TaskRunner>& runner)
{
    return messageNotifyManager_->Initialize(runner);
}

void Room::Finalize()
{
    messageNotifyManager_->Finalize();
}

void Room::SetIpProxy(const IpProxy& ipproxy)
{
    ipproxy_ = ipproxy;
}

void Room::SetRoomServerIp(const std::string& serverip)
{
    messageNotifyManager_->SetServerIp(serverip);
}

void Room::SetTcpManager(TcpClientController* tcpManager)
{
    messageNotifyManager_->SetTcpManager(tcpManager);
}

bool Room::EnterForOperation(const std::string& cookies, 
    const std::string& usertoken, uint32 userid, uint32* singer_clanid,
    const base::Callback<void()>& conn_break_callback)
{
    uint32 richlevel = 0;
    uint32 ismaster = 0;
    uint32 singerid = 0;
    std::string key = "";
    std::string ext = "";

    if (!OpenRoom(cookies))
        return false;

    if (!GetStarInfo(cookies))
        return false;

    if (!GetStarKugouId(cookies))
        return false;

    // 年度大奖需求，不需要连接房间
    //GetStarGuard();

    if (!EnterRoom(cookies, userid, usertoken))
        return false;
    
    if (!ConnectToNotifyServer_(roomid_, userid, usertoken, conn_break_callback))
        return false;

    if (singer_clanid)
        *singer_clanid = clanid_;

    //if (nickname)
    //    *nickname = nickname_;

    return true;
}

bool Room::EnterForAlive(const std::string& cookies, const std::string& usertoken, uint32 userid,
    const base::Callback<void()>& conn_break_callback)
{

    if (!OpenRoom(cookies))
        return false;

    if (!ConnectToNotifyServer_(roomid_, userid, usertoken, conn_break_callback))
    {
        return false;
    }
    return true;
}

bool Room::Exit()
{
    messageNotifyManager_->Finalize();
    return true;
}

bool Room::GetGiftList(const std::string& cookies, std::string* content)
{
    std::string url = "http://visitor.fanxing.kugou.com/";
    url += "/VServices/GiftService.GiftService.getGiftList/";
    url += base::UintToString(roomid_);
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    if (response.content.empty())
    {
        assert(false);
        return false;
    }

    content->assign(response.content.begin(), response.content.end());

    return true;
}

// 主播繁星号, 礼物id, 数量, 房间号 _是时间
// GET /UServices/GiftService/GiftService/sendGift?d=1476689413506&args=["141023689","869",1,"1070190",false]&_=1476689413506 HTTP/1.1
bool Room::SendGift(const std::string& cookies, uint32 gift_id, uint32 gift_count,
                    std::string* errormsg)
{
    assert(singerid_);
    assert(roomid_);

    std::string url = "http://fanxing.kugou.com/";
    url += "/UServices/GiftService/GiftService/sendGift";
    HttpRequest request;
    request.url = url;
    std::string time_string = GetNowTimeString();
    request.queries["d"] = time_string;
    request.queries["args"] = "%5B%22" + base::UintToString(singerid_) +
        "%22%2C%22" + base::UintToString(gift_id) + "%22%2C" + base::UintToString(gift_count) +
        "%2C%22" + base::UintToString(roomid_) + "%22%2Cfalse%5D";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    if (response.content.empty())
    {
        assert(false);
        return false;
    }

    std::string responsedata(response.content.begin(), response.content.end());

    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(responsedata, rootdata, false))
    {
        assert(false);
        return false;
    }

    uint32 unixtime = GetInt32FromJsonValue(rootdata, "servertime");
    uint32 status = GetInt32FromJsonValue(rootdata, "status");
    uint32 errorno = GetInt32FromJsonValue(rootdata, "errorno");
    if (status != 1 || errorno!=0 )
    {
        *errormsg = rootdata.get("errorcode", "").asString();
        return false;
    }
    
    return true;
}

bool Room::SendStar(const std::string& cookies, uint32 roomid, uint32 count,
    std::string* errormsg)
{
    ///UServices/GiftService/StorageService/getStorageByUserId?args=[]&_=1476974477980
    std::string url = "http://fanxing.kugou.com/NServices/GiftStarService/GiftStarService/send";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://fanxing.kugou.com/" + base::UintToString(roomid);
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    queries["d"] = GetNowTimeString();
    queries["args"] = UrlEncode("[\"" + base::UintToString(singerid_) + "\"," + 
        base::UintToString(count) +",\"" + base::UintToString(roomid)+"\"]");
    queries["_"] = GetNowTimeString();

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    if (response.content.empty())
    {
        assert(false);
        return false;
    }

    std::string responsedata(response.content.begin(), response.content.end());

    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(responsedata, rootdata, false))
    {
        assert(false);
        return false;
    }

    uint32 unixtime = rootdata.get("servertime", 1476689208).asUInt();
    uint32 status = rootdata.get("status", 0).asUInt();
    std::string errorcode = rootdata.get("errorcode", "").asString();
    if (status != 1)
    {
        assert(false && L"送星星失败");
        *errormsg = errorcode;
        return false;
    }

    Json::Value jvdata(Json::ValueType::arrayValue);
    Json::Value data = rootdata.get(std::string("data"), jvdata);
    if (!data.isBool())
    {
        assert(false);
        return false;
    }

    return data.asBool();
}

bool Room::RealSingLike(const std::string& cookies, uint32 user_kugou_id, 
    const std::string& user_token,
    const std::wstring& song_name, std::string* errormsg)
{
    assert(singerid_);
    assert(roomid_);

    std::string url = "http://service.fanxing.com";
    url += "/singlike/realsinglike/like";
    HttpRequest request;
    request.url = url;
    request.cookies = cookies;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_POST;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    std::map<std::string, std::string> postmap;
    postmap["times"] = GetNowTimeString();
    postmap["appId"] = "1010";
    postmap["pid"] = "90";
    postmap["starKugouId"] = base::UintToString(star_kugou_id_); // kugouid, 不是繁星号
    postmap["fanKugouId"] = base::UintToString(user_kugou_id); // kugouid, 不是繁星号
    postmap["token"] = user_token;
    postmap["callback"] = "postCallback";
    postmap["songName"] = base::WideToUTF8(song_name);
    MakePostdata(postmap, &request.postdata);

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    if (response.content.empty())
    {
        assert(false);
        return false;
    }

    std::string responsedata(response.content.begin(), response.content.end());
    if (responsedata.find("success") == std::string::npos)
        return false;

    return true;
}

bool Room::OpenRoomAndGetViewerList(const std::string& cookies,
    std::vector<EnterRoomUserInfo>* enterRoomUserInfoList)
{
    if (!OpenRoom(cookies))
    {
        return false;
    }

    if (!GetViewerList(cookies, enterRoomUserInfoList))
    {
        return false;
    }

    return true;
}

bool Room::GetViewerList(const std::string& cookies,
    std::vector<EnterRoomUserInfo>* enterRoomUserInfoList)
{
    std::string url = "http://visitor.fanxing.kugou.com";
    url += "/VServices/RoomService.RoomService.getViewerList/";
    url += base::UintToString(roomid_) +"-" + base::UintToString(singerid_);
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    if (response.content.empty())
    {
        assert(false);
        return false;
    }

    std::string responsedata(response.content.begin(), response.content.end());
    std::string jsondata = PickJson(responsedata);

    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(jsondata, rootdata, false))
    {
        assert(false);
        return false;
    }

    uint32 unixtime = rootdata.get("servertime", 1461378689).asUInt();
    uint32 status = rootdata.get("status", 0).asUInt();
    if (status != 1)
    {
        assert(false);
        return false;
    }
    Json::Value jvdata(Json::ValueType::objectValue);
    Json::Value data = rootdata.get(std::string("data"), jvdata);
    if (data.isNull() || !data.isObject())
    {
        assert(false);
        return false;
    }
    
    Json::Value jvlist(Json::ValueType::objectValue);
    Json::Value list = data.get(std::string("list"), jvlist);
    if (!list.isArray())
    {
        assert(false);
        return false;
    }

    for (const auto& item : list)
    {
        EnterRoomUserInfo enterRoomUserInfo;
        enterRoomUserInfo.nickname = item.get("nickname", "").asString();
		enterRoomUserInfo.richlevel = GetInt32FromJsonValue(item, "richlevel");
		enterRoomUserInfo.userid = GetInt32FromJsonValue(item, "userid");
        enterRoomUserInfo.unixtime = unixtime;
		enterRoomUserInfo.roomid = roomid_;
        enterRoomUserInfoList->push_back(enterRoomUserInfo);
    }

    return true;
}

bool Room::OpenRoomAndGetConsumerList(const std::string& cookies,
    std::vector<ConsumerInfo>* consumer_infos, uint32* star_level)
{
    if (!OpenRoom(cookies))
        return false;

    if (!GetStarInfo(cookies))
        return false;
    
    if (clanid_) // 因为挖主播的特殊需要，不要找有公会的主播
        return false;

    // 因为挖主播的特殊需要，不要找观众列表，这里是省代码，乱写的
    //if (!GetConsumerList(cookies, consumer_infos))
    //    return false;
    *star_level = singer_star_level_;
    return true;
}

bool Room::GetConsumerList(const std::string& cookies,
    std::vector<ConsumerInfo>* consumer_infos)
{
    std::string url = "http://service.fanxing.kugou.com/fx-rank-service/rank/getThirtyDayRank.json";
    std::map<std::string, std::string> param_map;
    param_map["kugouId"] = base::UintToString(star_kugou_id_);
    param_map["_p"] = base::UintToString(6);
    param_map["_v"] = "3.3.0.2platform=6";
    param_map["version"] = "3302";// 未明确这个值的意义
    std::string sign = GetSignFromMap(param_map);
    param_map["sign"] = sign;

    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.queries = param_map;
    request.headers["User-Agent"] = "酷狗直播 3.3.0 rv:3.3.0.2 (iPhone; iPhone OS 8.4; zh_CN)";

    HttpResponse reponse;
    if (!curlWrapper_->Execute(request, &reponse))
    {
        return false;
    }

    std::string json(reponse.content.begin(), reponse.content.end());
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(json, rootdata, false))
    {
        return false;
    }

    uint32 code = GetInt32FromJsonValue(rootdata, "code");

    Json::Value defaultval(Json::objectValue);
    Json::Value data = rootdata.get("data", defaultval);
    Json::Value rank_vo_list = data.get("rankVOList", defaultval);
    if (!rank_vo_list.isArray())
        return false;

    for (const auto& user : rank_vo_list)
    {
        auto members = user.getMemberNames();
        ConsumerInfo consumer;
        consumer.room_id = roomid_;
        for (const auto& member : members)
        {
            if (member.compare("userId")==0)
            {
                consumer.fanxing_id = user.get(member, 0).asUInt();
            }
            else if (member.compare("coin") == 0)
            {
                consumer.coin = user.get(member, 0).asUInt();
            }
            else if (member.compare("richLevel") == 0)
            {
                consumer.rich_level = user.get(member, 0).asUInt();
            }
            else if (member.compare("nichName") == 0)
            {
                consumer.nickname = user.get(member, "").asString();
            }
        }

        if (consumer.fanxing_id)
            consumer_infos->push_back(consumer);
    }


    return true;
}

bool Room::KickOutUser(KICK_TYPE kicktype, const std::string&cookies,
    const EnterRoomUserInfo& enterRoomUserInfo)
{
    std::string strroomid = base::IntToString(static_cast<int>(enterRoomUserInfo.roomid));
    std::string url = std::string("http://fanxing.kugou.com");
    url += "/Services.php?act=RoomService.RoomManageService&mtd=kickOut&d=";
    url += GetNowTimeString();
    url += R"(&args=)";
    std::string jsonstr;
    jsonstr += std::string(R"([")");
    jsonstr += base::UintToString(singerid_);
    jsonstr += R"(",")";
    jsonstr += base::UintToString(enterRoomUserInfo.userid);
    jsonstr += R"(",")";
    jsonstr += base::UintToString(enterRoomUserInfo.roomid);
    jsonstr += R"(",3600,")";
    jsonstr += enterRoomUserInfo.nickname;
	if (kicktype == KICK_TYPE::KICK_TYPE_MONTH)
	{
		jsonstr += R"(",0,2])"; // 统一按踢一个月计算
	}
	else
	{
		jsonstr += R"(",0])"; // 统一按踢一小时计算
	}
    
    jsonstr = UrlEncode(jsonstr);

    url += jsonstr;

    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string data(response.content.begin(), response.content.end());
    //解析json数据
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(data, rootdata, false))
    {
        return false;
    }

    uint32 status = GetInt32FromJsonValue(rootdata, "status");
    if (status != 1)
    {
        return false;
    }
    return true;
}

bool Room::BanChat(const std::string& cookies, const EnterRoomUserInfo& enterRoomUserInfo)
{
    std::string strroomid = base::IntToString(static_cast<int>(enterRoomUserInfo.roomid));
    std::string url = std::string("http://fanxing.kugou.com");
    url += "/UServices/RoomService/RoomManageService/banChat/?d=";
    url += GetNowTimeString();
    url += R"(&args=)";
    std::string jsonstr;
    jsonstr += std::string(R"([")");
    jsonstr += base::UintToString(singerid_);
    jsonstr += R"(",")";
    jsonstr += base::UintToString(enterRoomUserInfo.userid);
    jsonstr += R"(",")";
    jsonstr += base::UintToString(enterRoomUserInfo.roomid);
    jsonstr += R"(",300,")";
    jsonstr += enterRoomUserInfo.nickname;
    jsonstr += R"("])";

    jsonstr = UrlEncode(jsonstr);

    url += jsonstr;

    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string data(response.content.begin(), response.content.end());
    //解析json数据
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(data, rootdata, false))
    {
        return false;
    }

    uint32 status = GetInt32FromJsonValue(rootdata, "status");
    if (status != 1)
    {
        return false;
    }
    return true;
}
bool Room::UnbanChat(const std::string& cookies, const EnterRoomUserInfo& enterRoomUserInfo)
{
    std::string strroomid = base::IntToString(static_cast<int>(enterRoomUserInfo.roomid));
    std::string url = std::string("http://fanxing.kugou.com");
    url += "/UServices/RoomService/RoomManageService/undoBanChat/?d=";
    url += GetNowTimeString();
    url += R"(&args=)";
    std::string jsonstr;
    jsonstr += std::string(R"([")");
    jsonstr += base::UintToString(singerid_);
    jsonstr += R"(",")";
    jsonstr += base::UintToString(enterRoomUserInfo.userid);
    jsonstr += R"(",")";
    jsonstr += base::UintToString(enterRoomUserInfo.roomid);
    jsonstr += R"(",")";
    jsonstr += enterRoomUserInfo.nickname;
    jsonstr += R"("])";

    jsonstr = UrlEncode(jsonstr);

    url += jsonstr;

    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string data(response.content.begin(),response.content.end());
    //解析json数据
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(data, rootdata, false))
    {
        return false;
    }

    uint32 status = GetInt32FromJsonValue(rootdata, "status");
    if (status != 1)
    {
        return false;
    }
    return true;
}

bool Room::SendChatMessage(const std::string& nickname, uint32 richlevel,
    const std::string& message)
{
    //return messageNotifyManager_->SendChatMessage(nickname, richlevel, message);
    return messageNotifyManager_->NewSendChatMessage(nickname, richlevel, message);
}

bool Room::SendChatMessage(const RoomChatMessage& roomChatMessage)
{
    return messageNotifyManager_->NewSendChatMessageRobot(roomChatMessage);
}

void Room::SetNormalNotify(NormalNotify normalNotify)
{
    messageNotifyManager_->SetNormalNotify(normalNotify);
}

void Room::SetNotify201(Notify201 notify201)
{
    messageNotifyManager_->SetNotify201(notify201);
}

void Room::SetNotify501(Notify501 notify501)
{
    messageNotifyManager_->SetNotify501(notify501);
}

void Room::SetNotify601(Notify601 notify601)
{
    notify601transfer_ = notify601;
    messageNotifyManager_->SetNotify601(
        std::bind(&Room::TranferNotify601,this,std::placeholders::_1));
}

void Room::SetNotify620(Notify620 notify_620)
{
    messageNotifyManager_->SetNotify620(notify_620);
}

bool Room::OpenRoom(const std::string& cookies)
{
    HttpRequest request;
    request.url = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/";
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string content;
    content.assign(response.content.begin(), response.content.end());

    for (const auto& it : response.cookies)
    {
        //assert(false && L"这里应该是不会设置cookie的");
        cookiesHelper_->SetCookies(it);
    }

    // 打开房间的功能不需要处理返回来的页面数据
    if (content.empty())
    {
        return false;
    }

    do 
    {// 普通房间
        std::string isClanRoomMark = "isClanRoom";
        std::string starId = "starId";
        auto isClanRoomPos = content.find(isClanRoomMark);
        if (isClanRoomPos == std::string::npos)
        {
            break;
        }
        auto starPos = content.find(starId, isClanRoomPos + isClanRoomMark.length());

        auto beginPos = content.find(':', starPos);
        beginPos += 1;
        auto endPos = content.find(',', beginPos);
        std::string temp = content.substr(beginPos, endPos - beginPos);

        RemoveSpace(&temp);
        temp = temp.substr(1, temp.length() - 2);
        if (temp.length() < 5)
        {
            break;
        }
        std::string singerid = temp;
        base::StringToUint(singerid, &singerid_);
    } while (0);

    if (singerid_==0)
    {
        // pk房间
        std::string target = "/index.php?action=user&id=";
        auto target_pos = content.find(target);
        if (target_pos == std::string::npos)
        {
            return false;
        }
        auto begin_pos = target_pos + target.size();
        auto end_pos = content.find('\"', begin_pos);
        std::string temp = content.substr(begin_pos, end_pos - begin_pos);

        RemoveSpace(&temp);
        if (temp.length() < 5)
        {
            return false;
        }
        std::string singerid = temp;
        base::StringToUint(singerid, &singerid_);
        assert(singerid_);
    }

    return !!singerid_;
}

bool Room::GetStarInfo(const std::string& cookies)
{
    assert(singerid_);
    std::string url = "http://visitor.fanxing.kugou.com";
    url += "/VServices/RoomService.RoomService.getStarInfo/";
    url += base::UintToString(singerid_);
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    if (response.content.empty())
    {
        assert(false);
        return false;
    }

    std::string responsedata(response.content.begin(), response.content.end());
    std::string jsondata = PickJson(responsedata);

    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(jsondata, rootdata, false))
    {
        assert(false);
        return false;
    }

    uint32 unixtime = rootdata.get("servertime", 1461378689).asUInt();
    uint32 status = rootdata.get("status", 0).asUInt();
    if (status != 1)
    {
        assert(false);
        return false;
    }
    Json::Value jvdata(Json::ValueType::objectValue);
    Json::Value data = rootdata.get(std::string("data"), jvdata);
    if (data.isNull() || !data.isObject())
    {
        assert(false);
        return false;
    }

    Json::Value::Members members = data.getMemberNames();
    for (const auto& member : members)
    {
        if (member.compare("clanId") == 0)
        {
            clanid_ = GetInt32FromJsonValue(data, member);
        }
        else if (member.compare("nickName")==0)
        {
            nickname_ = data.get(member, "").asString();
        }
        else if (member.compare("starLevel") == 0)
        {
            singer_star_level_ = GetInt32FromJsonValue(data, member);
        }
    }
    return true;
}

bool Room::GetStarKugouId(const std::string& cookies)
{
    assert(singerid_);
    std::string url = "http://fanxing.kugou.com";
    url += "/index.php";
    HttpRequest request;
    request.url = url;
    request.queries["action"] = "user";
    request.queries["id"] = base::UintToString(singerid_);;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    if (response.content.empty())
    {
        assert(false);
        return false;
    }

    std::string responsedata(response.content.begin(), response.content.end());

    std::string _starKugouId = "_starKugouId";
    auto beginpos = responsedata.find(_starKugouId);
    beginpos += _starKugouId.size();
    beginpos = responsedata.find("\"", beginpos);
    beginpos++;
    auto endpos = responsedata.find("\"", beginpos);

    std::string str_kugouid = responsedata.substr(beginpos, endpos - beginpos);

    base::StringToUint(str_kugouid, &star_kugou_id_);

    return true;
}
// 需要重写
bool Room::EnterRoom(const std::string& cookies, uint32 userid, const std::string& usertoken)
{
    HttpRequest request;
    request.url = "http://fanxing.kugou.com/UServices/RoomService/RoomService/tryEnter";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/" + base::IntToString(roomid_);
    request.queries["args"] = "[%22" + 
        base::IntToString(static_cast<int>(roomid_))+
        "%22,%22%22,%22%22,%22web%22]";
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string content;
    content.assign(response.content.begin(), response.content.end());

    for (const auto& it : response.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    if (content.empty())
    {
        return false;
    }

    const std::string& data = content;
    //解析json数据
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(data, rootdata, false))
    {
        return false;
    }

	uint32 status = GetInt32FromJsonValue(rootdata, "status");
    if (status != 1)
    {
        return false;
    }
    return true;
}

bool Room::GetStarGuard()
{
    assert(singerid_ && roomid_);
    std::string url = "http://visitor.fanxing.kugou.com/VServices/GuardService.GuardService.getRoomGuardLst/";
    url += base::UintToString(singerid_) + "-" + base::UintToString(roomid_);
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/" + base::IntToString(roomid_);
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string content;
    content.assign(response.content.begin(), response.content.end());

    for (const auto& it : response.cookies)
        cookiesHelper_->SetCookies(it);

    if (content.empty())
        return false;

    const std::string& rootdata = PickJson(content);
    //解析json数据
    Json::Reader reader;
    Json::Value root(Json::objectValue);
    if (!reader.parse(rootdata, root, false))
        return false;

    uint32 status = GetInt32FromJsonValue(root, "status");
    if (status != 1)
        return false;

    Json::Value defaultval(Json::objectValue);
    auto data = root.get("data", defaultval);
    if (!data.isObject())
    {
        return false;
    }

    auto guardlist = data.get("list", defaultval);
    if (!guardlist.isArray())
    {
        return false;
    }

    for (auto guardinfo : guardlist)
    {
        std::string nickName = guardinfo.get("nickName", "").asString();
        std::string userLogo = guardinfo.get("userLogo", "").asString();
        uint32 roomId = GetInt32FromJsonValue(guardinfo, "roomId");
        uint32 guardLevel = GetInt32FromJsonValue(guardinfo, "guardLevel");
        uint32 annualFee = GetInt32FromJsonValue(guardinfo, "annualFee");
        std::string guardLvIcon = guardinfo.get("guardLvIcon", "").asString();
        uint32 userid = GetInt32FromJsonValue(guardinfo, "userId");
        uint32 online = GetInt32FromJsonValue(guardinfo, "online");
        guarduserids_.push_back(userid);
    }

    return true;
}

void Room::TranferNotify601(const RoomGiftInfo601& roomgiftinfo)
{
    // 如果不是在本房间送给主播的消息，过滤掉不回调
    if ((roomid_ != roomgiftinfo.roomid)
        || (singerid_ != roomgiftinfo.receiverid))
    {
        return;
    }
    if (!notify601transfer_)
        return;

    notify601transfer_(roomgiftinfo);
}

bool Room::ConnectToNotifyServer_(uint32 roomid, uint32 userid,
    const std::string& usertoken,
    const base::Callback<void()>& conn_break_callback)
{
    bool ret = false;
    if (ipproxy_.GetProxyType()!= IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        messageNotifyManager_->SetIpProxy(ipproxy_);
    }
    
    ret = messageNotifyManager_->NewConnect843(roomid, userid, usertoken, conn_break_callback);
    assert(ret);
    return ret;
}
