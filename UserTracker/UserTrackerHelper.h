#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
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
    uint32 kugouid;
    uint32 star_level;
    uint32 status;
    uint32 last_online;
    std::string nickname;
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

    bool Initialize();// �����߳�
    void Finalize();// �����߳�

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

    // ǿ�Ƹ���ȫվ���������û��б�����
    bool UpdataAllStarRoomUserMap(const base::Callback<void(uint32, uint32)>& progress_callback);

    bool UpdataAllStarRoomForNoClan(const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // ��ʹ�û������ݣ��߸��»���߲����û���ֻҪ�ҵ���ֹͣ����������������
    bool UpdateForFindUser(const std::vector<uint32> user_ids,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // ���Ѿ����ºõ�ȫվ�����в����û���Ϣ
    bool GetUserLocationByUserId(const std::vector<uint32> user_ids,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    bool ClearCache();

    // ��ȡ����Ů�������û�й���������������Ƿ񿪲�
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

    // ǿ�Ƹ���ȫվ���������û��б�����
    void DoUpdataAllStarRoomUserMap(const base::Callback<void(uint32, uint32)>& progress_callback);

    void DoUpdataAllStarRoomForNoClan(const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // ��ʹ�û������ݣ��߸��»���߲����û���ֻҪ�ҵ���ֹͣ����������������
    void DoUpdateForFindUser(const std::vector<uint32> user_ids,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // ���Ѿ����ºõ�ȫվ�����в����û���Ϣ
    void DoGetUserLocationByUserId(const std::vector<uint32> user_ids,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    //void DoSearchHotKey(const std::wstring& hotkey, uint32 times,
    //    const base::Callback<void(uint32, uint32)>& progress_callback);

    //void SearchHotKeyCallback(uint32 all_times,
    //    const base::Callback<void(uint32, uint32)>& progress_callback,
    //    const HttpResponse& response);


    // ��ȡָ����Χ����ŵ�������ϸ��Ϣ
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

    bool GetAllStarRoomInfos(std::vector<uint32>* roomids); // ��ȫվ��ȡ��������ֱ��������ֱ��������Ϣ��Ϊ��һ����ȡ�����û��б���׼��
    bool GetTargetStarRoomInfos(const std::string& url, std::vector<DisplayRoomInfo>* roomids);
    bool GetPhoneTargetStarRoomInfos(uint32 page_num, 
        std::vector<DisplayRoomInfo>* roomids, bool* has_next); // �ֻ����󷵻��������ݶ�һ�㣬�ṹ��һ��

    // ǿ�Ƹ��·����û���Ϣ��roomid_user_map�������з��������û���Ϣ
    bool GetAllRoomViewers(const std::vector<uint32>& roomids,
        std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
        const base::Callback<void(uint32, uint32)>& progress_callback);

    // �����û��߸��·����û���Ϣ��ֻҪ�����û����Ͳ������ˣ�roomid_user_mapֻ���ز��ҹ��ķ��������û���Ϣ
    bool FindUsersWhenUpdateRoomViewerList(const std::vector<uint32>& roomids,
        std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
        const std::vector<uint32>& user_ids,
        std::map<uint32, uint32>* user_room_map,
        const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

    // ��ָ�������ȡ�û��б�׷���û�
    bool GetRoomViewerList(uint32 roomid, std::map<uint32, EnterRoomUserInfo>* user_map);

    // ��ָ�������ȡ30�������û��б�����׷�������û���ͨ���ֻ�Э�飬����Ҫcookie
    bool GetRoomConsumerList(uint32 roomid, uint32* star_level, std::map<uint32, ConsumerInfo>* consumers_map);
    
    //GET /UServices/UserService/UserExtService/getFollowList?args=[1,10,0,%22%22,0,3]&_=1478426799924
    //Referer: http://fanxing.kugou.com/index.php?action=userFollowList
    bool GetMyConcernList(std::vector<FollowUserInfo>* follow_user_infos); // ��ȡ��¼�û��Ĺ�ע�б�

    // ��ȡ�û�����ҳ����Ϣ, �����ж����Ƿ�Ϊ����, ���ǿ�ѡ�ĸ�������
    bool GetUserInfoByUserId(); 

    // ��ȡ�û���ע��������Ϣ, ���ǿ�ѡ�ĸ�������
    bool GetUserConcernList(); 

    //========= ���º���Ϊ�����ȡ��������Ч��ʹ�� =========================
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
    bool is_expired = false; // �ڲ������Ƿ���ڣ�������ڣ��������ļ�д�����Ժ�Ҳ���÷���

    uint32 search_hot_key_count_ = 0;

    std::map<uint32, uint32> room_error_map_; // ��һ�η���ʧ����Ҫ���Եķ����
    ProgressCallback progress_callback_;
    ResultCallback result_callback_;
    std::unique_ptr<SingerInfoDatabase> database_;

    std::map<uint32, uint32> status_map_;

    std::wstring recode_date_;
};

