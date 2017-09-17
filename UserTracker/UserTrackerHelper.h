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


class User;
class CurlWrapper;
class EasyHttpImpl;
class HttpResponse;
class AuthorityHelper;
class UserTrackerAuthority;

struct FollowUserInfo
{
    uint32 user_id;
    std::string nickname;
    std::string last_login;
};

class UserTrackerHelper
{
public:
    UserTrackerHelper();
    ~UserTrackerHelper();

    bool Initialize();// �����߳�
    void Finalize();// �����߳�

    void Test();

    void SetNotifyMessageCallback(const base::Callback<void(const std::wstring&)> callback);

    void SetSearchConfig(bool check_star, bool check_diamon,
        bool check_1_3_crown, bool check_4_crown_up);

    std::wstring GetAuthorityMessage() const;

    void CancelCurrentOperation();

    bool LoginGetVerifyCode(std::vector<uint8>* picture);

    bool LoginUser(const std::string& user_name, const std::string& password,
        const std::string& verifycode, 
        const base::Callback<void(bool,uint32, const std::string&)>& callback);

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

    bool GetAllBeautyStarForNoClan(const base::Callback<void(uint32, uint32)>& progress_callback,
        const base::Callback<void(uint32, uint32)>& result_callback);

private:

    void DoLoginUser(const std::string& user_name, 
        const std::string& password, const std::string& verifycode,
        const base::Callback<void(bool, uint32, const std::string&)>& progress_callback);

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

    void DoClearCache();

    bool GetAllStarRoomInfos(std::vector<uint32>* roomids); // ��ȫվ��ȡ��������ֱ��������ֱ��������Ϣ��Ϊ��һ����ȡ�����û��б���׼��
    bool GetTargetStarRoomInfos(const std::string& url, std::vector<uint32>* roomids);

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
    bool GetRoomConsumerList(uint32 roomid, std::map<uint32, ConsumerInfo>* consumers_map);
    
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

    bool check_star_ = false;
    bool check_diamon_ = false;
    bool check_1_3_crown_ = false;
    bool check_4_crown_up_ = false;

    uint32 all_room_count_ = 0;
    uint32 current_room_count_ = 0;
};

