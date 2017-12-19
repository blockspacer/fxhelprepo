#include "stdafx.h"

#include "UserTrackerHelper.h"
#include "SingerInfo.h"

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
#include "third_party/chromium/base/strings/string_split.h"

namespace
{
    const uint32 max_http_error_count = 2;
    bool GetGoodSinger(const std::string& responsedata)
    {
        std::string good_voice_target = base::WideToUTF8(L"好声音");
        auto beginpos = responsedata.find(good_voice_target);
        if (beginpos != std::string::npos)
            return true;

        std::string beauty_target = base::WideToUTF8(L"女神");
        beginpos = responsedata.find(beauty_target);
        if (beginpos != std::string::npos)
            return true;

        std::string special_target = base::WideToUTF8(L"歌手");
        beginpos = responsedata.find(special_target);
        if (beginpos != std::string::npos)
            return true;

        return false;
    }

    bool GetJsonKeyValue(const std::string& content, int begin_pos,
        const std::string& key, std::string* out)
    {
        auto key_pos = content.find(key, begin_pos);

        auto value_pos = content.find(':', key_pos);
        value_pos += 1;
        auto end_pos = content.find(',', value_pos);
        std::string temp = content.substr(value_pos, end_pos - value_pos);

        RemoveSpace(&temp);
        temp = temp.substr(1, temp.length() - 2);
        *out = temp;
        return !temp.empty();
    }

    bool GetJsonKeyValue(const std::string& content, int begin_pos,
        const std::string& key, uint32* out)
    {
        auto key_pos = content.find(key, begin_pos);

        auto value_pos = content.find(':', key_pos);
        value_pos += 1;
        auto end_pos = content.find(',', value_pos);
        std::string temp = content.substr(value_pos, end_pos - value_pos);

        RemoveSpace(&temp);
        temp = temp.substr(1, temp.length() - 2);
        
        return base::StringToUint(temp, out);
    }

    bool LastOnlineStringToDays(const std::wstring& last_string, uint32* days)
    {
        int endpos = -1;
        if ((endpos = last_string.find(L"天前")) != std::string::npos)
        {
            std::wstring day_string = last_string.substr(0, endpos);
            uint32 last_online_day = 0;
            if (!base::StringToUint(day_string, &last_online_day))
                return false;
            *days = last_online_day;
        }
        else if (last_string == L"没有直播")
        {
            *days = 999999;
        }
        else if ((endpos = last_string.find(L"月前")) != std::string::npos)
        {
            std::wstring month_string = last_string.substr(0, endpos);
            uint32 last_online_month = 0;
            if (!base::StringToUint(month_string, &last_online_month))
                return false;
            *days = last_online_month * 30;
        }
        else if (last_string.find(L"时") != std::string::npos ||
            last_string.find(L"分") != std::string::npos||
            last_string.find(L"秒") != std::string::npos)
        {
            *days = 0;
        }
        else
        {
            return false;
        }
        return true;
    }
}

//static
bool UserTrackerHelper::GetOutputFileName(base::FilePath* outpath)
{
    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    std::wstring time = base::UTF8ToWide(GetNowTimeString());
    std::wstring filename = L"搜索官方主播结果";
    filename += time + L".txt";
    path = path.Append(filename);
    *outpath = path;
    return true;
}

UserTrackerHelper::UserTrackerHelper()
    :curl_wrapper_(new CurlWrapper)
    , easy_http_impl_(new EasyHttpImpl)
    , worker_thread_(new base::Thread("UserTrackerHelper"))
    , user_(new User()) //必须先登录再让操作
    , database_(new SingerInfoDatabase())
{
    recode_date_ = base::UTF8ToWide(MakeFormatDateString(base::Time::Now()));
}


UserTrackerHelper::~UserTrackerHelper()
{
}

bool UserTrackerHelper::Initialize()
{
    CurlWrapper::CurlInit();
    tracker_authority_.reset(new UserTrackerAuthority);
    AuthorityHelper authority_helper;
    if (!authority_helper.LoadUserTrackerAuthority(tracker_authority_.get()))
    {
        return false;
    }
    
    authority_helper.GetTrackerAuthorityDisplayInfo(
        *tracker_authority_.get(), &authority_msg_);

    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    path = path.Append(L"fanxing_singer.db");

    database_->Initialize(path.value());
    worker_thread_->Start();   
    return true;
}

void UserTrackerHelper::Finalize()
{
    easy_http_impl_->ShutdownService();
    worker_thread_->Stop();
    database_->Finalize();
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

void UserTrackerHelper::SetRangeSearchCallback(ProgressCallback progress_callback,
    ResultCallback result_callback)
{
    progress_callback_ = progress_callback;
    result_callback_ = result_callback;
    handle_callback_ = base::Bind(&UserTrackerHelper::RangeSearchResultToDB,
        base::Unretained(this));
}

void UserTrackerHelper::SetSearchConfig(uint32 min_star_level, 
    bool check_star, bool check_diamon,
    bool check_1_3_crown, bool check_4_crown_up)
{
    min_star_level_ = min_star_level;
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

bool UserTrackerHelper::UpdatePhoneForNoClan(
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    if (!user_)
        return false;

    worker_thread_->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserTrackerHelper::DoUpdatePhoneForNoClan,
        base::Unretained(this), progress_callback, result_callback));
    return true;
}

void UserTrackerHelper::DoUpdatePhoneForNoClan(
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    std::vector<uint32> roomids;
    std::wstring msg;
    if (!GetPhoneRoomInfos(&roomids))
    {
        msg = L"获取房间列表失败";
        message_callback_.Run(msg);
        return;
    }
    msg = L"获取房间列表成功";
    message_callback_.Run(msg);

    //uint32 count = 0;
    //std::map<uint32, std::map<uint32, ConsumerInfo>> consumer_infos_map;
    //for (auto roomid : roomids)
    //{
    //    std::map<uint32, ConsumerInfo> consumer_infos;
    //    progress_callback.Run(++count, roomids.size());
    //    uint32 star_level = 0;
    //    if (!GetRoomConsumerList(roomid, &star_level, &consumer_infos))
    //        continue;

    //    if (star_level < min_star_level_)
    //        continue;

    //    consumer_infos_map[roomid] = consumer_infos;
    //    result_callback.Run(0, roomid);
    //}

    //base::FilePath path;
    //if (!GetOutputFileName(&path))
    //    return;

    //base::File file;
    //file.Initialize(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    //for (auto consumer_infos : consumer_infos_map)
    //{
    //    std::string roomid = base::UintToString(consumer_infos.first);
    //    LOG(INFO) << "=======" << roomid;
    //    file.WriteAtCurrentPos(roomid.c_str(), roomid.size());
    //    file.WriteAtCurrentPos("\r\n", 2);
    //}
    //file.Close();
    //msg = L"获取主播成功";
    //message_callback_.Run(msg);
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
        uint32 star_level = 0;
        if (!GetRoomConsumerList(roomid, &star_level, &consumer_infos))
            continue;

        if (star_level < min_star_level_)
            continue;

        consumer_infos_map[roomid] = consumer_infos;
        result_callback.Run(0, roomid);
    }

    base::FilePath path;
    if (!GetOutputFileName(&path))
        return;

    // 特殊处理，11111111是放出去的测试用户，不导出房间号
    if (tracker_authority_->user_id != 11111111)
    {
        base::File file;
        file.Initialize(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
        for (auto consumer_infos : consumer_infos_map)
        {
            std::string roomid = base::UintToString(consumer_infos.first);
            LOG(INFO) << "=======" << roomid;
            file.WriteAtCurrentPos(roomid.c_str(), roomid.size());
            file.WriteAtCurrentPos("\r\n", 2);
        }
        file.Close();
    }
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

bool UserTrackerHelper::GetAllBeautyStarForNoClan(uint32 days,
    const base::Callback<void(uint32, uint32)>& progress_callback,
    const base::Callback<void(uint32, uint32)>& result_callback)
{
    std::vector<DisplayRoomInfo> all_roomids;

    ///VServices/IndexService.IndexService.getRoomListByTagsId/20-1-20/
    for (uint32 page_number = 1; page_number++; )
    {
        std::string url = "http://visitor.fanxing.kugou.com/VServices/IndexService.IndexService.getRoomListByTagsId/20-";
        url += base::UintToString(page_number) + "-20";
        std::vector<DisplayRoomInfo> roomids;
        if (!GetTargetStarRoomInfos(url, &roomids))
            break;

        all_roomids.insert(all_roomids.end(), roomids.begin(), roomids.end());

        if (roomids.size()<20) // 到最后一页了
            break;
    }
    
    std::vector<DisplayRoomInfo> result_room;
    uint32 all_count = all_roomids.size();
    uint32 count = 0;
    for (const auto& room : all_roomids)
    {
        progress_callback.Run(++count, all_count);
        if (room.last_online > days) // 接近一年没播的主播
        {
            result_room.push_back(room);
            result_callback.Run(0, room.roomid);
        }
            
    }

    return true;
}

bool UserTrackerHelper::RunSearchHotKey(const std::wstring& hotkey, uint32 times,
    const base::Callback<void(uint32, uint32)>& progress_callback)
{
    //worker_thread_->message_loop()->PostTask(FROM_HERE,
    //    base::Bind(&UserTrackerHelper::DoSearchHotKey,
    //    base::Unretained(this), hotkey, times, progress_callback));
    return true;
}
//
//void UserTrackerHelper::DoSearchHotKey(const std::wstring& hotkey, uint32 times,
//    const base::Callback<void(uint32, uint32)>& progress_callback)
//{
//    search_hot_key_count_ = 0;
//    auto callback = std::bind(&UserTrackerHelper::SearchHotKeyCallback,
//        this, times, progress_callback, std::placeholders::_1);
//    HttpRequest request;
//    request.url = std::string("http://service.fanxing.kugou.com/pt_search/pcsearch/v1/hot_words.jsonp");
//    request.queries["keywords"] = base::WideToUTF8(hotkey);
//    request.queries["num"] = "5";
//    request.queries["_t"] = GetNowTimeString();
//    request.queries["callback"] = "jsonpcallback_httpservicefanxingkugoucompt_searchpcsearchv1type_alljsonpkeywordsE5BEAEE7AC91nums168200_t1509013785090";
//
//    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
//    request.referer = "http://fanxing.kugou.com/";
//    request.cookies = user_->GetCookies();
//    request.asyncHttpResponseCallback = callback;
//
//    easy_http_impl_->AsyncHttpRequest(request);
//}

void UserTrackerHelper::RunSearchRoomIdRange(
    uint32 roomid_min, uint32 roomid_max)
{
    if (!worker_thread_->task_runner()->BelongsToCurrentThread())
    {
        worker_thread_->message_loop()->PostTask(FROM_HERE,
            base::Bind(&UserTrackerHelper::RunSearchRoomIdRange,
            base::Unretained(this), roomid_min, roomid_max));
        return;
    }

    if (roomid_max < roomid_min)
        return;

    std::vector<uint32> roomids;
    std::wstring msg;
    for (uint32 roomid = roomid_min; roomid <= roomid_max; roomid++)
    {
        roomids.push_back(roomid);
    }
    all_room_count_ = roomids.size();
    current_room_count_ = 0;

    msg = L"开始转移任务";
    message_callback_.Run(msg);
    uint32 count = 0;

    room_error_map_.clear();
    status_map_.clear();
    for (auto roomid : roomids)
    {
        progress_callback_.Run(++count, roomids.size());
        std::string singerid;
        DoOpenRoomForGetSingerid(roomid);
    }

    msg = L"转移任务成功";
    message_callback_.Run(msg);

    return;
}

bool UserTrackerHelper::SaveRooms(const std::vector<std::wstring>& roomids)
{
    base::FilePath path;
    if (!GetOutputFileName(&path))
        return false;

    base::File file;
    file.Initialize(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    for (auto roomid : roomids)
    {
        std::string str_roomid = base::WideToUTF8(roomid);
        LOG(INFO) << "=======" << roomid;
        file.WriteAtCurrentPos(str_roomid.c_str(), str_roomid.size());
        file.WriteAtCurrentPos("\r\n", 2);
    }
    file.Close();
    return true;
}

bool UserTrackerHelper::LoadRooms(const std::wstring& path_file_name, 
    std::vector<std::wstring>* roomids)
{
    base::FilePath path(path_file_name);
    base::File userfile(path, base::File::FLAG_OPEN | base::File::FLAG_READ);
    if (!userfile.IsValid())
    {
        return false;
    }
    const uint32 memlen = 1024;
    std::string data;
    char str[memlen] = { 0 };
    userfile.Seek(base::File::FROM_BEGIN, 0);
    int read = userfile.ReadAtCurrentPos(str, memlen);
    DWORD err = GetLastError();
    while (read > 0)
    {
        data.insert(data.end(), str, str + read);
        if (read < memlen)//读完了
            break;
        read = userfile.ReadAtCurrentPos(str, memlen);
    }

    assert(!data.empty());

    std::vector<std::string> rooms = SplitString(data, "\r\n");
    uint32 total = rooms.size();
    for (const auto& it : rooms)
    {
        std::string roomstr = it;
        RemoveSpace(&roomstr);
        uint32 roomid = 0;
        if (!base::StringToUint(roomstr, &roomid))
        {
            //assert(false && L"房间号转换错误");
            LOG(ERROR) << L"roomid change error! " << it;
            continue;
        }
        std::wstring w_room_id = base::UintToString16(roomid);
        roomids->push_back(w_room_id);
    }
    return true;
}

bool UserTrackerHelper::DoOpenRoomForGetSingerid(uint32 roomid)
{
    auto cb = [=](const HttpResponse& response)->void
    {
        SingerInfo singer_info;

        std::string content;
        content.assign(response.content.begin(), response.content.end());

        bool result = false;
        do
        {
            // 打开房间的功能不需要处理返回来的页面数据
            if (content.empty())
                break;

            // 普通房间
            std::string liveInitData = "var liveInitData";

            auto live_pos = content.find(liveInitData);
            if (live_pos == std::string::npos)
                break;

            uint32 fxid;
            if (!GetJsonKeyValue(content, live_pos, "starId", &fxid))
                break;

            singer_info.user_info.fanxing_id = fxid;

            std::string nickname;
            if (!GetJsonKeyValue(content, live_pos, "starName", &nickname))
                break;

            singer_info.user_info.nickname = base::UTF8ToWide(nickname);

            uint32 star_level = 0;
            if (!GetJsonKeyValue(content, live_pos, "starLevel", &star_level))
                break;

            singer_info.user_info.star_level = star_level;

            uint32 kgid = 0;
            if (!GetJsonKeyValue(content, live_pos, "starKugouId", &kgid))
                break;

            singer_info.user_info.kugou_id = kgid;

            uint32 room_id = 0;
            if (!GetJsonKeyValue(content, live_pos, "roomId", &room_id))
                break;

            singer_info.room_info.room_id = room_id;
            DCHECK(roomid == room_id);
            result = true;
        } while (0);

        std::wstring str_roomid = base::UintToString16(roomid);
        singer_info.date = recode_date_;
        if (!result)
        {
            std::wstring msg = str_roomid + L"无法获取房间信息";
            message_callback_.Run(msg);
            result_callback_.Run(roomid, singer_info, response.statuscode, RangSearchErrorCode::RS_ROOM_OPEN_FAILED);
            return;
        }

        std::wstring msg = str_roomid + L"获取房间信息成功===============";
        message_callback_.Run(msg);

        // 继续下一个流程，获取上次上播时间

        worker_thread_->task_runner()->PostTask(
            FROM_HERE,
            base::Bind(base::IgnoreResult(&UserTrackerHelper::DoGetSingerLastOnline),
            base::Unretained(this),
            singer_info));
    };
    HttpRequest request;
    request.url = std::string("http://fanxing.kugou.com/") +
        base::UintToString(roomid);
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/";
    request.asyncHttpResponseCallback = cb;

    return easy_http_impl_->AsyncHttpRequest(request);
}

bool UserTrackerHelper::DoGetSingerLastOnline(SingerInfo singer_info)
{
    auto callback = [=] (const HttpResponse& response)->void
    {
        SingerInfo temp = singer_info;
        std::string content;
        content.assign(response.content.begin(), response.content.end());

        bool result = false;
        do
        {
            if (response.content.empty())
                break;

            std::string responsedata(response.content.begin(), response.content.end());

            std::string clan_tag("/index.php?action=clan&id=");
            auto clan_pos = responsedata.find(clan_tag);
            if (clan_pos != std::string::npos) // 如果是公会艺人
            {
                clan_pos += clan_tag.size();
                std::string clan = responsedata.substr(clan_pos, 4);
                uint32 clanid = 0;
                if (!base::StringToUint(clan, &clanid))
                {
                    DCHECK(clanid && L"转换错误");
                }
                temp.user_info.clan_id = clanid;
            }

            std::string target = base::WideToUTF8(L"上次直播");
            auto beginpos = responsedata.find(target);
            if (beginpos == std::string::npos)
                break;

            beginpos += target.size();
            beginpos = responsedata.find("<dd>", beginpos);
            beginpos += 4;
            auto endpos = responsedata.find("</dd>", beginpos);
            if (endpos == std::string::npos)
                break;

            std::string time_string = responsedata.substr(beginpos, endpos - beginpos);

            uint32 last_day = 0;
            if (!LastOnlineStringToDays(base::UTF8ToWide(time_string), &last_day))
                break;;

            std::string dd_input = "<dd class=\"inputArea \">";
            // 非必选项资料
            std::string sex_string = base::WideToUTF8(L"<dt>性<span class=\"blank24\"></span>别：</dt>");
            beginpos = responsedata.find(sex_string);
            if (beginpos != std::string::npos)
            {
                beginpos += sex_string.size();
                beginpos = responsedata.find(dd_input, beginpos);
                beginpos += dd_input.size();
                endpos = responsedata.find("</dd>", beginpos);
                if (endpos != std::string::npos)
                {
                    sex_string = responsedata.substr(beginpos, endpos - beginpos);
                    RemoveSpace(&sex_string);
                    temp.user_info.sex = base::UTF8ToWide(sex_string);
                }
            }

            std::string location_str = base::WideToUTF8(L"<dt>所 在 地：</dt>");
            beginpos = responsedata.find(location_str);
            if (beginpos != std::string::npos)
            {
                beginpos += location_str.size();
                beginpos = responsedata.find(dd_input, beginpos);
                beginpos += dd_input.size();
                endpos = responsedata.find("</dd>", beginpos);
                if (endpos != std::string::npos)
                {
                    location_str = responsedata.substr(beginpos, endpos - beginpos);
                    RemoveSpace(&location_str);
                    temp.user_info.location = base::UTF8ToWide(location_str);
                }
            }

            std::string room_addr_str = base::WideToUTF8(L"直播间地址");
            // <a href="/1439683" target="room_1439683">
            beginpos = responsedata.find(room_addr_str);
            if (beginpos != std::string::npos)
            {
                std::string href_mark = "href=\"/";
                beginpos += room_addr_str.size();
                beginpos = responsedata.find(href_mark, beginpos);
                beginpos += href_mark.size();
                endpos = responsedata.find("\"", beginpos);
                if (endpos != std::string::npos)
                {
                    room_addr_str = responsedata.substr(beginpos, endpos - beginpos);
                    RemoveSpace(&room_addr_str);
                    bool ret = base::StringToUint(room_addr_str, &temp.room_info.room_id);
                    DCHECK(ret);
                }
            }

            std::wstring msg = base::Uint64ToString16(temp.room_info.phone_room_id) 
                + L"(" + base::UintToString16(temp.room_info.room_id) + L")" 
                + L":" + base::UTF8ToWide(time_string);
            message_callback_.Run(msg);

            temp.last_online = base::UTF8ToWide(time_string);
            temp.last_online_day = last_day;

            result = true;
        } while (0);

        if (!result)
        {
            handle_callback_.Run(singer_info.room_info.phone_room_id, singer_info,
                response.statuscode, RangSearchErrorCode::RS_GET_LAST_ONLINE_FAILED);
            return;
        }

        worker_thread_->task_runner()->PostTask(
            FROM_HERE,
            base::Bind(&UserTrackerHelper::DoGetSingerTags,
            base::Unretained(this),
            temp));
    };

    std::string url = "http://fanxing.kugou.com";
    url += "/index.php";
    HttpRequest request;
    request.url = url;
    request.queries["action"] = "user";
    std::string singerid = base::UintToString(singer_info.user_info.fanxing_id);
    request.queries["id"] = singerid;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;

    request.asyncHttpResponseCallback = callback;

    return easy_http_impl_->AsyncHttpRequest(request);
}

// 如果也满足标签要求，就回调为目标房间号
void UserTrackerHelper::DoGetSingerTags(SingerInfo singer_info)
{
    auto callback = [=](const HttpResponse& response)
    {
        SingerInfo temp = singer_info;
        std::string content;
        content.assign(response.content.begin(), response.content.end());
        std::string jsondata = PickJson(content);

        bool result = false;
        do 
        {
            Json::Reader reader;
            Json::Value rootdata(Json::objectValue);
            if (!reader.parse(jsondata, rootdata, false))
                break;

            uint32 unixtime = rootdata.get("servertime", 1506009600).asUInt();

            Json::Value jvdata(Json::ValueType::objectValue);
            Json::Value data = rootdata.get(std::string("data"), jvdata);
            if (data.isNull() || !data.isArray())
                break;

            std::wstring tags;
            for (const auto& tag : data)
            {
                std::string tag_name = tag.get("tagsName", "").asString();
                std::wstring w_tag_name = base::UTF8ToWide(tag_name);
                if (!w_tag_name.empty())
                    tags += w_tag_name + L"_";
            }

            if (!tags.empty())
                temp.tags = tags;
            
            result = true;
        } while (0);

        worker_thread_->task_runner()->PostTask(
            FROM_HERE,
            base::Bind(&UserTrackerHelper::DoGetRoomMessage,
            base::Unretained(this),
            temp));
    };


    std::string star_kugou_id = base::UintToString(singer_info.user_info.kugou_id);
    std::string url = "http://fx.service.kugou.com";
    url += "/VServices/StarTagsService.StarTagsService.getStarUserTagsList/";
    url += star_kugou_id + "/";
    HttpRequest request;
    request.url = url;
    request.queries["jsoncallback"] = std::string("jsonpcallback_httpsfxservicekugoucomVServicesStarTagsServiceStarTagsServicegetStarUserTagsList")+star_kugou_id;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;

    request.asyncHttpResponseCallback = callback;

    easy_http_impl_->AsyncHttpRequest(request);
}

void UserTrackerHelper::DoGetRoomMessage(SingerInfo singer_info)
{
    auto callback = [=](const HttpResponse& response)
    {
        SingerInfo temp = singer_info;
        std::string content;
        content.assign(response.content.begin(), response.content.end());
        std::string jsondata = PickJson(content);

        bool result = false;
        do
        {
            Json::Reader reader;
            Json::Value rootdata(Json::objectValue);
            if (!reader.parse(jsondata, rootdata, false))
                break;

            uint32 unixtime = rootdata.get("servertime", 1506009600).asUInt();

            Json::Value jvdata(Json::ValueType::objectValue);
            Json::Value data = rootdata.get(std::string("data"), jvdata);
            if (data.isNull() || !data.isObject())
                break;

            std::string public_msg = data.get("publicMesg", "").asString();
            std::string private_msg = data.get("privateMesg", "").asString();
            temp.room_info.public_msg = base::UTF8ToWide(public_msg);
            temp.room_info.private_msg = base::UTF8ToWide(private_msg);

            result = true;
        } while (0);

        worker_thread_->task_runner()->PostTask(
            FROM_HERE,
            base::Bind(&UserTrackerHelper::DoGetSuperFans,
            base::Unretained(this),
            temp));
    };
    std::string roomid = base::UintToString(singer_info.room_info.room_id);
    std::string url = "http://visitor.fanxing.kugou.com";
    url += "/VServices/RoomService.RoomService.getInfo/";
    url += roomid;
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;

    request.asyncHttpResponseCallback = callback;

    easy_http_impl_->AsyncHttpRequest(request);
}

void UserTrackerHelper::DoGetSuperFans(SingerInfo singer_info)
{
    auto callback = [=](const HttpResponse& response)
    {
        SingerInfo temp = singer_info;
        std::string content;
        content.assign(response.content.begin(), response.content.end());
        std::string jsondata = PickJson(content);

        bool result = false;
        do
        {
            Json::Reader reader;
            Json::Value rootdata(Json::objectValue);
            if (!reader.parse(jsondata, rootdata, false))
                break;

            uint32 unixtime = rootdata.get("servertime", 1506009600).asUInt();

            Json::Value jvdata(Json::ValueType::objectValue);
            Json::Value data = rootdata.get(std::string("data"), jvdata);

            if (data.isNull() || !data.isArray())
                break;

            std::vector<uint32> fansids;
            for (const auto& fans : data)
            {
                // FIX ME: 目前为了追踪被盗币是否上30天榜，暂时存玩家id
                uint32 fansid = GetInt32FromJsonValue(fans, "senderId");
                uint32 send_coin = GetInt32FromJsonValue(fans, "total");
                std::string fansnickname = fans.get("nickName", "").asString();
                fansids.push_back(fansid);
            }
            if (fansids.size() == 5)
            {
                temp.room_info.billboard_all_5 = fansids.at(4);
            }

            if (fansids.size() >= 4)
            {
                temp.room_info.billboard_all_4 = fansids.at(3);
            }

            if (fansids.size() >= 3)
            {
                temp.room_info.billboard_all_3 = fansids.at(2);
            }

            if (fansids.size() >= 2)
            {
                temp.room_info.billboard_all_2 = fansids.at(1);
            }

            if (fansids.size() >= 1)
            {
                temp.room_info.billboard_all_1 = fansids.at(0);
            }

            result = true;
        } while (0);

        worker_thread_->task_runner()->PostTask(
            FROM_HERE,
            base::Bind(&UserTrackerHelper::DoGetThirtydays,
            base::Unretained(this),
            temp));
    };
    std::string fanxing_id = base::UintToString(singer_info.user_info.fanxing_id);
    std::string url = "http://visitor.fanxing.kugou.com/VServices/ChartService.FansService.getSuperFans/";
    url += fanxing_id;
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;

    request.asyncHttpResponseCallback = callback;

    easy_http_impl_->AsyncHttpRequest(request);
}

void UserTrackerHelper::DoGetThirtydays(SingerInfo singer_info)
{
    auto callback = [=](const HttpResponse& response)
    {
        SingerInfo temp = singer_info;
        std::string content;
        content.assign(response.content.begin(), response.content.end());
        std::string jsondata = PickJson(content);

        bool result = false;
        do
        {
            Json::Reader reader;
            Json::Value rootdata(Json::objectValue);
            if (!reader.parse(jsondata, rootdata, false))
                break;

            uint32 unixtime = rootdata.get("servertime", 1506009600).asUInt();

            Json::Value jvdata(Json::ValueType::objectValue);
            Json::Value data = rootdata.get(std::string("data"), jvdata);

            if (data.isNull() || !data.isArray())
                break;

            std::vector<uint32> fansids;
            for (const auto& fans : data)
            {
                // FIX ME: 目前为了追踪被盗币是否上30天榜，暂时存玩家id
                uint32 fansid = GetInt32FromJsonValue(fans, "senderId");
                uint32 send_coin = GetInt32FromJsonValue(fans, "total");
                std::string fansnickname = fans.get("nickName", "").asString();
                fansids.push_back(fansid);
            }
            if (fansids.size() == 5)
            {
                temp.room_info.billboard_month_5 = fansids.at(4);
            }

            if (fansids.size() >= 4)
            {
                temp.room_info.billboard_month_4 = fansids.at(3);
            }

            if (fansids.size() >= 3)
            {
                temp.room_info.billboard_month_3 = fansids.at(2);
            }

            if (fansids.size() >= 2)
            {
                temp.room_info.billboard_month_2 = fansids.at(1);
            }

            if (fansids.size() >= 1)
            {
                temp.room_info.billboard_month_1 = fansids.at(0);
            }

            result = true;
        } while (0);

        handle_callback_.Run(temp.room_info.phone_room_id,
            temp, response.statuscode, RangSearchErrorCode::RS_OK);
    };
    std::string fanxing_id = base::UintToString(singer_info.user_info.fanxing_id);
    std::string url = "http://visitor.fanxing.kugou.com/VServices/ChartService.FansService.getSuperFans/";
    url += fanxing_id;
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;

    request.asyncHttpResponseCallback = callback;

    easy_http_impl_->AsyncHttpRequest(request);
}

void UserTrackerHelper::RangeSearchResultToDB(uint32 roomid, const SingerInfo& singer_info,
    uint32 status, RangSearchErrorCode error)
{
    status_map_[status]++;

    if (error == RangSearchErrorCode::RS_OK)
    {
        std::wstring retry;
        if (room_error_map_.end()!=room_error_map_.find(roomid))
        {
            // 重试成功
            retry = L"重试成功";
        }
        std::wstring msg = base::UintToString16(roomid) + L"请求成功  " + retry;
        message_callback_.Run(msg);
        DCHECK(singer_info.user_info.fanxing_id);
        bool result = database_->InsertRecord(singer_info);
        DCHECK(result);
        progress_callback_.Run(++current_room_count_, all_room_count_);

        bool result_to_callback = false;
        if (singer_info.room_info.room_id == 0)
        {
            // 没有roomid代表没有申请过正式直播，是玩家身份，不论是否在公会，都可以申请直播
            result_to_callback = true;
        }
        else if (singer_info.user_info.clan_id == 0)
        {
            // 如果有正式房间号，但是没有公会，那么判断90天时间
            if (singer_info.last_online_day>=90)
            {
                result_to_callback = true;
            }
        }
        else // 同时有公会有房间号，就不在考虑范围了
        {
            result_to_callback = false;
        }

        if (result_to_callback)
        {
            result_callback_.Run(roomid, singer_info, status, error);
        }

        return;
    }

    if (status == 488 || status/100==5) // 服务器认为是攻击，拒绝服务，要稍后再试一次
    {
        uint32 error_count = room_error_map_[roomid]++;

        if (error_count > max_http_error_count)
        {
            progress_callback_.Run(++current_room_count_, all_room_count_);
            return;
        }

        std::wstring msg = base::UintToString16(roomid) + L"请求错误,准备重试第" + base::UintToString16(error_count) +L"次, ec = " + base::IntToString16((int)error);
        message_callback_.Run(msg);
        worker_thread_->task_runner()->PostDelayedTask(
            FROM_HERE, base::Bind(
            base::IgnoreResult(&UserTrackerHelper::DoOpenRoomForGetSingerid),
            base::Unretained(this),roomid),
            base::TimeDelta::FromSeconds(3));
        return;
    }
    std::wstring msg = L"房间号" + base::UintToString16(roomid) + L"请求失败，放弃重试";
    message_callback_.Run(msg);
    progress_callback_.Run(++current_room_count_, all_room_count_);
}
//void UserTrackerHelper::SearchRoomIdRangeCallback(uint32 all_times,
//    const base::Callback<void(uint32, uint32)>& progress_callback,
//    const HttpResponse& response)
//{
//    search_hot_key_count_++;
//    progress_callback.Run(search_hot_key_count_, all_times);
//}

//void UserTrackerHelper::SearchHotKeyCallback(uint32 all_times, 
//    const base::Callback<void(uint32, uint32)>& progress_callback,
//    const HttpResponse& response)
//{
//    search_hot_key_count_++;
//    progress_callback.Run(search_hot_key_count_, all_times);
//}

bool UserTrackerHelper::GetDanceRoomInfos(std::vector<uint32>* roomids)
{
    // 暂时不实现，没实际利用价值
    return false;
}
bool UserTrackerHelper::GetPhoneRoomInfos(std::vector<uint32>* roomids)
{
    std::vector<DisplayRoomInfo> roomid1;

    std::wstring msg = L"正在获取手机主播房间列表 ...";
    message_callback_.Run(msg);
    bool has_next = true;
    uint32 page_num = 1;

    while (has_next)
    {
        bool result = GetPhoneTargetStarRoomInfos(page_num++, &roomid1, &has_next);
        DCHECK(result);
    }


    all_room_count_ = roomid1.size();
    //std::vector<uint32> tempids;
    //for (const auto& room : roomid1)
    //{
    //    if (room.status != 0)
    //        tempids.push_back(room.roomid);
    //}
    //roomids->insert(roomids->end(), tempids.begin(), tempids.end());

    for (const auto& room : roomid1)
    {
        SingerInfo singer_info;
        singer_info.room_info.phone_room_id = room.phone_room_id;
        singer_info.room_info.room_id = room.roomid;
        singer_info.user_info.star_level = room.star_level;
        singer_info.user_info.fanxing_id = room.starid;
        singer_info.user_info.kugou_id = room.kugouid;
        singer_info.user_info.nickname = base::UTF8ToWide(room.nickname);
        DoGetSingerLastOnline(singer_info);
    }

    return true;
}

bool UserTrackerHelper::GetAllStarRoomInfos(std::vector<uint32>* roomids)
{
    if (is_expired)
        return false;

    std::vector<DisplayRoomInfo> roomid1;
    std::vector<DisplayRoomInfo> roomid2;
    std::vector<DisplayRoomInfo> roomid3;
    std::vector<DisplayRoomInfo> roomid4;

    const std::string& host = tracker_authority_->tracker_host;
    //const std::string host = "visitor.fanxing.kugou.com";
    std::string url = "http://" + host + "/VServices/IndexService.IndexService.getLiveList";
    if (check_star_)
    {
        std::string url1 = url + "/1-1-1/";
        std::wstring msg = L"正在获取星级主播房间列表 ...";
        message_callback_.Run(msg);
        GetTargetStarRoomInfos(url1, &roomid1);

        std::vector<uint32> tempids;
        for (const auto& room : roomid1)
        {
            if (room.status != 0)
                tempids.push_back(room.roomid);
        }
        roomids->insert(roomids->end(), tempids.begin(), tempids.end());
    }

    if (check_diamon_)
    {
        std::string url2 = url + "/1-2-1/";
        std::wstring msg = L"正在获取钻级主播房间列表 ...";
        message_callback_.Run(msg);
        GetTargetStarRoomInfos(url2, &roomid2);
        std::vector<uint32> tempids;
        for (const auto& room : roomid2)
        {
            if (room.status != 0)
                tempids.push_back(room.roomid);
        }
        roomids->insert(roomids->end(), tempids.begin(), tempids.end());
    }

    if (check_1_3_crown_)
    {
        std::string url3 = url + "/1-3-1/";
        std::wstring msg = L"正在获取1-4冠主播房间列表 ...";
        message_callback_.Run(msg);
        GetTargetStarRoomInfos(url3, &roomid3);
        std::vector<uint32> tempids;
        for (const auto& room : roomid3)
        {
            if (room.status != 0)
                tempids.push_back(room.roomid);
        }
        roomids->insert(roomids->end(), tempids.begin(), tempids.end());
    }

    if (check_4_crown_up_)
    {
        std::string url4 = url + "/1-4-1/";
        std::wstring msg = L"正在获取5冠以上主播房间列表 ...";
        message_callback_.Run(msg);
        GetTargetStarRoomInfos(url4, &roomid4);
        std::vector<uint32> tempids;
        for (const auto& room : roomid4)
        {
            if (room.status != 0)
                tempids.push_back(room.roomid);
        }
        roomids->insert(roomids->end(), tempids.begin(), tempids.end());
    }

    return true;
}


bool UserTrackerHelper::GetPhoneTargetStarRoomInfos(
    uint32 page_num, std::vector<DisplayRoomInfo>* roomids,
    bool* has_next)
{
    if (is_expired)
        return false;

    const std::string host = "fx.service.kugou.com";
    std::string url = "http://" + host + "/mps-web/cdn/mobileLive/roomList_v2";
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.referer = "http://fanxing.kugou.com/";
    request.cookies = user_->GetCookies();
    request.queries["pid"] = "85";
    request.queries["version"] = "1234";
    request.queries["pageNum"] = base::UintToString(page_num);
    request.queries["pageSize"] = "20";
    request.queries["jsonpcallback"] = std::string("jsonpcallback_httpsfxservicekugoucommpswebcdnmobileLiveroomList_v2pid85version1234pageNum")
        + base::UintToString(page_num) + "pageSize20";

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

    uint32 unixtime = rootdata.get("servertime", 1506009600).asUInt();

    uint64 expiretime = tracker_authority_->expiretime - base::Time::UnixEpoch().ToInternalValue();
    expiretime /= 1000000;

#ifndef _DEBUG
    if (unixtime > expiretime)
    {
        is_expired = true;
        AuthorityHelper authority_helper;
        if (!authority_helper.DestoryTrackAuthority())
        {
            return false;
        }

        return false;
    }
#endif

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

    uint32 hasNext = data.get("hasNext", 0).asUInt();
    *has_next = !!hasNext;

    Json::Value liveStarTypeList = data.get(std::string("liveStarTypeList"), jvdata);
    if (liveStarTypeList.isNull() || !liveStarTypeList.isArray())
    {
        assert(false);
        return false;
    }

    for (const auto& roominfo : liveStarTypeList)
    {
        uint32 roomId = GetInt32FromJsonValue(roominfo, "roomId");
        uint32 starId = GetInt32FromJsonValue(roominfo, "userId");
        uint32 kugouId = GetInt32FromJsonValue(roominfo, "kugouId");
        uint32 status = GetInt32FromJsonValue(roominfo, "status");
        uint32 star_level = GetInt32FromJsonValue(roominfo, "starLevel");
        std::string nickName = roominfo.get("nickName", "").asString();
        uint32 last_online = 0;

        DisplayRoomInfo display;
        display.phone_room_id = roomId;
        display.starid = starId;
        display.star_level = star_level;
        display.last_online = last_online;
        display.kugouid = kugouId;
        display.nickname = nickName;
        roomids->push_back(display);
    }
    return true;
}

bool UserTrackerHelper::GetTargetStarRoomInfos(const std::string& url, std::vector<DisplayRoomInfo>* roomids)
{
    if (is_expired)
        return false;

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

    uint32 unixtime = rootdata.get("servertime", 1506009600).asUInt();

    uint64 expiretime = tracker_authority_->expiretime - base::Time::UnixEpoch().ToInternalValue();
    expiretime /= 1000000;

#ifndef _DEBUG
    if (unixtime > expiretime)
    {
        is_expired = true;
        AuthorityHelper authority_helper;
        if (!authority_helper.DestoryTrackAuthority())
        {
            return false;
        }

        return false;
    }
#endif

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

    for (const auto& roominfo : data)
    {
        uint32 roomId = GetInt32FromJsonValue(roominfo, "roomId");
        uint32 starId = GetInt32FromJsonValue(roominfo, "userId");
        uint32 status = GetInt32FromJsonValue(roominfo, "status");
        uint32 star_level = GetInt32FromJsonValue(roominfo, "starLevel");
        uint32 last_online = 0;
        std::string str_last_online = roominfo.get("startTime", "").asString();
        // 转化为天数
        std::string day = base::WideToUTF8(L"天");
        std::string hour = base::WideToUTF8(L"时");
        if (str_last_online.find(day)!=std::string::npos)
        {
            std::vector<std::string> str_vec;
            base::SplitStringUsingSubstr(str_last_online, day, &str_vec);
            if (!base::StringToUint(str_vec.at(0), &last_online))
            {
                DCHECK(false && L"转换失败");
                continue;
            }
        }

        DisplayRoomInfo display;
        display.roomid = roomId;
        display.starid = starId;
        display.star_level = star_level;
        display.last_online = last_online;

        roomids->push_back(display);
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

bool UserTrackerHelper::GetRoomConsumerList(uint32 roomid, uint32* star_level, std::map<uint32, ConsumerInfo>* consumers_map)
{
    std::vector<ConsumerInfo> consumer_infos;
    if (!user_->OpenRoomAndGetConsumerList(roomid, &consumer_infos, star_level))
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




