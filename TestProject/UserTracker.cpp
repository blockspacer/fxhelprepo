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

    std::vector<uint32> users = {81603503};

    std::map<uint32, uint32> user_room_map;
    UpdateForFindUser(users, &user_room_map);

    int k = 123;
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

bool UserTracker::UpdataAllStarRoomUserMap()
{
    std::vector<uint32> roomids;
    if (!GetAllStarRoomInfos(&roomids))
    {
        return false;
    }

    std::map<uint32, std::map<uint32, EnterRoomUserInfo>> roomid_userid_map;
    if (!GetAllRoomViewers(roomids, &roomid_userid_map))
    {
        return false;
    }

    roomid_userid_map_ = roomid_userid_map;
    return true;
}

// 不使用缓存数据，边更新缓存边查找用户，只要找到就停止，不继续往后面找
bool UserTracker::UpdateForFindUser(const std::vector<uint32> user_ids,
    std::map<uint32, uint32>* user_room_map)
{
    std::vector<uint32> roomids;
    if (!GetAllStarRoomInfos(&roomids))
    {
        return false;
    }
    std::map<uint32, std::map<uint32, EnterRoomUserInfo>> roomid_userid_map;
    FindUsersWhenRoomViewerList(roomids, &roomid_userid_map, user_ids, user_room_map);

    // 扫了多少更新多少到目前的数据里
    for (auto temp : roomid_userid_map)
    {
        roomid_userid_map_[temp.first] = temp.second;
    }

    return true;
}

bool UserTracker::GetUserLocationByUserId(const std::vector<uint32> user_ids,
    std::map<uint32, uint32>* user_room_map)
{
    if (roomid_userid_map_.empty())
        return false;

    for (const auto &room : roomid_userid_map_)
    {
        for (const auto& user_id : user_ids)
        {
            auto it = room.second.find(user_id);
            if (it != room.second.end())
            {
                (*user_room_map)[user_id] = room.first;
            }
        }     
    }
    return true;
}

bool UserTracker::UpdataAllStarRoomComsumerMap()
{
    std::vector<uint32> roomids;
    if (!GetAllStarRoomInfos(&roomids))
    {
        return false;
    }

    std::map<uint32, std::map<uint32, ConsumerInfo>> roomid_consumer_map;
    if (!GetAllRoomConsumers(roomids, &roomid_consumer_map))
    {
        return false;
    }

    roomid_consumer_map_ = roomid_consumer_map;
    return true;
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

    roomids->insert(roomids->end(), roomid1.begin(), roomid1.end());
    roomids->insert(roomids->end(), roomid2.begin(), roomid2.end());
    roomids->insert(roomids->end(), roomid3.begin(), roomid3.end());
    roomids->insert(roomids->end(), roomid4.begin(), roomid4.end());

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

bool UserTracker::GetAllRoomViewers(
    const std::vector<uint32>& roomids,
    std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map)
{
    if (!roomid_user_map)
        return false;

    for (auto roomid : roomids)
    {
        std::map<uint32, EnterRoomUserInfo> roomid_users;
        if (!GetRoomViewerList(roomid, &roomid_users))
        {
            //assert(false); 可能在pk房，也会出现进房和获取房间成员失败
            continue;
        }
        (*roomid_user_map)[roomid] = roomid_users;
    }

    return !roomid_user_map->empty();
}

bool UserTracker::GetAllRoomConsumers(const std::vector<uint32>& roomids,
    std::map<uint32, std::map<uint32, ConsumerInfo>>* roomid_consumer_map)
{
    if (!roomid_consumer_map)
        return false;

    for (auto roomid : roomids)
    {
        std::map<uint32, ConsumerInfo> roomid_consumers;
        if (!GetConsumerList(roomid, &roomid_consumers))
        {
            //assert(false); 可能在pk房，也会出现进房和获取房间成员失败
            continue;
        }
        (*roomid_consumer_map)[roomid] = roomid_consumers;
    }

    return !roomid_consumer_map->empty();
}

bool UserTracker::FindUsersWhenRoomViewerList(const std::vector<uint32>& roomids,
    std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
    const std::vector<uint32>& user_ids,
    std::map<uint32, uint32>* user_room_map)
{
    std::vector<uint32> temp_user_ids = user_ids;
    for (auto roomid : roomids)
    {
        std::map<uint32, EnterRoomUserInfo> roomid_users;
        if (!GetRoomViewerList(roomid, &roomid_users))
        {
            //assert(false); 可能在pk房，也会出现进房和获取房间成员失败
            continue;
        }

        (*roomid_user_map)[roomid] = roomid_users;

        for (auto user_it = temp_user_ids.begin();
            user_it != temp_user_ids.end(); user_it++)
        {
            if (roomid_users.end() != roomid_users.find(*user_it))
            {
                (*user_room_map)[*user_it] = roomid;
                temp_user_ids.erase(user_it);
                break;
            }
        }

        if (temp_user_ids.empty())
        {
            return true; // 全部找到了
        } 
    }
    return false; // 没有找到全部
}

bool UserTracker::GetRoomViewerList(uint32 roomid, std::map<uint32, EnterRoomUserInfo>* user_map)
{
    std::vector<EnterRoomUserInfo> enterRoomUserInfoList;
    if (!user_->OpenRoomAndGetViewerList(roomid, &enterRoomUserInfoList))
    {
        return false;
    }
    
    for (auto user_info : enterRoomUserInfoList)
    {
        (*user_map)[user_info.userid] = user_info;
    }
    return true;
}

bool UserTracker::GetConsumerList(uint32 roomid,
    std::map<uint32, ConsumerInfo>* consumer_map)
{
    //std::vector<ConsumerInfo> consumer_info;
    //if (!user_->OpenRoomAndGetConsumerList(roomid, &consumer_info))
    //{
    //    return false;
    //}

    //for (auto user : consumer_info)
    //{
    //    (*consumer_map)[user.fanxing_id] = user;
    //}
    return true;
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




