#include "Room.h"
#include "CurlWrapper.h"
#include "MessageNotifyManager.h"
#include "CookiesHelper.h"
#include "EncodeHelper.h"

#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/json/json.h"

Room::Room(uint32 roomid)
    :roomid_(roomid)
    , curlWrapper_(new CurlWrapper)
    , messageNotifyManager_(new MessageNotifyManager)
    , cookiesHelper_(new CookiesHelper)
{
    messageNotifyManager_->Initialize();
}

Room::~Room()
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

void Room::SetTcpManager(TcpManager* tcpManager)
{
    messageNotifyManager_->SetTcpManager(tcpManager);
}

bool Room::EnterForOperation(const std::string& cookies, 
    const std::string& usertoken, uint32 userid, uint32* singer_clanid)
{
    std::string nickname = "";
    uint32 richlevel = 0;
    uint32 ismaster = 0;
    uint32 singerid = 0;
    std::string key = "";
    std::string ext = "";

    if (!OpenRoom(cookies))
        return false;

    if (!GetStarInfo(cookies))
        return false;

    GetStarGuard();

    if (!EnterRoom(cookies, userid, usertoken))
        return false;
    
    if (!ConnectToNotifyServer_(roomid_, userid, usertoken))
        return false;

    if (singer_clanid)
        *singer_clanid = clanid_;

    return true;
}

bool Room::EnterForAlive(const std::string& cookies, const std::string& usertoken, uint32 userid)
{
    if (!ConnectToNotifyServer_(roomid_, userid, usertoken))
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
bool Room::SendGift(const std::string& cookies, uint32 gift_id, uint32 gift_count)
{
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
        assert(false && L"送礼物请求失败");
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
        assert(false && L"这里应该是不会设置cookie的");
        cookiesHelper_->SetCookies(it);
    }

    // 打开房间的功能不需要处理返回来的页面数据
    if (content.empty())
    {
        return false;
    }
    std::string isClanRoomMark = "isClanRoom";
    std::string starId = "starId";
    auto isClanRoomPos = content.find(isClanRoomMark);
    if (isClanRoomPos == std::string::npos)
    {
        return false;
    }
    auto starPos = content.find(starId, isClanRoomPos + isClanRoomMark.length());

    auto beginPos = content.find("\"", starPos);
    beginPos += 1;
    auto endPos = content.find("\"", beginPos);

    std::string singerid = content.substr(beginPos, endPos - beginPos);
    base::StringToUint(singerid, &singerid_);
    return true;
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
    }
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
    const std::string& usertoken)
{
    bool ret = false;
    if (ipproxy_.GetProxyType()!= IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        messageNotifyManager_->SetIpProxy(ipproxy_);
    }
    
    ret = messageNotifyManager_->NewConnect843(roomid, userid, usertoken);
    assert(ret);
    return ret;
}
