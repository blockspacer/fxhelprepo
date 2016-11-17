#include "UserTracker.h"

#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "Network/User.h"


UserTracker::UserTracker()
    :curl_wrapper_(new CurlWrapper)
{
}


UserTracker::~UserTracker()
{
}

void UserTracker::Test()
{
    LoginUser("fanxingtest002", "1233211234567");

    std::vector<uint32> roomids;
    GetAllStarRoomInfos(&roomids);
}

bool UserTracker::LoginUser(const std::string& user_name, const std::string& password)
{
    if (!user_)
        user_.reset(new User);

    std::string error_msg;
    if (!user_->Login(user_name, password, "", &error_msg))
    {
        return false;
    }

    return true;
}

bool UserTracker::GetUserLocationByUserId(uint32 user_id, uint32 room_id)
{
    return false;
}

//GET /UServices/UserService/UserExtService/getFollowList?args=[1,10,0,%22%22,0,3]&_=1478426799924
//Referer: http://fanxing.kugou.com/index.php?action=userFollowList
bool UserTracker::GetMyConcernList(std::vector<FollowUserInfo>* follow_user_infos)
{
    HttpRequest request;
    request.url = "http://fanxing.kugou.com/UServices/UserService/UserExtService/getFollowList";
    request.queries["args"] = "[1,10,0,%22%22,0,3]";
    request.queries["_"] = GetNowTimeString();
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/index.php?action=userFollowList";
    request.cookies = user_->GetCookies();

    HttpResponse response;
    if (!curl_wrapper_->Execute(request, &response))
    {
        return false;
    }


    
    return true;
}

bool UserTracker::GetUserInfoByUserId()
{
    return false;
}


bool UserTracker::GetUserConcernList()
{
    return false;
}

bool UserTracker::GetAllStarRoomInfos(std::vector<uint32>* roomids)
{   
    std::string url1 = "http://visitor.fanxing.kugou.com/VServices/IndexService.IndexService.getLiveList/1-1-1/";
    std::string url2 = "http://visitor.fanxing.kugou.com/VServices/IndexService.IndexService.getLiveList/1-2-1/";
    std::string url3 = "http://visitor.fanxing.kugou.com/VServices/IndexService.IndexService.getLiveList/1-3-1/";
    std::string url4 = "http://visitor.fanxing.kugou.com/VServices/IndexService.IndexService.getLiveList/1-4-1/";

    std::vector<uint32> roomid1;
    std::vector<uint32> roomid2;
    std::vector<uint32> roomid3;
    std::vector<uint32> roomid4;

    GetTargetStarRoomInfos(url1, &roomid1);
    GetTargetStarRoomInfos(url2, &roomid2);
    GetTargetStarRoomInfos(url3, &roomid3);
    GetTargetStarRoomInfos(url4, &roomid4);

    uint32 track_userid = 223370767;
    uint32 found_roomid = 0;
    std::vector<EnterRoomUserInfo> allEnterRoomUserInfoList;

    for (auto roomid : roomid1)
    {
        std::vector<EnterRoomUserInfo> enterRoomUserInfoList;
        if (!GetRoomViwerList(roomid, &enterRoomUserInfoList))
        {
            //assert(false); 可能在pk房，也会出现进房和获取房间成员失败
            continue;
        }

        for (auto userinfo : enterRoomUserInfoList)
        {
            if (userinfo.userid == track_userid)
            {
                found_roomid = roomid;
                break;
            }
        }
        if (found_roomid)
        {
            break;
        }
        allEnterRoomUserInfoList.insert(allEnterRoomUserInfoList.end(),
            enterRoomUserInfoList.begin(), enterRoomUserInfoList.end());
    }
    return true;
}

bool UserTracker::GetTargetStarRoomInfos(const std::string& url, std::vector<uint32>* roomids)
{
    // /VServices/IndexService.IndexService.getLiveList/1-3-1/
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/";
    request.cookies = user_->GetCookies();

    HttpResponse response;
    if (!curl_wrapper_->Execute(request, &response))
    {
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
    if (data.isNull() || !data.isArray())
    {
        assert(false);
        return false;
    }

    for (auto roominfo : data)
    {
        uint32 roomId = GetInt32FromJsonValue(roominfo, "roomId");
        roomids->push_back(roomId);
    }
    return true;
}

bool UserTracker::GetRoomViwerList(uint32 roomid, std::vector<EnterRoomUserInfo>* enterRoomUserInfoList)
{
    return user_->OpenRoomAndGetViewerList(roomid, enterRoomUserInfoList);
}

