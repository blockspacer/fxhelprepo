#pragma once
#include <string>
#include <vector>
#include "third_party/chromium/base/basictypes.h"

enum class LOGIN_STATE
{  
    LOGOUT = 0,
    LOGIN = 1
};

enum class ROOM_STATE
{
    OUT_ROOM = 0,
    IN_ROOM = 1
};

// 本类功能
// 根据传入数据初始化用户使用页面的数据: 用户名+密码/cookie路径方式
// 这里不要做任何线程保存功能
class User
{
public:
    User();
    ~User();

    // 设置参数
    void SetUserName(const std::string& username);
    std::string GetUserName() const;
    
    void SetPassword(const std::string& password);
    std::string GetPassword() const;

    void SetCookies(const std::vector<std::string> cookies);
    std::vector<std::string> GetCookies() const;

    //设置房间命令消息回调函数,命令的解析和行为处理要在另外的模块处理
    void SetRoomNotify();   

    // 操作行为
    bool Login();
    bool Logout();

    bool EnterRoom(uint32 roomid);
    bool ExitRoom();

    bool Chat(const std::string& message);
    bool SendStar(uint32 count);
    bool RetrieveStart();
    bool SendGift(uint32 giftid);

    bool KickoutUser(uint32 userid);
    bool SilencedUser(uint32 userid);

private:
    std::string username_;
    std::string password_;
    std::vector<std::string> cookies_;

    // 登录后才能获得的用户信息
    uint32 userid_;
    uint32 coin_;
    std::string usertoken_;
    std::string userkey_;

    // 
};

