#pragma once
#include <string>
#include <vector>
#include <memory>

#include "third_party/chromium/base/basictypes.h"
#include "Network/User.h"

class User;
class CurlWrapper;

struct FollowUserInfo
{
    uint32 user_id;
    std::string nickname;
    std::string last_login;
};
class UserTracker
{
public:
    UserTracker();
    ~UserTracker();

    void Test();

    bool LoginUser(const std::string& user_name, const std::string& password);

    // ǿ�Ƹ���ȫվ���������û��б�����
    bool UpdataAllStarRoomUserMap();

    // ��ʹ�û������ݣ��߸��»���߲����û���ֻҪ�ҵ���ֹͣ����������������
    bool UpdateForFindUser(const std::vector<uint32> user_ids,
        std::map<uint32, uint32>* user_room_map);

    // ���Ѿ����ºõ�ȫվ�����в����û���Ϣ
    bool GetUserLocationByUserId(const std::vector<uint32> user_ids,
        std::map<uint32, uint32>* user_room_map);

    // ǿ�Ƹ���ȫվ���������û���������
    bool UpdataAllStarRoomComsumerMap();

private:

    bool GetAllStarRoomInfos(std::vector<uint32>* roomids); // ��ȫվ��ȡ��������ֱ��������ֱ��������Ϣ��Ϊ��һ����ȡ�����û��б���׼��
    bool GetTargetStarRoomInfos(const std::string& url, std::vector<uint32>* roomids);

    // ǿ�Ƹ��·����û���Ϣ��roomid_user_map�������з��������û���Ϣ
    bool GetAllRoomViewers(const std::vector<uint32>& roomids,
        std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map);

    // ǿ�Ƹ��·���������Ϣ��roomid_consumer_map��������30�������û���Ϣ
    bool GetAllRoomConsumers(const std::vector<uint32>& roomids,
        std::map<uint32, std::map<uint32, ConsumerInfo>>* roomid_consumer_map);

    // �����û��߸��·����û���Ϣ��ֻҪ�����û����Ͳ������ˣ�roomid_user_mapֻ���ز��ҹ��ķ��������û���Ϣ
    bool FindUsersWhenRoomViewerList(const std::vector<uint32>& roomids,
        std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
        const std::vector<uint32>& user_ids,
        std::map<uint32, uint32>* user_room_map);

    bool GetRoomViewerList(uint32 roomid, std::map<uint32, EnterRoomUserInfo>* user_map); // ��ָ�������ȡ�û��б�׷���û�
    
    bool GetConsumerList(uint32 roomid, std::map<uint32, ConsumerInfo>* consumer_map);

    //GET /UServices/UserService/UserExtService/getFollowList?args=[1,10,0,%22%22,0,3]&_=1478426799924
    //Referer: http://fanxing.kugou.com/index.php?action=userFollowList
    bool GetMyConcernList(std::vector<FollowUserInfo>* follow_user_infos); // ��ȡ��¼�û��Ĺ�ע�б�

    bool GetUserInfoByUserId(); // ��ȡ�û�����ҳ����Ϣ, �����ж����Ƿ�Ϊ����, ���ǿ�ѡ�ĸ�������

    bool GetUserConcernList(); // ��ȡ�û���ע��������Ϣ, ���ǿ�ѡ�ĸ�������

    std::unique_ptr<User> user_;
    std::unique_ptr<CurlWrapper> curl_wrapper_;

    std::map<uint32, std::map<uint32, EnterRoomUserInfo>> roomid_userid_map_;

    std::map<uint32, std::map<uint32, ConsumerInfo>> roomid_consumer_map_;
};

