#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "Room.h"
#include "Network/IpProxy.h"

#undef max
#undef min
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

class TcpManager;
class CurlWrapper;
class CookiesHelper;
class Room;
// 本类功能
// 根据传入数据初始化用户使用页面的数据: 用户名+密码/cookie路径方式
// 这里不要做任何线程保存功能
class User
{
public:
    User();
    //User(const std::string& username, const std::string& password);
    ~User();

    // 设置参数
    void SetUsername(const std::string& username);
    std::string GetUsername() const;
    
    void SetPassword(const std::string& password);
    std::string GetPassword() const;

    void SetIpProxy(const IpProxy& ipproxy);
    IpProxy GetIpProxy() const;

    void SetCookies(const std::string& cookies);
    std::string GetCookies() const;

    void SetRoomServerIp(const std::string& serverip);

    void SetTcpManager(TcpManager* tcpManager);

    //设置房间命令消息回调函数,命令的解析和行为处理要在另外的模块处理
    void SetNormalNotify(NormalNotify normalNotify);
    void SetNotify201(Notify201 notify201);
    void SetNotify501(Notify501 notify501);
    void SetNotify601(Notify601 notify601);

    // 操作行为
    bool Login();
    bool Login(const std::string& username,
               const std::string& password, 
               const std::string& verifycode,
               std::string* errormsg);

    bool LoginWithCookies(const std::string& cookies, 
                          std::string* errormsg);
    bool Logout();
    bool LoginGetVerifyCode(std::vector<uint8>* picture);

    uint32 GetServerTime() const;
    uint32 GetFanxingId() const;
    uint32 GetClanId() const;

    bool EnterRoomFopOperation(uint32 roomid);
    bool EnterRoomFopAlive(uint32 roomid);
    bool ExitRoom(uint32 roomid);
    bool ExitRooms();

    void SetRobotApiKey(const std::string& apikey);
    bool SendChatMessage(uint32 roomid, const std::string& message);
    bool SendChatMessageRobot(const RoomChatMessage& roomChatMessage);
    bool RequestRobot(uint32 senderid, const std::string& request, std::string* response);
    bool SendStar(uint32 count);
    bool RetrieveStart();
    bool SendGift(uint32 giftid);

    bool GetViewerList(uint32 roomid, 
        std::vector<EnterRoomUserInfo>* enterRoomUserInfo);

	bool KickoutUser(KICK_TYPE kicktype, uint32 roomid, 
        const EnterRoomUserInfo& enterRoomUserInfo);
    bool BanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool UnbanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool Worship(uint32 roomid, uint32 userid);
    bool Worship_(const std::string& cookies, uint32 roomid, uint32 userid);
    
private:
    bool CheckVerifyCode(const std::string& verifycode, std::string* errormsg);
    bool LoginHttps(const std::string& username, const std::string& password, 
        const std::string& verifycode, std::string* errormsg);

    bool LoginUServiceGetMyUserDataInfo(std::string* errormsg);

    bool LoginIndexServiceGetUserCenter(std::string* errormsg);

    TcpManager* tcpManager_;
    std::string username_;
    std::string password_;
    std::string serverip_;
    IpProxy ipproxy_;

    // 图灵机器人使用的接口key
    std::string apikey_;

    // 登录后才能获得的用户信息
    std::string nickname_ = "";
    uint32 richlevel_ = 0;
    uint32 kugouid_ = 0;
    uint32 fanxingid_ = 0;
    uint32 clanid_ = 0;
    uint32 coin_ = 0;
    std::string usertoken_ = "";
    std::string userkey_ = "";
    uint32 servertime_ = 0xFFFFFFFF; // 这个值用来做权限控制的时间判断使用

    std::unique_ptr<CurlWrapper> curlWrapper_ = nullptr;
    std::unique_ptr<CookiesHelper> cookiesHelper_ = nullptr;
    std::map<uint32, std::shared_ptr<Room>> rooms_;

    NormalNotify normalNotify_;
    Notify201 notify201_;
    Notify501 notify501_;
    Notify601 notify601_;
};

