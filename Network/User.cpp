#include <regex>
#include "User.h"
#include "Room.h"
#include "CurlWrapper.h"
#include "CookiesHelper.h"
#include "EncodeHelper.h"

#include "third_party/json/json.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

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
}

User::User()
    :curlWrapper_(new CurlWrapper)
    , cookiesHelper_(new CookiesHelper)
{
}

User::~User()
{
    Logout();
}

// 设置参数
void User::SetUsername(const std::string& username)
{
    username_ = username;
}

std::string User::GetUsername() const
{
    return username_;
}

void User::SetPassword(const std::string& password)
{
    password_ = password;
}

std::string User::GetPassword() const
{
    return password_;
}

void User::SetIpProxy(const IpProxy& ipproxy)
{
    ipproxy_ = ipproxy;
}

IpProxy User::GetIpProxy() const
{
    return ipproxy_;
}

void User::SetCookies(const std::string& cookies)
{
    cookiesHelper_->SetCookies(cookies);
}

std::string User::GetCookies() const
{
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    std::string cookie = cookiesHelper_->GetCookies(keys);
    return cookie;
}

void User::SetRoomServerIp(const std::string& serverip)
{
    serverip_ = serverip;
}

void User::SetTcpManager(TcpManager* tcpManager)
{
    tcpManager_ = tcpManager;
}

//设置房间命令消息回调函数,命令的解析和行为处理要在另外的模块处理
void User::SetNormalNotify(NormalNotify normalNotify)
{
    normalNotify_ = normalNotify;
}

void User::SetNotify201(Notify201 notify201)
{
    notify201_ = notify201;
}

void User::SetNotify501(Notify501 notify501)
{
    notify501_ = notify501;
}

void User::SetNotify601(Notify601 notify601)
{
    notify601_ = notify601;
}

bool User::Login()
{
    std::string msg;
    return Login(username_, password_, "", &msg);
}

// 操作行为
bool User::Login(const std::string& username, const std::string& password, 
    const std::string& verifycode, std::string* errormsg)
{
    if (!verifycode.empty())
    {
        if (!CheckVerifyCode(verifycode, errormsg))
        {
            return false;
        }
    }

    std::string msg;
    if (!LoginHttps(username, password, verifycode, &msg))
    {
        *errormsg = msg;
        return false;
    }
    
    if (!LoginUServiceGetMyUserDataInfo(&msg))
    {
        *errormsg = msg;
        return false;
    }

    if (!LoginIndexServiceGetUserCenter(&msg))
    {
        *errormsg = msg;
        return false;
    }
    username_ = username;
    password_ = password;
    return true;
}

// KuGoo=KugooID=641607819&KugooPwd=8AAB7C888611C4D2ACE635A44B5FC273&
//NickName=%u0066%u0061%u006e%u0078%u0069%u006e%u0067%u0074%u0065%u0073%u0074%u0030%u0030%u0031&
//Pic=http://imge.kugou.com/kugouicon/165/20100101/20100101192931478054.jpg&
//RegState=1&RegFrom=&t=20c7dd5dbfdea2e25846e666f820d64ab70eb771b034e8ae568e578c8154450d&
//a_id=1010&ct=1466237656&UserName=%u0066%u0061%u006e%u0078%u0069%u006e%u0067%u0074%u0065%u0073%u0074%u0030%u0030%u0031
bool User::LoginWithCookies(const std::string& cookies, std::string* errormsg)
{
    std::regex pattern1(R"(KugooID=[0-9]*)");

    std::string KugooID;
    std::string s = cookies;
    std::smatch match;
    while (std::regex_search(s, match, pattern1))
    {
        for (auto x : match)
            KugooID = x;
        s = match.suffix().str();
    }

    if (KugooID.length()<7)
        return false;
    
    KugooID = KugooID.substr(8);
    std::regex pattern2(R"(t=[0-9a-f]{64})");

    std::string token;
    s = cookies;
    while (std::regex_search(s, match, pattern2))
    {
        for (auto x : match)
            token = x;
        s = match.suffix().str();
    }

    if (token.length() < 3)
        return false;

    token = token.substr(2);
    assert(token.size() == 64);
    usertoken_ = token;
    base::StringToUint(KugooID, &kugouid_);

    cookiesHelper_->SetCookies(cookies);

    std::string msg;
    if (!LoginUServiceGetMyUserDataInfo(&msg))
    {
        *errormsg = msg;
        return false;
    }

    if (!LoginIndexServiceGetUserCenter(&msg))
    {
        *errormsg = msg;
        return false;
    }

    return true;
}

bool User::Logout()
{
    // 需要断掉房间连接
    for (const auto& room : rooms_)
    {
        room.second->Exit();
    }
    return false;
}

bool User::LoginGetVerifyCode(std::vector<uint8>* picture)
{
    std::string url = "http://verifycode.service.kugou.com/v1/get_img_code";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://fanxing.kugou.com/";
    request.queries["type"] = "LoginCheckCode";
    request.queries["appid"] = "1010";
    request.queries["codetype"] = "0";
    request.queries["t"] = GetNowTimeString();
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    for (const auto& it : response.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    *picture = response.content;
    return true;
}

uint32 User::GetServerTime() const
{
    return servertime_;
}

uint32 User::GetFanxingId() const
{
    return fanxingid_;
}

uint32 User::GetClanId() const
{
    return clanid_;
}

bool User::EnterRoomFopOperation(uint32 roomid)
{
    std::shared_ptr<Room> room(new Room(roomid));
    room->SetTcpManager(tcpManager_);
    room->SetRoomServerIp(serverip_);
    std::vector<std::string> keys;
    keys.push_back("_fx_coin");
    keys.push_back("_fx_user");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    keys.push_back("KuGoo");
    std::string cookie = cookiesHelper_->GetCookies(keys);
    if (normalNotify_)
    {
        room->SetNormalNotify(normalNotify_);
    }
    if (notify201_)
    {
        room->SetNotify201(notify201_);
    }

    if (notify501_)
    {
        room->SetNotify501(notify501_);
    }

    if (notify601_)
    {
        room->SetNotify601(notify601_);
    }
    // 如果存在重复的房间，先断掉旧的
    this->ExitRoom(roomid);

    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        room->SetIpProxy(ipproxy_);
    }

    if (!room->EnterForOperation(cookie,usertoken_,kugouid_))
    {
        return false;
    }
    
    rooms_[roomid] = room;
    return true;
}

bool User::EnterRoomFopAlive(uint32 roomid)
{
    std::shared_ptr<Room> room(new Room(roomid));
    room->SetTcpManager(tcpManager_);
    room->SetRoomServerIp(serverip_);
    std::vector<std::string> keys;
    keys.push_back("_fx_coin");
    keys.push_back("_fx_user");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    keys.push_back("KuGoo");
    std::string cookie = cookiesHelper_->GetCookies(keys);
    if (normalNotify_)
    {
        room->SetNormalNotify(normalNotify_);
    }
    if (notify201_)
    {
        room->SetNotify201(notify201_);
    }

    if (notify501_)
    {
        room->SetNotify501(notify501_);
    }
    // 如果存在重复的房间，先断掉旧的
    this->ExitRoom(roomid);

    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        room->SetIpProxy(ipproxy_);
    }

    if (!room->EnterForAlive(cookie, usertoken_, kugouid_))
    {
        return false;
    }

    rooms_[roomid] = room;
    return true;
}
bool User::ExitRoom(uint32 roomid)
{
    auto it = rooms_.find(roomid);
    if (it != rooms_.end())
    {
        it->second->Exit();
        rooms_.erase(it);
        return true;
    }
    return false;
}

bool User::ExitRooms()
{
    for (auto it : rooms_)
    {
        it.second->Exit();
    }
    rooms_.clear();
    return true;
}

bool User::SendChatMessage(uint32 roomid, const std::string& message)
{
    auto room = rooms_.find(roomid);
    if (room == rooms_.end())
    {
        return false;
    }
    return room->second->SendChatMessage(nickname_, richlevel_, message);
}

bool User::SendChatMessageRobot(const RoomChatMessage& roomChatMessage)
{
    auto room = rooms_.find(roomChatMessage.roomid);
    if (room == rooms_.end())
    {
        return false;
    }
    return room->second->SendChatMessage(roomChatMessage);
}

void User::SetRobotApiKey(const std::string& apikey)
{
    apikey_ = apikey;
}

bool User::RequestRobot(uint32 senderid, const std::string& request, std::string* response)
{
    if (!response || !senderid || request.empty())
        return false;

    assert(!apikey_.empty());
    if (apikey_.empty())
        return false;

    HttpRequest httpRequest;
    httpRequest.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_POST;
    httpRequest.url = "http://www.tuling123.com/openapi/api";
    std::map<std::string, std::string> postmap;
    postmap["key"] = apikey_;
    postmap["info"] = request;
    postmap["userid"] = base::UintToString(senderid);
    MakePostdata(postmap, &httpRequest.postdata);

    HttpResponse httpResponse;
    if (!curlWrapper_->Execute(httpRequest, &httpResponse))
    {
        return false;
    }
    
    std::string responsedata(httpResponse.content.begin(), httpResponse.content.end());
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(responsedata, rootdata, false))
    {
        return false;
    }

    uint32 code = GetInt32FromJsonValue(rootdata, "code");
    if (code != 100000)
    {
        *response = base::WideToUTF8(L"别闹, 我的智商不够用了");
        return false;
    }
    *response = rootdata.get("text", "").asString();
    return true;
}
bool User::SendStar(uint32 count)
{
    return false;
}

bool User::RetrieveStart()
{
    return false;
}

bool User::SendGift(uint32 giftid)
{
    return false;
}

bool User::GetViewerList(uint32 roomid,
    std::vector<EnterRoomUserInfo>* enterRoomUserInfo)
{
    auto room = rooms_.find(roomid);
    if (room == rooms_.end())
    {
        return false;
    }
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    keys.push_back("_fx_coin");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    std::string cookies = cookiesHelper_->GetCookies(keys);
    bool result = room->second->GetViewerList(cookies, enterRoomUserInfo);
    return result;
}

bool User::KickoutUser(KICK_TYPE kicktype, uint32 roomid,
    const EnterRoomUserInfo& enterRoomUserInfo)
{
    auto room = rooms_.find(roomid);
    if (room==rooms_.end())
    {
        return false;
    }

    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    keys.push_back("_fx_coin");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    return room->second->KickOutUser(kicktype, cookies,enterRoomUserInfo);
}

bool User::BanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo)
{
    auto room = rooms_.find(roomid);
    if (room == rooms_.end())
    {
        return false;
    }
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    keys.push_back("_fx_coin");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    return room->second->BanChat(cookies, enterRoomUserInfo);
}
bool User::UnbanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo)
{
    auto room = rooms_.find(roomid);
    if (room == rooms_.end())
    {
        return false;
    }
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    keys.push_back("_fx_coin");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    return room->second->UnbanChat(cookies, enterRoomUserInfo);
}

bool User::Worship(uint32 roomid, uint32 userid, std::string* errormsg)
{
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    keys.push_back("_fx_coin");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    return Worship_(cookies, roomid, userid, errormsg);
}

bool User::Worship_(const std::string& cookies, uint32 roomid, uint32 userid,
    std::string* errormsg)
{
    std::string url = "http://fanxing.kugou.com";
    url += "/NServices/worship/WorshipService/worship";

    HttpRequest request;
    request.url = url;
    request.queries["args"] = "[%22" + base::IntToString(userid) + "%22,%22"+
        base::IntToString(roomid)+"%22]";
    request.queries["_"] = GetNowTimeString();
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid);
    request.cookies = cookies;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string jsondata(response.content.begin(), response.content.end());
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
        *errormsg = rootdata.get("errorcode", "").asString();
        if (!errormsg->empty())
        {
            return false;
        }
    }
    Json::Value jvdata(Json::ValueType::objectValue);
    Json::Value data = rootdata.get(std::string("data"), jvdata);
    if (data.isNull() || !data.isObject())
    {
        assert(false);
        return false;
    }
    return true;
}

bool User::CheckVerifyCode(const std::string& verifycode, std::string* errormsg)
{
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = "http://verifycode.service.kugou.com/v1/check_img_code/";
    request.referer = "http://www.fanxing.kugou.com/";
    request.cookies = cookiesHelper_->GetCookies("LoginCheckCode");;
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    queries["appid"] = "1010";;
    queries["code"] = verifycode;
    queries["ct"] = base::UintToString(
        static_cast<uint32>(base::Time::Now().ToDoubleT()));
    queries["type"] = "LoginCheckCode";
    queries["callback"] = "checkCodeForLoginCallback";

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        *errormsg = "http request error";
        return false;
    }

    std::string responsedata;
    responsedata.assign(response.content.begin(), response.content.end());
    if (responsedata.empty())
    {
        *errormsg = "http response data empty";
        return  false;
    }


    std::string jsondata = PickJson(responsedata);
    Json::Reader reader;
    Json::Value logindata(Json::objectValue);
    if (!reader.parse(jsondata, logindata, false))
    {
        *errormsg = "json parse error";
        return false;
    }

    *errormsg = logindata.get("errorMsg", "").asString();
    if (!errormsg->empty())
    {
        std::wstring werrorMsg = base::UTF8ToWide(*errormsg);
        return false;
    }

    return true;
}

bool User::LoginHttps(const std::string& username, const std::string& password, 
    const std::string& verifycode, std::string* errormsg)
{
    const char* loginuserurl = "https://login-user.kugou.com/v1/login/";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = loginuserurl;
    request.referer = "http://www.fanxing.kugou.com"; 
    request.cookies = cookiesHelper_->GetCookies("LoginCheckCode");;
    if (ipproxy_.GetProxyType()!=IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    queries["appid"] = "1010";
    queries["username"] = UrlEncode(username);
    queries["pwd"] = MakeMd5FromString(password);;
    queries["code"] = verifycode;
    queries["clienttime"] = base::UintToString(
        static_cast<uint32>(base::Time::Now().ToDoubleT()));
    queries["expire_day"] = "3";
    queries["autologin"] = "false";
    queries["redirect_uri"] = "";
    queries["state"] = "";
    queries["callback"] = "loginSuccessCallback";
    queries["login_ver"] = "1";

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }
 //   loginSuccessCallback({
	//"pic" : "http:\/\/imge.kugou.com\/kugouicon\/165\/20100101\/20100101192931478054.jpg",
	//"nickname" : "fanxingtest001",
	//"username" : "fanxingtest001",
	//"token" : "20c7dd5dbfdea2e25846e666f820d64ae1ad3afdb8a0a7652a447fcc392d1e46",
	//"userid" : 641607819
 //   })

    std::string responsedata;
    responsedata.assign(response.content.begin(), response.content.end());
    if (responsedata.empty())
        return  false;
    std::string header = "loginSuccessCallback(";
    std::string jsondata = PickJson(responsedata);
    //std::string beginmark = R"("token":")";
    //auto beginpos = responsedata.find(beginmark);
    //if (beginpos == std::string::npos)
    //    return false;
    //beginpos += beginmark.size();
    //auto endpos = responsedata.find("\"",beginpos);
    //usertoken_ = responsedata.substr(beginpos, endpos - beginpos);

    Json::Reader reader;
    Json::Value logindata(Json::objectValue);
    if (!reader.parse(jsondata, logindata, false))
    {
        *errormsg = "json parse error";
        return false;
    }

    *errormsg = logindata.get("errorMsg", "").asString();
    if (!errormsg->empty())
    {
        std::wstring werrorMsg = base::UTF8ToWide(*errormsg);
        return false;
    }

    Json::Value jvCmd(Json::ValueType::intValue);
    usertoken_ = logindata.get("token", "").asString();
    kugouid_ = logindata.get("userid",0).asUInt();

    for (const auto& it : response.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    return true;
}

bool User::LoginUServiceGetMyUserDataInfo(std::string* errormsg)
{
    const char* GetMyUserDataInfoUrl = "http://fanxing.kugou.com/UServices/UserService/UserService/getMyUserDataInfo";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = GetMyUserDataInfoUrl;
    request.referer = "http://www.fanxing.kugou.com";
    request.cookies = cookiesHelper_->GetCookies("KuGoo");
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    queries["args"] = "[]";
    queries["_"] = GetNowTimeString();

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        *errormsg = "http request error";
        return false;
    }

    std::string responsedata;
    responsedata.assign(response.content.begin(), response.content.end());
    if (responsedata.empty())
        return  false;

    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(responsedata, rootdata, false))
    {
        *errormsg = "json parse error";
        return false;
    }

    // 有必要检测status的值
    uint32 status = GetInt32FromJsonValue(rootdata, "status");
    if (status != 1)
    {
        std::string errorMsg = rootdata.get("errorcode", "").asString();
        *errormsg = errorMsg;
        std::wstring werrorMsg = base::UTF8ToWide(errorMsg);
        return false;
    }
    uint32 servertime = GetInt32FromJsonValue(rootdata, "servertime");
    servertime_ = servertime;
    Json::Value dataObject(Json::objectValue);
    dataObject = rootdata.get(std::string("data"), dataObject);
    if (dataObject.empty())
    {
        *errormsg = "json parse error";
        return false;
    }

    auto fxUserInfo = dataObject.get("fxUserInfo", Json::Value());
    Json::Value::Members members = fxUserInfo.getMemberNames();
    for (const auto& member : members)
    {
        if (member.compare("clanId")==0)
        {
            clanid_ = GetInt32FromJsonValue(fxUserInfo, member);
        }
        else if (member.compare("coin") == 0)
        {
            coin_ = static_cast<uint32>(GetDoubleFromJsonValue(fxUserInfo, member));
        }
        else if (member.compare("userId")==0)
        {
            fanxingid_ = static_cast<uint32>(GetDoubleFromJsonValue(fxUserInfo, member));
        }
        else if (member.compare("nickName") == 0)
        {
            nickname_ = fxUserInfo.get(member, "nickname").asString();
        }
        else if (member.compare("richLevel") == 0)
        {
            richlevel_ = GetInt32FromJsonValue(fxUserInfo, member);
        }
    }

    for (const auto& it : response.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    return true;
}

bool User::LoginIndexServiceGetUserCenter(std::string* errormsg)
{
    const char* GetMyUserDataInfoUrl = "http://fanxing.kugou.com/Services/IndexService/IndexService/getUserCenter";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = GetMyUserDataInfoUrl;
    request.referer = "http://www.fanxing.kugou.com";
    request.cookies = cookiesHelper_->GetCookies("KuGoo");
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    queries["args"] = "[%22%22]";
    queries["_"] = GetNowTimeString();

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    // 仅仅是为了取得cookies
    for (const auto& it : response.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    // response.content在这里不用处理。
    return true;
}
