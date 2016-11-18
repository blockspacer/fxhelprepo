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

    // 强制更新全站主播房间用户列表数据
    bool UpdataAllStarRoomUserMap();

    // 不使用缓存数据，边更新缓存边查找用户，只要找到就停止，不继续往后面找
    bool UpdateForFindUser(const std::vector<uint32> user_ids,
        std::map<uint32, uint32>* user_room_map);

    // 从已经更新好的全站缓存中查找用户信息
    bool GetUserLocationByUserId(const std::vector<uint32> user_ids,
        std::map<uint32, uint32>* user_room_map);

private:

    bool GetAllStarRoomInfos(std::vector<uint32>* roomids); // 从全站获取所有正在直播的主播直播粗略信息，为下一步获取房间用户列表做准备
    bool GetTargetStarRoomInfos(const std::string& url, std::vector<uint32>* roomids);

    // 强制更新房间用户信息，roomid_user_map返回所有房间在线用户信息
    bool GetAllRoomViewers(const std::vector<uint32>& roomids,
        std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map);

    // 边找用户边更新房间用户信息，只要找完用户，就不更新了，roomid_user_map只返回查找过的房间在线用户信息
    bool FindUsersWhenRoomViewerList(const std::vector<uint32>& roomids,
        std::map<uint32, std::map<uint32, EnterRoomUserInfo>>* roomid_user_map,
        const std::vector<uint32>& user_ids,
        std::map<uint32, uint32>* user_room_map);

    bool GetRoomViewerList(uint32 roomid, std::map<uint32, EnterRoomUserInfo>* user_map); // 从指定房间获取用户列表，追踪用户
    

    //GET /UServices/UserService/UserExtService/getFollowList?args=[1,10,0,%22%22,0,3]&_=1478426799924
    //Referer: http://fanxing.kugou.com/index.php?action=userFollowList
    bool GetMyConcernList(std::vector<FollowUserInfo>* follow_user_infos); // 获取登录用户的关注列表

    bool GetUserInfoByUserId(); // 获取用户个人页面信息, 用来判断其是否为主播, 这是可选的辅助功能

    bool GetUserConcernList(); // 获取用户关注主播的信息, 这是可选的辅助功能

    std::unique_ptr<User> user_;
    std::unique_ptr<CurlWrapper> curl_wrapper_;

    std::map<uint32, std::map<uint32, EnterRoomUserInfo>> roomid_userid_map_;
};

