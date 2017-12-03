#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/threading/thread.h"
#include "Network/MessageNotifyManager.h"
#include "Network/Room.h"
#include "SingerInfo.h"
#include "SingerInfoDatabase.h"


class User;
class CurlWrapper;
class EasyHttpImpl;
class HttpResponse;
class AuthorityHelper;
struct UserTrackerAuthority;

struct FollowUserInfo
{
    uint32 user_id;
    std::string nickname;
    std::string last_login;
};

struct DisplayRoomInfo
{
    uint32 roomid;
    uint32 starid;
    uint32 star_level;
    uint32 status;
    uint32 last_online;
};

enum class RangSearchErrorCode
{
    RS_OK,
    RS_ROOM_OPEN_FAILED,
    RS_GET_SINGINFO_FAILED,
    RS_GET_LAST_ONLINE_FAILED,
};

class UserTrackerHelper
{
public:
    UserTrackerHelper();
    ~UserTrackerHelper();
    typedef base::Callback<void(uint32, uint32)> ProgressCallback;

    typedef base::Callback<void(uint32, const SingerInfo&, 
        uint32/*httpresponsecode*/, RangSearchErrorCode)> ResultCallback;

    bool Initialize();// 启动线程
    void Finalize();// 结束线程

    void Test();

    void SetNotifyMessageCallback(const base::Callback<void(const std::wstring&)> callback);

    void SetRangeSearchCallback(ProgressCallback progress_callback, 
        ResultCallback result_callback);

    void SetSearchConfig(uint32 min_star_level, bool check_star, bool check_diamon,
        bool check_1_3_crown, bool check_4_crown_up);

    std::wstring GetAuthorityMessage() const;

    void CancelCurrentOperation();

    bool LoginGetVerifyCode(std::vector<uint8>* picture);

    bool LoginUser(const std::string& user_name, const std::string& password,
        const std::string& verifycode, 
        const base::Callback<void(bool,uint32, const std::string&)>& callback);

    bool UpdatePhoneForNoClan(const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // 强制更新全站主播房间用户列表数据
    bool UpdataAllStarRoomUserMap(const base::Callback<void(uint32, uint32)>& progress_callback);

    bool UpdataAllStarRoomForNoClan(const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // 不使用缓存数据，边更新缓存边查找用户，只要找到就停止，不继续往后面找
    bool UpdateForFindUser(const std::vector<uint32> user_ids,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // 从已经更新好的全站缓存中查找用户信息
    bool GetUserLocationByUserId(const std::vector<uint32> user_ids,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    bool ClearCache();

    // 获取所有女神分类中没有公会的主播，不管是否开播
    bool GetAllBeautyStarForNoClan(uint32 days,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    bool RunSearchHotKey(const std::wstring& hotkey, uint32 times,
        const base::Callback<void(uint32, uint32)>& progress_callback);

    void RunSearchRoomIdRange(uint32 roomid_min, uint32 roomid_max);

    bool SaveRooms(const std::vector<std::wstring>& roomids);

    bool LoadRooms(const std::wstring& path_file_name, std::vector<std::wstring>* roomids);

    static bool GetOutputFileName(base::FilePath* outpath);
private:

    void DoLoginUser(const std::string& user_name, 
        const std::string& password, const std::string& verifycode,
        const base::Callback<void(bool, uint32, const std::string&)>& progress_callback);

    void DoUpdatePhoneForNoClan(
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // 强制更新全站主播房间用户列表数据
    void DoUpdataAllStarRoomUserMap(const base::Callback<void(uint32, uint32)>& progress_callback);

    void DoUpdataAllStarRoomForNoClan(const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // 不使用缓存数据，边更新缓存边查找用户，只要找到就停止，不继续往后面找
    void DoUpdateForFindUser(const std::vector<uint32> user_ids,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // 从已经更新好的全站缓存中查找用户信息
    void DoGetUserLocationByUserId(const std::vector<uint32> user_ids,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    //void DoSearchHotKey(const std::wstring& hotkey, uint32 times,
    //    const base::Callback<void(uint32, uint32)>& progress_callback);

    //void SearchHotKeyCallback(uint32 all_times,
    //    const base::Callback<void(uint32, uint32)>& progress_callback,
    //    const HttpResponse& response);


    // 获取指定范围房间号的主播详细信息
    bool DoOpenRoomForGetSingerid(uint32 roomid);
    bool DoGetSingerLastOnline(SingerInfo singerid);
    void DoGetSingerTags(SingerInfo singer_info);
    void DoGetRoomMessage(SingerInfo singer_info);
    void DoGetSuperFans(SingerInfo singer_info);
    void DoGetThirtydays(SingerInfo singer_info);

    void RangeSearchResultToDB(uint32 roomid, const SingerInfo& singer_info,
        uint32 status, RangSearchErrorCode error);

    void DoClearCache();

    bool GetDanceRoomInfos(std::vector<uint32>* roomids);
    bool GetPhoneRoomInfos(std::vector<uint32>* roomids);

    bool GetAllStarRoomInfos(std::vector<uint32>* roomids); // 从全站获取所有正在直播的主播直播粗略信息，为下一步获取房间用户列表做准备
    bool GetTargetStarRoomInfos(const std::string& url, std::vector<DisplayRoomInfo>* roomids);

    // 强制更新房间用户信息，roomid_user_map返回所有房间在线用户信息
    bool GetAllRoomViewers(const std::vector<uint32>& roomids,
        std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
        const base::Callback<void(uint32, uint32)>& progress_callback);

    // 边找用户边更新房间用户信息，只要找完用户，就不更新了，roomid_user_map只返回查找过的房间在线用户信息
    bool FindUsersWhenUpdateRoomViewerList(const std::vector<uint32>& roomids,
        std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
        const std::vector<uint32>& user_ids,
        std::map<uint32, uint32>* user_room_map,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // 从指定房间获取用户列表，追踪用户
    bool GetRoomViewerList(uint32 roomid, std::map<uint32, EnterRoomUserInfo>* user_map);

    // 从指定房间获取30天消费用户列表，用于追踪消费用户。通过手机协议，不需要cookie
    bool GetRoomConsumerList(uint32 roomid, uint32* star_level, std::map<uint32, ConsumerInfo>* consumers_map);
    
    //GET /UServices/UserService/UserExtService/getFollowList?args=[1,10,0,%22%22,0,3]&_=1478426799924
    //Referer: http://fanxing.kugou.com/index.php?action=userFollowList
    bool GetMyConcernList(std::vector<FollowUserInfo>* follow_user_infos); // 获取登录用户的关注列表

    // 获取用户个人页面信息, 用来判断其是否为主播, 这是可选的辅助功能
    bool GetUserInfoByUserId(); 

    // 获取用户关注主播的信息, 这是可选的辅助功能
    bool GetUserConcernList(); 

    //========= 以下函数为提高拉取房间数据效率使用 =========================
    bool AsyncOpenRoom(uint32 roomid, 
        const base::Callback<void(uint32, uint32)>& progress_callback);

    void OpenRoomCallback(uint32 roomid,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const HttpResponse& response);

    void AsyncGetRoomViewerList(uint32 roomid, uint32 singerid,
        const base::Callback<void(uint32, uint32)>& progress_callback);

    void GetRoomViewerListCallback(uint32 roomid,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const HttpResponse& response);

    scoped_ptr<base::Thread> worker_thread_;
    std::unique_ptr<User> user_;
    std::unique_ptr<CurlWrapper> curl_wrapper_;
    std::unique_ptr<EasyHttpImpl> easy_http_impl_;
    std::unique_ptr<UserTrackerAuthority> tracker_authority_;
    std::wstring authority_msg_;

    std::map<uint32, std::map<uint32, EnterRoomUserInfo>> roomid_userid_map_;

    base::Callback<void(const std::wstring&)> message_callback_;

    bool cancel_flag_ = false;

    uint32 min_star_level_ = 0;
    bool check_star_ = false;
    bool check_diamon_ = false;
    bool check_1_3_crown_ = false;
    bool check_4_crown_up_ = false;

    uint32 all_room_count_ = 0;
    uint32 current_room_count_ = 0;
    bool is_expired = false; // 内部控制是否过期，如果过期，把配置文件写坏，以后也不让访问

    uint32 search_hot_key_count_ = 0;

    std::map<uint32, uint32> room_error_map_; // 第一次访问失败需要重试的房间号
    ProgressCallback progress_callback_;
    ResultCallback result_callback_;
    std::unique_ptr<SingerInfoDatabase> database_;

    std::map<uint32, uint32> status_map_;

    std::wstring recode_date_;
};

