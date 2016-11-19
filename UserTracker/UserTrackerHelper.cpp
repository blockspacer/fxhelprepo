#include "stdafx.h"

#include "UserTrackerHelper.h"
#include "Network/User.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/bind.h"

UserTrackerHelper::UserTrackerHelper()
    :curl_wrapper_(new CurlWrapper)
    , worker_thread_(new base::Thread("UserTrackerHelper"))
{
}


UserTrackerHelper::~UserTrackerHelper()
{
}

bool UserTrackerHelper::Initialize()
{
    CurlWrapper::CurlInit();
    worker_thread_->Start();
    return true;
}

void UserTrackerHelper::Finalize()
{
    worker_thread_->Stop();
    CurlWrapper::CurlCleanup();
}

void UserTrackerHelper::Test()
{
    LoginUser("fanxingtest002", "1233211234567");

    std::vector<uint32> users = { 40297520 };

    std::map<uint32, uint32> user_room_map;
    UpdateForFindUser(users);

    int k = 123;
}

void UserTrackerHelper::SetNotifyMessageCallback(
    const base::Callback<void(const std::wstring&)> callback)
{
    message_callback_ = callback;
}

void UserTrackerHelper::CancelCurrentOperation()
{
    cancel_flag_ = true;
}

bool UserTrackerHelper::LoginUser(const std::string& user_name, const std::string& password)
{
    worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoLoginUser,
        base::Unretained(this), user_name, password));
    return true;
}

void UserTrackerHelper::DoLoginUser(const std::string& user_name, const std::string& password)
{
    if (!user_)
        user_.reset(new User);

    std::wstring msg;
    std::string error_msg;
    if (!user_->Login(user_name, password, "", &error_msg))
    {
        msg = L"登录失败";
        message_callback_.Run(msg);
        return;
    }
    msg = L"登录成功";
    message_callback_.Run(msg);
    return;
}

bool UserTrackerHelper::UpdataAllStarRoomUserMap()
{
    worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoUpdataAllStarRoomUserMap,
        base::Unretained(this)));
    return true;
}
void UserTrackerHelper::DoUpdataAllStarRoomUserMap()
{
    std::vector<uint32> roomids;
    std::wstring msg;
    if (!GetAllStarRoomInfos(&roomids))
    {
        msg = L"获取房间列表失败";
        message_callback_.Run(msg);
        return ;
    }
    msg = L"获取房间列表成功";
    message_callback_.Run(msg);
    std::map<uint32, std::map<uint32, EnterRoomUserInfo>> roomid_userid_map;
    if (!GetAllRoomViewers(roomids, &roomid_userid_map))
    {
        msg = L"获取所有房间观众列表失败";
        message_callback_.Run(msg);
        return ;
    }
    msg = L"获取所有房间观众列表成功";
    message_callback_.Run(msg);
    roomid_userid_map_ = roomid_userid_map;
    return ;
}

bool UserTrackerHelper::UpdateForFindUser(const std::vector<uint32> user_ids)
{
    worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoUpdateForFindUser,
        base::Unretained(this), user_ids));
    return true;
}
// 不使用缓存数据，边更新缓存边查找用户，只要找到就停止，不继续往后面找
void UserTrackerHelper::DoUpdateForFindUser(const std::vector<uint32> user_ids)
{
    std::wstring msg = L"更新并查找用户开始";
    message_callback_.Run(msg);
    std::vector<uint32> roomids;
    if (!GetAllStarRoomInfos(&roomids))
    {
        msg = L"获取房间列表失败";
        message_callback_.Run(msg);
        return;
    }
    msg = L"获取房间列表成功";
    message_callback_.Run(msg);
    std::map<uint32, std::map<uint32, EnterRoomUserInfo>> roomid_userid_map;
    std::map<uint32, uint32> user_room_map;
    FindUsersWhenRoomViewerList(roomids, &roomid_userid_map, user_ids, &user_room_map);

    for (auto user_room : user_room_map)
    {
        msg = L"用户[" + base::UintToString16(user_room.first) + L"]";
        msg += L"在房间[" + base::UintToString16(user_room.second) + L"]";
        message_callback_.Run(msg);
    }

    // 扫了多少更新多少到目前的数据里
    for (auto temp : roomid_userid_map)
    {
        roomid_userid_map_[temp.first] = temp.second;
    }
    msg = L"更新房间数[" + base::UintToString16(roomid_userid_map.size()) + L"]";
    message_callback_.Run(msg);
    msg = L"更新并查找用户结束";
    message_callback_.Run(msg);
    return;
}

bool UserTrackerHelper::GetUserLocationByUserId(const std::vector<uint32> user_ids)
{
    worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoGetUserLocationByUserId,
        base::Unretained(this), user_ids));
    return true;
}

void UserTrackerHelper::DoGetUserLocationByUserId(const std::vector<uint32> user_ids)
{
    if (roomid_userid_map_.empty())
        return;

    std::wstring msg = L"在缓存中查找用户开始";
    message_callback_.Run(msg);
    std::map<uint32, uint32> user_room_map;
    for (const auto &room : roomid_userid_map_)
    {
        for (const auto& user_id : user_ids)
        {
            auto it = room.second.find(user_id);
            if (it != room.second.end())
            {
                user_room_map[user_id] = room.first;
            }
        }     
    }

    for (auto user_room : user_room_map)
    {
        msg = L"用户[" + base::UintToString16(user_room.first) + L"]";
        msg += L"在房间[" + base::UintToString16(user_room.second) + L"]";
        message_callback_.Run(msg);
    }
    msg = L"在缓存中查找用户结束";
    message_callback_.Run(msg);
    return;
}

bool UserTrackerHelper::GetAllStarRoomInfos(std::vector<uint32>* roomids)
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

bool UserTrackerHelper::GetTargetStarRoomInfos(const std::string& url, std::vector<uint32>* roomids)
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

    // 暂时只限制2016年11月30号前能使用，防止外泄
    if (unixtime > 1480435200)
    {
        return false;
    }

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

bool UserTrackerHelper::GetAllRoomViewers(
    const std::vector<uint32>& roomids,
    std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map)
{
    if (!roomid_user_map)
        return false;

    cancel_flag_ = false;
    std::wstring msg;
    for (auto roomid : roomids)
    {
        if (cancel_flag_)
        {
            msg = L"用户取消当前操作";
            message_callback_.Run(msg);
            cancel_flag_ = false;
            return true;
        }

        msg = L"开始获取房间[" + base::UintToString16(roomid) + L"] 观众";
        message_callback_.Run(msg);
        std::map<uint32, EnterRoomUserInfo> roomid_users;
        if (!GetRoomViewerList(roomid, &roomid_users))
        {
            //assert(false); 可能在pk房，也会出现进房和获取房间成员失败
            msg = L"获取房间[" + base::UintToString16(roomid) + L"] 观众失败";
            message_callback_.Run(msg);
            continue;
        }
        msg = L"获取房间[" + base::UintToString16(roomid) + L"] 观众成功";
        message_callback_.Run(msg);
        (*roomid_user_map)[roomid] = roomid_users;
    }

    return !roomid_user_map->empty();
}

bool UserTrackerHelper::FindUsersWhenRoomViewerList(const std::vector<uint32>& roomids,
    std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
    const std::vector<uint32>& user_ids,
    std::map<uint32, uint32>* user_room_map)
{
    cancel_flag_ = false;
    std::wstring msg;
    std::vector<uint32> temp_user_ids = user_ids;
    for (auto roomid : roomids)
    {
        if (cancel_flag_)
        {
            msg = L"用户取消当前操作";
            message_callback_.Run(msg);
            cancel_flag_ = false;
            return true;
        }

        msg = L"获取房间[" + base::UintToString16(roomid) + L"] 观众开始";
        message_callback_.Run(msg);
        std::map<uint32, EnterRoomUserInfo> roomid_users;
        if (!GetRoomViewerList(roomid, &roomid_users))
        {
            //assert(false); 可能在pk房，也会出现进房和获取房间成员失败
            msg = L"获取房间[" + base::UintToString16(roomid) + L"] 观众失败";
            message_callback_.Run(msg);
            continue;
        }
        msg = L"获取房间[" + base::UintToString16(roomid) + L"] 观众成功";
        message_callback_.Run(msg);

        (*roomid_user_map)[roomid] = roomid_users;

        for (auto user_it = temp_user_ids.begin();
            user_it != temp_user_ids.end(); user_it++)
        {
            if (roomid_users.end() != roomid_users.find(*user_it))
            {
                (*user_room_map)[*user_it] = roomid;
                msg = L"在房间[" + base::UintToString16(roomid) + L"] 找到目标 [" + base::UintToString16(*user_it) + L"]";
                message_callback_.Run(msg);
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

bool UserTrackerHelper::GetRoomViewerList(uint32 roomid, std::map<uint32, EnterRoomUserInfo>* user_map)
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

//GET /UServices/UserService/UserExtService/getFollowList?args=[1,10,0,%22%22,0,3]&_=1478426799924
//Referer: http://fanxing.kugou.com/index.php?action=userFollowList
bool UserTrackerHelper::GetMyConcernList(std::vector<FollowUserInfo>* follow_user_infos)
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

bool UserTrackerHelper::GetUserInfoByUserId()
{
    return false;
}


bool UserTrackerHelper::GetUserConcernList()
{
    return false;
}




