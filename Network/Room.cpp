#include "Room.h"
#include "CurlWrapper.h"
#include "FanXingUI/GiftInfoHelper.h"
#include "GiftNotifyManager.h"
#include "CookiesHelper.h"
#include "EncodeHelper.h"

#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/json/json.h"

Room::Room(uint32 roomid)
    :roomid_(roomid)
    , curlWrapper_(new CurlWrapper)
    , giftNotifyManager_(new GiftNotifyManager)
    , cookiesHelper_(new CookiesHelper)
{
    giftNotifyManager_->Initialize();
}


Room::~Room()
{
    giftNotifyManager_->Finalize();
}

bool Room::Enter(const std::string& cookies)
{
    uint32 userid = 0;
    std::string nickname = "";
    uint32 richlevel = 0;
    uint32 ismaster = 0;
    uint32 singerid = 0;
    std::string key = "";
    std::string ext = "";

    if (!OpenRoom(cookies))
    {
        return false;
    }

    if (!GetCurrentUserInfo(cookies, &userid, &nickname, &richlevel))
    {
        return false;
    }

    if (!EnterRoom(cookies, &singerid, &key, &ext))
    {
        return false;
    }
    
    if (!ConnectToNotifyServer_(roomid_, userid, nickname, richlevel, ismaster, 
        singerid, key, ext))
    {
        return false;
    }
    return true;
}

bool Room::Exit()
{
    giftNotifyManager_->Finalize();
    return true;
}

void Room::SetNormalNotify(NormalNotify normalNotify)
{
    giftNotifyManager_->SetNormalNotify(normalNotify);
}

void Room::SetNotify201(Notify201 notify201)
{
    giftNotifyManager_->SetNotify201(notify201);
}

bool Room::OpenRoom(const std::string& cookies) const
{
    HttpRequest request;
    request.url = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid_);
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/";
    request.cookies = cookies;

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
    return true;
}

bool Room::EnterRoom(const std::string& cookies,
    uint32* singerid, std::string* key, std::string* ext)
{
    HttpRequest request;
    request.url = "http://fanxing.kugou.com/UServices/RoomService/RoomService/enterRoom";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/" + base::IntToString(roomid_);
    request.queries["_"] = GetNowTimeString();
    request.queries["args"] = "[%22" + 
        base::IntToString(static_cast<int>(roomid_))+
        "%22,%22%22,%22%22,%22web%22]";
    request.cookies = cookies;

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

    // 暂时没有必要检测status的值
    Json::Value dataObject(Json::objectValue);
    dataObject = rootdata.get(std::string("data"), dataObject);
    if (dataObject.empty())
    {
        return false;
    }

    Json::Value starDataObject(Json::objectValue);
    starDataObject = dataObject.get("starData", starDataObject);
    if (starDataObject.empty())
    {
        return false;
    }

    std::string strstarUserId = starDataObject["userId"].asString();
    if (!base::StringToUint(strstarUserId, singerid))
    {
        return false;
    }

    Json::Value socketConfigObject(Json::objectValue);
    socketConfigObject = dataObject.get("socketConfig", socketConfigObject);
    if (socketConfigObject.empty())
    {
        return false;
    }

    // 这是flash的tcp连接的服务器的ip(由url解析出来)和端口信息，后续可能用到，目前不解析
    std::string strsokt = socketConfigObject["sokt"].asString();

    // 重要数据，进入房间后连接tcp所使用的key
    *key = socketConfigObject["enter"].asString();
    *ext = socketConfigObject["ext"].asString();

    return true;
}

bool Room::GetCurrentUserInfo(const std::string& cookies,
    uint32* userid, std::string* nickname, uint32* richlevel)
{
    HttpRequest request;
    request.url = std::string("http://fanxing.kugou.com/Services.php");
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/" + base::IntToString(roomid_);;
    request.queries["d"] = GetNowTimeString();
    request.queries["act"] = "UserService.UserService";
    request.queries["mtd"] = "getCurrentUserInfo";
    request.queries["args"] = "%5B%5D";
    request.queries["test"] = "3";
    request.cookies = cookies;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string contents(response.content.begin(), response.content.end());

    if (contents.empty())
    {
        return false;
    }

    const std::string& data = contents;
    //解析json数据
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(data, rootdata, false))
    {
        return false;
    }

    // 有必要检测status的值
    uint32 status = rootdata.get(std::string("status"), 0).asInt();
    if (status == 0)
    {
        return false;
    }

    Json::Value dataObject(Json::objectValue);
    dataObject = rootdata.get(std::string("data"), dataObject);
    if (dataObject.empty())
    {
        return false;
    }

    Json::Value fxUserInfoObject(Json::objectValue);
    fxUserInfoObject = dataObject.get("fxUserInfo", fxUserInfoObject);
    if (fxUserInfoObject.empty())
    {
        return false;
    }

    std::string struserid = fxUserInfoObject["userId"].asString();
    if (!base::StringToUint(struserid, userid))
    {
        return false;
    }

    *nickname = fxUserInfoObject["nickName"].asString();
    std::string strrichLevel = fxUserInfoObject["richLevel"].asString();
    if (!base::StringToUint(strrichLevel, richlevel))
    {
        return false;
    }
    return true;
}

bool Room::ConnectToNotifyServer_(uint32 roomid, uint32 userid,
    const std::string& nickname,
    uint32 richlevel, uint32 ismaster,
    uint32 staruserid,
    const std::string& key,
    const std::string& ext)
{
    bool ret = false;
    ret = giftNotifyManager_->Connect843();
    assert(ret);

    // 多用户版本不需要处理信息回调
    //giftNotifyManager_->SetNotify201(
    //    std::bind(&NetworkHelper::NotifyCallback201,
    //    this, std::placeholders::_1));

    //giftNotifyManager_->SetNotify601(
    //    std::bind(&NetworkHelper::NotifyCallback601,
    //    this, roomid, staruserid, std::placeholders::_1));

    //giftNotifyManager_->SetNormalNotify(
    //    std::bind(&NetworkHelper::NotifyCallback,
    //    this, std::placeholders::_1));

    ret = giftNotifyManager_->Connect8080(roomid, userid, nickname, richlevel,
        ismaster, staruserid, key, ext);
    return ret;
}
