#include "stdafx.h"

#include "UserTrackerHelper.h"
#include "Network/User.h"
#include "Network/CurlWrapper.h"
#include "Network/easy_http_impl.h"
#include "Network/EncodeHelper.h"
#include "AuthorityHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/bind.h"

#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_enumerator.h"

namespace
{
    bool GetOutputFileName(base::FilePath* outpath)
    {
        base::FilePath path;
        PathService::Get(base::DIR_EXE, &path);

        path = path.Append(L"搜索官方主播结果.txt");
        *outpath = path;
        return true;
    }
}

UserTrackerHelper::UserTrackerHelper()
    :curl_wrapper_(new CurlWrapper)
    , easy_http_impl_(new EasyHttpImpl)
    , worker_thread_(new base::Thread("UserTrackerHelper"))
    , user_(new User()) //必须先登录再让操作
{
}


UserTrackerHelper::~UserTrackerHelper()
{
}

bool UserTrackerHelper::Initialize()
{
    CurlWrapper::CurlInit();
    tracker_authority_.reset(new UserTrackerAuthority);
    AuthorityHelper authority_helper;
    authority_helper.LoadUserTrackerAuthority(tracker_authority_.get());
    authority_helper.GetTrackerAuthorityDisplayInfo(
        *tracker_authority_.get(), &authority_msg_);

    worker_thread_->Start();   
    return true;
}

void UserTrackerHelper::Finalize()
{
    easy_http_impl_->ShutdownService();
    worker_thread_->Stop();
    CurlWrapper::CurlCleanup();
}

void UserTrackerHelper::Test()
{
}

void UserTrackerHelper::SetNotifyMessageCallback(
    const base::Callback<void(const std::wstring&)> callback)
{
    message_callback_ = callback;
}

void UserTrackerHelper::SetSearchConfig(bool check_star, bool check_diamon,
    bool check_1_3_crown, bool check_4_crown_up)
{
    check_star_ = check_star;
    check_diamon_ = check_diamon;
    check_1_3_crown_ = check_1_3_crown;
    check_4_crown_up_ = check_4_crown_up;
}

std::wstring UserTrackerHelper::GetAuthorityMessage() const
{
    return authority_msg_;
}

void UserTrackerHelper::CancelCurrentOperation()
{
    cancel_flag_ = true;
}

bool UserTrackerHelper::LoginGetVerifyCode(std::vector<uint8>* picture)
{
    return user_->LoginGetVerifyCode(picture);
}

bool UserTrackerHelper::LoginUser(
    const std::string& user_name, const std::string& password,
    const std::string& verifycode,
    const base::Callback<void(bool, uint32, const std::string&)>& callback)
{
    return worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoLoginUser,
        base::Unretained(this), user_name, password, verifycode, callback));
}

void UserTrackerHelper::DoLoginUser(const std::string& user_name, 
    const std::string& password, const std::string& verifycode,
    const base::Callback<void(bool, uint32, const std::string&)>& callback)
{
    if (!user_)
        user_.reset(new User);

    std::wstring msg;
    std::string error_msg;
    if (!user_->Login(user_name, password, verifycode, &error_msg))
    {
        msg = L"登录失败." + base::UTF8ToWide(error_msg);;
        message_callback_.Run(msg);
        uint32 servertime = user_->GetServerTime();
        callback.Run(false, servertime, base::WideToUTF8(msg));
        return;
    }

    uint32 servertime = user_->GetServerTime();
    msg = L"登录成功";
    message_callback_.Run(msg);

#ifndef _DEBUG
    //if (tracker_authority_->user_id != user_->GetFanxingId())
    //{
    //    msg = authority_msg_ + L". 当前用户未授权";
    //    message_callback_.Run(msg);
    //    callback.Run(false, servertime, base::WideToUTF8(msg));
    //    return;
    //}

    //uint64 expiretime = tracker_authority_->expiretime - base::Time::UnixEpoch().ToInternalValue();
    //expiretime /= 1000000;
    //if (servertime > expiretime)
    //{
    //    msg = authority_msg_ + L"用户授权已到期，请续费!";
    //    message_callback_.Run(msg);
    //    callback.Run(false, servertime, base::WideToUTF8(msg));
    //    return;
    //}
#endif    
    callback.Run(true, servertime, error_msg);
    return;
}

bool UserTrackerHelper::UpdataAllStarRoomUserMap(
    const base::Callback<void(uint32, uint32)>& callback)
{
    if (!user_)
        return false;

    worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoUpdataAllStarRoomUserMap,
        base::Unretained(this), callback));
    return true;
}

void UserTrackerHelper::DoUpdataAllStarRoomUserMap(
    const base::Callback<void(uint32, uint32)>& callback)
{
    std::vector<uint32> roomids;
    std::wstring msg;
    if (!GetAllStarRoomInfos(&roomids))
    {
        msg = L"获取房间列表失败";
        message_callback_.Run(msg);
        return;
    }
    msg = L"获取房间列表成功";
    message_callback_.Run(msg);
    if (!GetAllRoomViewers(roomids, &roomid_userid_map_, callback))
    {
        msg = L"获取所有房间观众列表失败";
        message_callback_.Run(msg);
        return;
    }
    msg = L"获取所有房间观众列表成功";
    message_callback_.Run(msg);
    return;
}

bool UserTrackerHelper::UpdataAllStarRoomForNoClan(
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    if (!user_)
        return false;

    worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoUpdataAllStarRoomForNoClan,
        base::Unretained(this), progress_callback, result_callback));
    return true;
}

void UserTrackerHelper::DoUpdataAllStarRoomForNoClan(
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    std::vector<uint32> roomids;
    std::wstring msg;
    if (!GetAllStarRoomInfos(&roomids))
    {
        msg = L"获取房间列表失败";
        message_callback_.Run(msg);
        return;
    }
    msg = L"获取房间列表成功";
    message_callback_.Run(msg);

    uint32 count = 0;
    std::map<uint32, std::map<uint32, ConsumerInfo>> consumer_infos_map;
    for (auto roomid : roomids)
    {
        std::map<uint32, ConsumerInfo> consumer_infos;
        progress_callback.Run(++count, roomids.size());
        if (!GetRoomConsumerList(roomid, &consumer_infos))
            continue;

        consumer_infos_map[roomid] = consumer_infos;
    }

    base::FilePath path;
    if (!GetOutputFileName(&path))
        return;

    base::File file;
    file.Initialize(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    for (auto consumer_infos : consumer_infos_map)
    {
        std::string roomid = base::UintToString(consumer_infos.first);
        LOG(INFO) << "=======" << roomid;
        file.WriteAtCurrentPos(roomid.c_str(), roomid.size());
        file.WriteAtCurrentPos("\r\n", 2);
        result_callback.Run(0, consumer_infos.first);
    }
    file.Close();
    msg = L"获取主播成功";
    message_callback_.Run(msg);
    return;
}

bool UserTrackerHelper::UpdateForFindUser(
    const std::vector<uint32> user_ids,
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    if (!user_)
        return false;

    worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoUpdateForFindUser,
        base::Unretained(this), user_ids, progress_callback, result_callback));
    return true;
}

// 不使用缓存数据，边更新缓存边查找用户，只要找到就停止，不继续往后面找
void UserTrackerHelper::DoUpdateForFindUser(
    const std::vector<uint32> user_ids,
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
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
    FindUsersWhenUpdateRoomViewerList(roomids, &roomid_userid_map, 
        user_ids, &user_room_map, progress_callback, result_callback);

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

bool UserTrackerHelper::GetUserLocationByUserId(
    const std::vector<uint32> user_ids,
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    if (!user_)
        return false;

    worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoGetUserLocationByUserId,
        base::Unretained(this), user_ids, progress_callback, result_callback));
    return true;
}

void UserTrackerHelper::DoGetUserLocationByUserId(
    const std::vector<uint32> user_ids,
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    if (roomid_userid_map_.empty())
        return;

    std::wstring msg = L"在缓存中查找用户开始";
    message_callback_.Run(msg);
    std::map<uint32, uint32> user_room_map;
    uint32 all = roomid_userid_map_.size();
    uint32 current = 0;
    for (const auto &room : roomid_userid_map_)
    {
        current++;
        for (const auto& user_id : user_ids)
        {
            auto it = room.second.find(user_id);
            if (it != room.second.end())
            {
                user_room_map[user_id] = room.first;
                // 避免漏掉一个用户在多个房间的情况
                msg = L"用户[" + base::UintToString16(user_id) + L"]";
                msg += L"在房间[" + base::UintToString16(room.first) + L"]";
                message_callback_.Run(msg);
                result_callback.Run(user_id, room.first);
            }
        }
        progress_callback.Run(current, all);
    }

    msg = L"在缓存中查找用户结束";
    message_callback_.Run(msg);
    return;
}

bool UserTrackerHelper::ClearCache()
{
    if (!user_)
        return false;

    return worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoClearCache,
        base::Unretained(this)));
}

void UserTrackerHelper::DoClearCache()
{
    roomid_userid_map_.clear();
}

bool UserTrackerHelper::GetAllBeautyStarForNoClan(
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    return false;
}

bool UserTrackerHelper::GetAllStarRoomInfos(std::vector<uint32>* roomids)
{
    
    std::vector<uint32> roomid1;
    std::vector<uint32> roomid2;
    std::vector<uint32> roomid3;
    std::vector<uint32> roomid4;

    //const std::string& host = tracker_authority_->tracker_host;
    const std::string host = "visitor.fanxing.kugou.com";
    std::string url = "http://" + host + "/VServices/IndexService.IndexService.getLiveList";
    if (check_star_)
    {
        std::string url1 = url + "/1-1-1/";
        std::wstring msg = L"正在获取星级主播房间列表 ...";
        message_callback_.Run(msg);
        GetTargetStarRoomInfos(url1, &roomid1);
        roomids->insert(roomids->end(), roomid1.begin(), roomid1.end());
    }

    if (check_diamon_)
    {
        std::string url2 = url + "/1-2-1/";
        std::wstring msg = L"正在获取钻级主播房间列表 ...";
        message_callback_.Run(msg);
        GetTargetStarRoomInfos(url2, &roomid2);
        roomids->insert(roomids->end(), roomid2.begin(), roomid2.end());
    }

    if (check_1_3_crown_)
    {
        std::string url3 = url + "/1-3-1/";
        std::wstring msg = L"正在获取1-4冠主播房间列表 ...";
        message_callback_.Run(msg);
        GetTargetStarRoomInfos(url3, &roomid3);
        roomids->insert(roomids->end(), roomid3.begin(), roomid3.end());
    }

    if (check_4_crown_up_)
    {
        std::string url4 = url + "/1-4-1/";
        std::wstring msg = L"正在获取5冠以上主播房间列表 ...";
        message_callback_.Run(msg);
        GetTargetStarRoomInfos(url4, &roomid4);
        roomids->insert(roomids->end(), roomid4.begin(), roomid4.end());
    }

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

    // 暂时只限制2017年1月1号前能使用，防止外泄
    //if (unixtime > 1483203600)
    //{
    //    return false;
    //}

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
        uint32 startId = GetInt32FromJsonValue(roominfo, "userId");
        roomids->push_back(roomId);
    }
    return true;
}

bool UserTrackerHelper::GetAllRoomViewers(
    const std::vector<uint32>& roomids,
    std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
    const base::Callback<void(uint32, uint32)>& callback)
{
    if (!roomid_user_map)
        return false;

    cancel_flag_ = false;
    std::wstring msg;
    //uint32 all = roomids.size();
    //uint32 current = 0;
    all_room_count_ = roomids.size();
    current_room_count_ = 0;

    for (auto roomid : roomids)
    {
        if (cancel_flag_)
        {
            msg = L"用户取消当前操作";
            message_callback_.Run(msg);
            cancel_flag_ = false;
            return true;
        }

        //msg = L"开始获取房间[" + base::UintToString16(roomid) + L"] 观众";
        //message_callback_.Run(msg);
        std::map<uint32, EnterRoomUserInfo> roomid_users;
        if (!AsyncOpenRoom(roomid, callback))
        {
            //assert(false); 可能在pk房，也会出现进房和获取房间成员失败
            //msg = L"获取房间[" + base::UintToString16(roomid) + L"] 观众失败";
            //message_callback_.Run(msg);
            //callback.Run(current, all);
            continue;
        }
        //msg = L"获取房间[" + base::UintToString16(roomid) + L"] 观众成功";
        //message_callback_.Run(msg);
        //callback.Run(current, all);
        //(*roomid_user_map)[roomid] = roomid_users;
    }

    //return !roomid_user_map->empty();
    return true;
}

bool UserTrackerHelper::FindUsersWhenUpdateRoomViewerList(
    const std::vector<uint32>& roomids,
    std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
    const std::vector<uint32>& user_ids,
    std::map<uint32, uint32>* user_room_map,
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    cancel_flag_ = false;
    std::wstring msg;
    std::vector<uint32> temp_user_ids = user_ids;
    uint32 all = roomids.size();
    uint32 current = 0;
    for (auto roomid : roomids)
    {
        current++;
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
            progress_callback.Run(current, all);
            continue;
        }
        msg = L"获取房间[" + base::UintToString16(roomid) + L"] 观众成功";
        message_callback_.Run(msg);
        progress_callback.Run(current, all);

        (*roomid_user_map)[roomid] = roomid_users;

        for (auto user_it = temp_user_ids.begin();
            user_it != temp_user_ids.end(); user_it++)
        {
            if (roomid_users.end() != roomid_users.find(*user_it))
            {
                (*user_room_map)[*user_it] = roomid;
                msg = L"在房间[" + base::UintToString16(roomid) + 
                    L"] 找到目标 [" + base::UintToString16(*user_it) + L"]";
                message_callback_.Run(msg);
                result_callback.Run(*user_it, roomid);
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

bool UserTrackerHelper::GetRoomConsumerList(uint32 roomid, std::map<uint32, ConsumerInfo>* consumers_map)
{
    std::vector<ConsumerInfo> consumer_infos;
    if (!user_->OpenRoomAndGetConsumerList(roomid, &consumer_infos))
    {
        return false;
    }

    for (auto user_info : consumer_infos)
    {
        (*consumers_map)[user_info.fanxing_id] = user_info;
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

bool UserTrackerHelper::AsyncOpenRoom(
    uint32 roomid, const base::Callback<void(uint32, uint32)>& progress_callback)
{
    auto callback = std::bind(&UserTrackerHelper::OpenRoomCallback,
        this, roomid, progress_callback, std::placeholders::_1);
    HttpRequest request;
    request.url = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid);
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/";
    request.cookies = user_->GetCookies();
    request.asyncHttpResponseCallback = callback;

    return easy_http_impl_->AsyncHttpRequest(request);  
}

// 切换线程回来处理
void UserTrackerHelper::OpenRoomCallback(uint32 roomid,
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const HttpResponse& response)
{
    if ( base::MessageLoop::current()!= worker_thread_->message_loop())
    {
        worker_thread_->message_loop()->PostTask(FROM_HERE,
            base::Bind(&UserTrackerHelper::OpenRoomCallback,
            base::Unretained(this), roomid, progress_callback, response));
        return;
    }

    std::string content;
    content.assign(response.content.begin(), response.content.end());
    do
    {
        if (content.empty())
        {
            break;
        }
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
        std::string temp = content.substr(beginPos + 1, endPos - beginPos - 1);
        RemoveSpace(&temp);
        if (temp.length() < 6)
        {
            break;
        }
        std::string str_singerid = temp.substr(1, temp.length() - 2);
        uint32 singerid = 0;
        base::StringToUint(str_singerid, &singerid);

        // 成功获取主播信息，继续走下一步
        worker_thread_->message_loop()->PostTask(
            FROM_HERE,
            base::Bind(&UserTrackerHelper::AsyncGetRoomViewerList,
            base::Unretained(this), roomid, singerid, progress_callback));
        return;        
    } while (0);

    // 获取主播信息失败，放弃房间
    std::wstring msg = L"获取房间[" + base::UintToString16(roomid) + L"] 信息-失败";
    message_callback_.Run(msg);
    progress_callback.Run(++current_room_count_, all_room_count_);
}

void UserTrackerHelper::AsyncGetRoomViewerList(uint32 roomid, uint32 singerid,
    const base::Callback<void(uint32, uint32)>& progress_callback)
{
    std::string url = "http://visitor.fanxing.kugou.com";
    url += "/VServices/RoomService.RoomService.getViewerList/";
    url += base::UintToString(roomid) + "-" + base::UintToString(singerid);
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid);
    request.cookies = user_->GetCookies();

    auto callback = std::bind(&UserTrackerHelper::GetRoomViewerListCallback,
        this, roomid, progress_callback, std::placeholders::_1);

    request.asyncHttpResponseCallback = callback;

    easy_http_impl_->AsyncHttpRequest(request);
}

// 切换线程回来处理
void UserTrackerHelper::GetRoomViewerListCallback(uint32 roomid,
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const HttpResponse& response)
{
    if (base::MessageLoop::current() != worker_thread_->message_loop())
    {
        worker_thread_->message_loop()->PostTask(FROM_HERE,
            base::Bind(&UserTrackerHelper::GetRoomViewerListCallback,
            base::Unretained(this), roomid, progress_callback, response));

        return;
    }

    std::wstring msg = L"获取房间[" + base::UintToString16(roomid) + L"] 观众-";
    do 
    {
        if (response.content.empty())
        {
            assert(false);
            break;
        }

        std::string responsedata(response.content.begin(), response.content.end());
        std::string jsondata = PickJson(responsedata);

        Json::Reader reader;
        Json::Value rootdata(Json::objectValue);
        if (!reader.parse(jsondata, rootdata, false))
        {
            assert(false);
            break;
        }

        uint32 unixtime = rootdata.get("servertime", 1461378689).asUInt();
        uint32 status = rootdata.get("status", 0).asUInt();
        if (status != 1)
        {
            assert(false);
            break;
        }
        Json::Value jvdata(Json::ValueType::objectValue);
        Json::Value data = rootdata.get(std::string("data"), jvdata);
        if (data.isNull() || !data.isObject())
        {
            assert(false);
            break;
        }

        Json::Value jvlist(Json::ValueType::objectValue);
        Json::Value list = data.get(std::string("list"), jvlist);
        if (!list.isArray())
        {
            assert(false);
            break;
        }

        roomid_userid_map_[roomid].clear();
        for (const auto& item : list)
        {
            EnterRoomUserInfo enterRoomUserInfo;
            enterRoomUserInfo.nickname = item.get("nickname", "").asString();
            enterRoomUserInfo.richlevel = GetInt32FromJsonValue(item, "richlevel");
            enterRoomUserInfo.userid = GetInt32FromJsonValue(item, "userid");
            enterRoomUserInfo.unixtime = unixtime;
            enterRoomUserInfo.roomid = roomid;
            roomid_userid_map_[roomid].insert(std::make_pair(enterRoomUserInfo.userid, enterRoomUserInfo));
        }
        msg += L"成功";
        message_callback_.Run(msg);
        progress_callback.Run(++current_room_count_, all_room_count_);
        return;
    } while (0);

    // 获取主播信息失败，放弃房间
    msg += L"失败";
    message_callback_.Run(msg);
    progress_callback.Run(++current_room_count_, all_room_count_);
}




