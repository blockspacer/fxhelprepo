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

enum TICKET_TYPE
{
    TICKET_AWARD = 870,
    TICKET_SINGLE = 872
};


struct UserStorageInfo
{
    std::string accountname;
    std::string nickname;
    uint32 rich_level;
    uint32 coin = 0;
    uint32 star_count = 0;
    uint32 gift_award = 0; // 大奖票
    uint32 gift_single = 0; // 单项票
};


class TcpClientController;
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

    bool Initialize(const scoped_refptr<base::TaskRunner>& runner);
    void Finalize();

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

    //void SetTcpManager(TcpClientController* tcpManager);
    void SetWebsocketClientController(WebsocketClientController* controller);

    //设置房间命令消息回调函数,命令的解析和行为处理要在另外的模块处理
    void SetNormalNotify(NormalNotify normalNotify);
    void SetNotify201(Notify201 notify201);
    void SetNotify501(Notify501 notify501);
    void SetNotify601(Notify601 notify601);
    void SetNotify620(Notify620 notify620);

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
    uint32 GetRichlevel() const;

    bool GetRoom(uint32 roomid, std::shared_ptr<Room>* room);
    bool EnterRoomFopOperation(uint32 roomid, uint32* singer_clanid,
        const base::Callback<void()>& conn_break_callback);
    bool EnterRoomFopAlive(uint32 roomid, 
        const base::Callback<void ()>& conn_break_callback);
    bool OpenRoomAndGetViewerList(uint32 roomid,
        std::vector<EnterRoomUserInfo>* enterRoomUserInfoList);

    // 这个函数是为了不建立房间连接而获取房间里30天消费数据，是为了追踪指定用户
    bool OpenRoomAndGetConsumerList(uint32 roomid,
        std::vector<ConsumerInfo>* consumer_infos, uint32* star_level);

    bool EnterRoomFopHttp(uint32 roomid, std::shared_ptr<Room> room);

    bool ExitRoom(uint32 roomid);
    bool ExitRooms();

    void SetRobotApiKey(const std::string& apikey);
    bool SendChatMessage(uint32 roomid, const std::string& message);
    bool SendPrivateMessageToSinger(uint32 roomid, const std::string& message);
    bool SendChatMessageRobot(const RoomChatMessage& roomChatMessage);
    bool RequestRobot(uint32 senderid, const std::string& request, std::string* response);
    bool SendStar(uint32 roomid, uint32 count);
    bool RetrieveStar();
    bool SendGift(uint32 roomid, uint32 gift_id, uint32 gift_count,
                  std::string* errormsg);
    bool RealSingLike(uint32 roomid, const std::wstring& song_name,
        std::string* errormsg);

    // 抢币动作
    bool RetrieveHappyFreeCoin(uint32 roomid, const std::string& gift_token, 
        uint32* coin, std::string* errormsg);

    bool GetGiftList(uint32 roomid, std::string* content);
    bool GetViewerList(uint32 roomid, 
        std::vector<EnterRoomUserInfo>* enterRoomUserInfo);

	bool KickoutUser(KICK_TYPE kicktype, uint32 roomid, 
        const EnterRoomUserInfo& enterRoomUserInfo);
    bool BanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool UnbanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);

    bool BanEnter(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool UnbanEnter(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);

    // 设置房间星低星币礼物显示
    bool SetRoomGiftNotifyLevel(uint32 roomid, uint32 gift_value);

    // 年度相关功能
    bool GetAnnualInfo(std::string* username, uint32 coin_count,
        uint32* award_count, uint32* single_count) const;
    bool RobVotes(uint32 roomid, uint32* award_count, uint32* single_count,
                  std::string* errormsg);
    bool GetStorageGift(UserStorageInfo* user_storage_info, std::string* errormsg);

    bool GetStarCount(uint32 room_id, uint32* star_count);

    bool Worship(uint32 roomid, uint32 userid, std::string* errormsg);
    bool ChangeNickname(const std::string& nickname, std::string* errormsg);

    bool ChangeLogo(const std::string& logo_path, std::string* errormsg);

private:
    bool LoginHttps(const std::string& username, const std::string& password, 
        const std::string& verifycode, std::string* errormsg);

    bool LoginUServiceGetMyUserDataInfo(std::string* errormsg);

    bool LoginIndexServiceGetUserCenter(std::string* errormsg);

    void ConnectionBreakCallback(uint32 room_id);

    bool Worship_(const std::string& cookies, uint32 roomid, uint32 userid,
        std::string* errormsg);

    //TcpClientController* tcpManager_;
    WebsocketClientController* websocket_client_controller_;
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
    Notify620 notify_620_;

    // 年度活动需求，记录当前免费的大奖票和单项票数据
    uint32 awards_ticket_count_;
    uint32 single_ticket_count_;

    scoped_refptr<base::TaskRunner> runner_;

    std::string kg_mid_;
};

