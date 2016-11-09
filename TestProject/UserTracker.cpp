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
    return false;
}

bool UserTracker::GetRoomViwerList(uint32 roomid, std::vector<EnterRoomUserInfo>* enterRoomUserInfoList)
{
    return user_->OpenRoomAndGetViewerList(roomid, enterRoomUserInfoList);
}

