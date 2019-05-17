#include <regex>
#include "User.h"
#include "Room.h"
#include "CurlWrapper.h"
#include "CookiesHelper.h"
#include "EncodeHelper.h"

#include "third_party/json/json.h"
#include "third_party/chromium/base/rand_util.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

namespace
{
    const char* login_callback_string = "Fx.login.loginSdkkugouCallback.loginSuccess";
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

bool User::Initialize(const scoped_refptr<base::TaskRunner>& runner)
{
    runner_ = runner;
    return true;
}

void User::Finalize()
{
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
    std::string cookie = cookiesHelper_->GetNormalCookies();
    return cookie;
}

// 因为进房连接的ip是通过dns解析"chat1.fanxing.kugou.com"地址决定的
void User::SetRoomServerIp(const std::string& serverip)
{
    serverip_ = serverip;
    //const char* url = "chat1.fanxing.kugou.com";
    //HttpRequest request;
    //request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    //request.url = url;
    //if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    //    request.ipproxy = ipproxy_;

    //HttpResponse response;
    //// 这里请求是都失败的，但是可以从dns解析里拿到目标地址
    //curlWrapper_->Execute(request, &response);

    //assert(!response.server_ip.empty());
    //serverip_ = response.server_ip;
}

//void User::SetTcpManager(TcpClientController* tcpManager)
//{
//    tcpManager_ = tcpManager;
//}

void User::SetWebsocketClientController(WebsocketClientController* controller)
{
    websocket_client_controller_ = controller;
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

void User::SetNotify620(Notify620 notify_620)
{
    notify_620_ = notify_620;
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
    if (kg_mid_.empty())
    {
        kg_mid_ = MakeMd5FromString(username + base::UintToString(
            static_cast<uint32>(base::Time::Now().ToDoubleT())));
        cookiesHelper_->SetCookies("kg_mid", "kg_mid=" + kg_mid_);
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

    //if (!LoginIndexServiceGetUserCenter(&msg))
    //{
    //    *errormsg = msg;
    //    return false;
    //}

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
    request.cookies = cookiesHelper_->GetCookies("kg_mid");
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

bool User::GetRoom(uint32 roomid, std::shared_ptr<Room>* room)
{
    auto result = rooms_.find(roomid);
    if (result == rooms_.end())
        return false;

    *room = result->second;
    return true;
}
uint32 User::GetRichlevel() const
{
    return richlevel_;
}

bool User::EnterRoomFopOperation(uint32 roomid, uint32* singer_clanid,
    const base::Callback<void()>& conn_break_callback)
{
    std::shared_ptr<Room> room(new Room(roomid));
    //room->SetTcpManager(tcpManager_);
    room->SetWebsocketClientController(websocket_client_controller_);
    room->SetRoomServerIp(serverip_);
    room->Initialize(runner_);
    std::vector<std::string> keys;
    keys.push_back("_fx_coin");
    keys.push_back("_fx_user");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    keys.push_back("KuGoo");
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
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

    if (!room->EnterForOperation(cookie, usertoken_, kugouid_, singer_clanid, conn_break_callback))
    {
        return false;
    }
    
    rooms_[roomid] = room;
    return true;
}

bool User::EnterRoomFopAlive(uint32 roomid,
    const base::Callback<void()>& conn_break_callback)
{
    std::shared_ptr<Room> room(new Room(roomid));
    room->Initialize(runner_);
    //room->SetTcpManager(tcpManager_);
    room->SetWebsocketClientController(websocket_client_controller_);
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
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

    if (notify_620_)
    {
        room->SetNotify620(notify_620_);
    }

    // 如果存在重复的房间，先断掉旧的
    this->ExitRoom(roomid);

    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        room->SetIpProxy(ipproxy_);
    }

    if (!room->EnterForAlive(cookie, usertoken_, kugouid_, conn_break_callback))
    {
        return false;
    }

    rooms_[roomid] = room;
    return true;
}

bool User::OpenRoomAndGetViewerList(uint32 roomid,
    std::vector<EnterRoomUserInfo>* enterRoomUserInfoList)
{
    std::shared_ptr<Room> room(new Room(roomid));
    std::vector<std::string> keys;
    keys.push_back("_fx_coin");
    keys.push_back("_fx_user");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    keys.push_back("KuGoo");
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
    std::string cookie = cookiesHelper_->GetCookies(keys);
    if (!room->OpenRoomAndGetViewerList(cookie, enterRoomUserInfoList))
        return false;

    return true;
}

bool User::OpenRoomAndGetConsumerList(uint32 roomid,
    std::vector<ConsumerInfo>* consumer_infos, uint32* star_level)
{
    std::shared_ptr<Room> room(new Room(roomid));
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
    if (!room->OpenRoomAndGetConsumerList(cookie, consumer_infos, star_level))
        return false;

    return true;
}

bool User::EnterRoomFopHttp(uint32 roomid, std::shared_ptr<Room> room)
{
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

bool User::SendPrivateMessageToSinger(uint32 roomid, const std::string& message)
{
    auto room = rooms_.find(roomid);
    if (room == rooms_.end())
    {
        return false;
    }

    RoomChatMessage responseChat;
    responseChat.roomid = roomid;
    responseChat.senderid = fanxingid_;
    responseChat.sendername = nickname_;
    responseChat.richlevel = richlevel_;
    responseChat.receiverid = room->second->GetSingerId();
    responseChat.receivername = room->second->GetSingerName();
    responseChat.issecrect = true;
    responseChat.chatmessage = message;

    return room->second->SendChatMessage(responseChat);
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
bool User::SendStar(uint32 roomid, uint32 count)
{
    auto room = rooms_.find(roomid);
    if (room == rooms_.end())
    {
        return false;
    }

    //std::vector<std::string> keys;
    //keys.push_back("KuGoo");
    //keys.push_back("_fx_coin");
    //keys.push_back("_fxNickName");
    //keys.push_back("_fxRichLevel");
    //keys.push_back("FANXING_COIN");
    //keys.push_back("FANXING");
    //keys.push_back("fxClientInfo");
    std::string cookies = cookiesHelper_->GetAllCookies();
    std::string errormsg;
    return room->second->SendStar(cookies, roomid, count, &errormsg);
}

bool User::RetrieveStar()
{
    return false;
}

bool User::SendGift(uint32 roomid, uint32 gift_id, uint32 gift_count,
                    std::string* errormsg)
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
    std::string cookies = cookiesHelper_->GetCookies(keys);
    return room->second->SendGift(cookies, gift_id, gift_count, errormsg);
}

bool User::RealSingLike(uint32 roomid, const std::wstring& song_name,
    std::string* errormsg)
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
    std::string cookies = cookiesHelper_->GetCookies(keys);
    return room->second->RealSingLike(cookies,
        kugouid_, usertoken_, song_name, errormsg);
}

bool User::RetrieveHappyFreeCoin(uint32 roomid, const std::string& gift_token, 
    uint32* coin, std::string* errormsg)
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

    std::string url = "http://fanxing.kugou.com/Services.php";
    HttpRequest request;
    request.url = url;
    request.cookies = cookies;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.queries["act"] = "GiftService%2EGiftService";
    request.queries["args"] = std::string("%5B%22")
        + base::UintToString(roomid) + "%22%2C%22"
        + gift_token + "%22%5D";
    request.queries["mtd"] = "tryGetHappyFreeCoin";

    // 生成一个小数，小数16位，方式比较丑陋，实现就好
    uint32 first = base::RandInt(10000000, 99999999);
    uint32 second = base::RandInt(10000000, 99999999);
    std::string ran = "0%2E" + base::UintToString(first) + base::UintToString(second);
    request.queries["ran"] = ran;

    // FIX ME: 现在不知道version是使用多少
    request.referer = "http://fanxing.kugou.com/static/swf/award/CommonMoneyGift.swf?version=20140221";

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
        return false;

    if (response.content.empty())
        return false;

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
        std::string utf8_str;
        UnicodeToUtf8(errorcode, &utf8_str);
        std::wstring message = L"抢币失败:" + base::UTF8ToWide(utf8_str);
        if (normalNotify_)
            normalNotify_(MessageLevel::MESSAGE_LEVEL_DISPLAY, message);

        *errormsg = errorcode;
        return false;
    }

    *coin = GetInt32FromJsonValue(rootdata, "data");

    std::wstring message = L"抢币成功，抢到" + base::UintToString16(*coin) + L"星币";
    if (normalNotify_)
        normalNotify_(MessageLevel::MESSAGE_LEVEL_DISPLAY, message);
    
    return true;
}

bool User::GetGiftList(uint32 roomid, std::string* content)
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
    std::string cookies = cookiesHelper_->GetCookies(keys);
    bool result = room->second->GetGiftList(cookies, content);
    return result;
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    return room->second->UnbanChat(cookies, enterRoomUserInfo);
}

bool User::BanEnter(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo)
{
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    //keys.push_back("_fx_coin");
    //keys.push_back("_fxNickName");
    //keys.push_back("_fxRichLevel");
    //keys.push_back("FANXING_COIN");
    //keys.push_back("FANXING");
    //keys.push_back("fxClientInfo");
    //keys.push_back("LoginCheckCode");
    //keys.push_back("kg_mid");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    std::string strroomid = base::IntToString(static_cast<int>(enterRoomUserInfo.roomid));
    std::string url = std::string("https://fx.service.kugou.com");
    url += "/UServices/RoomService/RoomManageService/kickOutAnyForStar";
    HttpRequest request;
    request.url = url;
    request.queries["args"] = "[" + base::IntToString(enterRoomUserInfo.userid) + "]";
    request.queries["jsonpcallback"] = "jsonpcallback_httpsfxservicekugoucomUServicesRoomServiceRoomManageServicekickOutAnyForStarargs" + base::IntToString(enterRoomUserInfo.userid);
    request.queries["_"] = GetNowTimeString();
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/index.php?action=userBlackList";
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

bool User::UnbanEnter(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo)
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    std::string strroomid = base::IntToString(static_cast<int>(enterRoomUserInfo.roomid));
    std::string url = std::string("https://fx.service.kugou.com");
    url += "/UServices/RoomService/RoomManageService/undoKickOutAnyForStar";
    HttpRequest request;
    request.url = url;
    request.queries["args"] = "[" + base::IntToString(enterRoomUserInfo.userid) + "]";
    request.queries["jsonpcallback"] = "jsonpcallback_httpsfxservicekugoucomUServicesRoomServiceRoomManageServiceundoKickOutAnyForStarargs" + base::IntToString(enterRoomUserInfo.userid);
    request.queries["_"] = GetNowTimeString();
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/index.php?action=userBlackList";
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

bool User::SetRoomGiftNotifyLevel(uint32 roomid, uint32 gift_value)
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
    keys.push_back("LoginCheckCode");
    keys.push_back("kg_mid");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    std::string url = std::string("https://fx.service.kugou.com");
    url += "/UServices/GiftService/GiftService/setShowLimit";
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid);
    request.queries["args"] = "[" + base::IntToString(static_cast<int>(gift_value)) + "]";
    request.queries["jsonpcallback"] = "jsonphttpsfxservicekugoucomUServicesGiftServiceGiftServicesetShowLimitargs0jsonpcallback";
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
    std::string json = PickJson(data);
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(json, rootdata, false))
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

bool User::GetAnnualInfo(std::string* username, uint32 coin_count,
    uint32* award_count, uint32* single_count) const
{
    return false;
}

bool User::RobVotes(uint32 roomid, uint32* award_count, uint32* single_count,
                    std::string* errormsg)
{
    if (rooms_.find(roomid) == rooms_.end())
    {
        *errormsg = base::WideToUTF8(L"本地数据错误, 未进入房间");
        return false;
    }

    std::string url = "http://service.fanxing.kugou.com/fxannualawards/api/StarAnnualAwards/robVotes";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://fanxing.kugou.com/" + base::UintToString(roomid);
    request.cookies = cookiesHelper_->GetNormalCookies();
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    queries["args"] = "[]";

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
        assert(false && L"开大奖票宝箱失败");
        *errormsg = errorcode;
        return false;
    }

    Json::Value jvdata(Json::ValueType::objectValue);
    Json::Value data = rootdata.get(std::string("data"), jvdata);
    if (data.isNull() || !data.isObject())
    {
        assert(false);
        return false;
    }

    std::string notifymsg = data.get(std::string("msg"), "").asString();
    *errormsg = notifymsg;
    return true;
}

bool User::GetStorageGift(UserStorageInfo* user_storage_info, std::string* errormsg)
{
    ///UServices/GiftService/StorageService/getStorageByUserId?args=[]&_=1476974477980
    std::string url = "http://fanxing.kugou.com/UServices/GiftService/StorageService/getStorageByUserId";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://fanxing.kugou.com/index.php?action=userStorage";
    request.cookies = cookiesHelper_->GetNormalCookies();
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    queries["args"] = "[]";
    queries["&_"] = GetNowTimeString();

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
        assert(false && L"请求仓库数据失败");
        *errormsg = errorcode;
        return false;
    }

    Json::Value jvdata(Json::ValueType::arrayValue);
    Json::Value data = rootdata.get(std::string("data"), jvdata);
    if (data.isNull() || !data.isArray())
    {
        assert(false);
        return false;
    }

    for (auto& giftitem : data)
    {
        uint32 gift_count = GetInt32FromJsonValue(giftitem, "num");
        Json::Value iteminfo = giftitem.get("itemInfo", "");
        if (iteminfo.isNull() || !iteminfo.isObject())
        {
            assert(false);
            continue;
        }
        uint32 gift_id = GetInt32FromJsonValue(iteminfo, "id");
        std::string gift_name = iteminfo.get("name", "no name").asString();
        if (gift_id == TICKET_AWARD)
        {
            user_storage_info->gift_award += gift_count;
        }
        else if (gift_id == TICKET_SINGLE)
        {
            user_storage_info->gift_single += gift_count;
        }
    }

    std::string temp;
    LoginUServiceGetMyUserDataInfo(&temp);// 更新coin，丑陋一点先实现
    user_storage_info->accountname = username_;
    user_storage_info->nickname = nickname_;
    user_storage_info->rich_level = richlevel_;
    user_storage_info->coin = coin_;
    return true;
}

bool User::GetStarCount(uint32 room_id, uint32* star_count)
{
    std::string url = "http://fanxing.kugou.com/NServices/GiftStarService/GiftStarService/getInfo";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://fanxing.kugou.com/" + base::UintToString(room_id);
    request.cookies = cookiesHelper_->GetNormalCookies();
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    queries["args"] = "[]";
    queries["&_"] = GetNowTimeString();

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
        assert(false && L"请求星星数据失败");
        return false;
    }

    Json::Value jvdata(Json::ValueType::arrayValue);
    Json::Value data = rootdata.get(std::string("data"), jvdata);
    if (data.isNull() || !data.isObject())
    {
        assert(false);
        return false;
    }

    *star_count = GetInt32FromJsonValue(data, "starCount");
    return true;
}

bool User::ChangeNickname(const std::string& nickname, std::string* errormsg)
{
    ///UServices/UserService/UserService/setNickName?args=%5B%22smile%E7%9A%84%E5%B0%8F%E8%8B%97%E8%8B%971%22%5D&_=1477054242948
    std::string url = "http://fanxing.kugou.com/UServices/UserService/UserService/setNickName";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://fanxing.kugou.com";
    request.cookies = cookiesHelper_->GetNormalCookies();
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    queries["args"] = std::string("%5B%22") + nickname + std::string("%22%5D");
    queries["&_"] = GetNowTimeString();

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

    uint32 unixtime = GetInt32FromJsonValue(rootdata,"servertime");
    uint32 status = GetInt32FromJsonValue(rootdata, "status");
    std::string errorcode = rootdata.get("errorcode", "").asString();
    if (status != 1)
    {
        assert(false && L"改名失败");
        *errormsg = errorcode;
        return false;
    }
    return true;
}

bool User::ChangeLogo(const std::string& logo_path, std::string* errormsg)
{
    std::string url = "http://fanxing.kugou.com/UServices/UserService/UserService/setUserInfo";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://fanxing.kugou.com/index.php?action=userChangeLogo";
    request.cookies = cookiesHelper_->GetNormalCookies();
    if (ipproxy_.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        request.ipproxy = ipproxy_;

    auto& queries = request.queries;
    std::string args = std::string(R"([{"userLogo":")") + logo_path + R"("}])";
    queries["args"] = UrlEncode(args);
    queries["&_"] = GetNowTimeString();

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
        assert(false && L"更改头像失败");
        *errormsg = errorcode;
        return false;
    }
    return true;
}


bool User::Worship(uint32 roomid, uint32 userid, std::string* errormsg)
{
    std::vector<std::string> keys;
    keys.push_back("kg_mid");
    keys.push_back("LoginCheckCode");
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
    request.queries["args"] = "[%22" + base::IntToString(userid) + "%22,%22" +
        base::IntToString(roomid) + "%22]";
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

    uint32 unixtime = GetInt32FromJsonValue(rootdata, "servertime");
    uint32 status = rootdata.get("status", 0).asUInt();
    if (status != 1)
    {
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

bool User::LoginHttps(const std::string& username, const std::string& password, 
    const std::string& verifycode, std::string* errormsg)
{
    const char* loginuserurl = "https://login-user.kugou.com/v1/login/";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = loginuserurl;
    request.referer = "http://fanxing.kugou.com";
    std::vector<std::string> keys = { "LoginCheckCode", "kg_mid" };
    request.cookies = cookiesHelper_->GetCookies(keys);
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
    queries["callback"] = login_callback_string;
    queries["login_ver"] = "1";
    queries["mobile"] = "";
    queries["mobile_code"] = "";
    queries["mid"] = kg_mid_;
    queries["kguser_jv"] = "180925";

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
    std::string header = std::string(login_callback_string)+ "(";
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
    const char* GetMyUserDataInfoUrl = "http://fanxing.kugou.com/biz/UserPlat/UserService/UserService/getCurrentUserInfoV2";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = GetMyUserDataInfoUrl;
    request.referer = "http://fanxing.kugou.com";
    request.cookies = cookiesHelper_->GetNormalCookies();
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
    const char* GetMyUserDataInfoUrl = "http://fx.service.kugou.com/Services/IndexService/IndexService/getUserCenter";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = GetMyUserDataInfoUrl;
    request.referer = "http://fanxing.kugou.com";
    request.cookies = cookiesHelper_->GetNormalCookies();
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
