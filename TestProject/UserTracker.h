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

    bool GetUserLocationByUserId(uint32 user_id, uint32 room_id);

private:
    //GET /UServices/UserService/UserExtService/getFollowList?args=[1,10,0,%22%22,0,3]&_=1478426799924
    //Referer: http://fanxing.kugou.com/index.php?action=userFollowList
    bool GetMyConcernList(std::vector<FollowUserInfo>* follow_user_infos); // 获取登录用户的关注列表

    bool GetUserInfoByUserId(); // 获取用户个人页面信息, 用来判断其是否为主播, 这是可选的辅助功能

    bool GetUserConcernList(); // 获取用户关注主播的信息, 这是可选的辅助功能

    bool GetAllStarRoomInfos(std::vector<uint32>* roomids); // 从全站获取所有正在直播的主播直播粗略信息，为下一步获取房间用户列表做准备
    bool GetTargetStarRoomInfos(const std::string& url, std::vector<uint32>* roomids);
    bool GetRoomViwerList(uint32 roomid, std::vector<EnterRoomUserInfo>* enterRoomUserInfoList); // 从指定房间获取用户列表，追踪用户

    std::unique_ptr<User> user_;
    std::unique_ptr<CurlWrapper> curl_wrapper_;
};

