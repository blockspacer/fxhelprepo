#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "Network/MessageNotifyManager.h"
#include "Network/CurlWrapper.h"
#include "AuthorityHelper.h"

class CurlWrapper;
class GiftNotifyManager;
class GiftInfoHelper;
class GiftInfo;
class User;
class TcpManager;

typedef std::function<void(const std::wstring&)> notifyfn;

typedef std::vector<std::wstring> RowData;
typedef std::vector<RowData> GridData;
typedef std::function<void(const RowData&)> notify201;
typedef std::function<void(const RowData&)> notify501;
typedef std::function<void(uint32,const std::wstring&)> notify502;
typedef std::function<void(const RoomGiftInfo601&, const GiftInfo&)> notify601;

enum class HANDLE_TYPE
{
    HANDLE_TYPE_NOTHANDLE = 0,
    HANDLE_TYPE_KICKOUT = 1,
    HANDLE_TYPE_BANCHAT = 2
};

class AntiStrategy 
    : public std::enable_shared_from_this<AntiStrategy>
{
public:
    AntiStrategy();
    ~AntiStrategy();
    HANDLE_TYPE GetUserHandleType(const std::string& nickname) const;
    HANDLE_TYPE GetHandleType() const;
    void SetHandleType(HANDLE_TYPE handletype);

    bool AddNickname(const std::string& vestname);
    bool RemoveNickname(const std::string& vestname);
private:
    std::set<std::string> vestnames_;
    HANDLE_TYPE handletype_ = HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;
};

class GiftStrategy
    :public std::enable_shared_from_this <GiftStrategy>
{
public:
    GiftStrategy();
    ~GiftStrategy();

    void SetThanksFlag(bool enable);
    bool GetGiftThanks(const RoomGiftInfo601& giftinfo, std::wstring* chatmessage);
private:
    bool thanksflag_ = false;
};

struct WelcomeInfo
{
    uint32 fanxing_id;
    std::wstring name;
    std::wstring content;
};

class EnterRoomStrategy
    :public std::enable_shared_from_this<EnterRoomStrategy>
{
public:
    EnterRoomStrategy();
    ~EnterRoomStrategy();



    void SetWelcomeFlag(bool enable);
    void SetWelcomeContent(const std::map<uint32, WelcomeInfo>& special_welcome);
    void GetWelcomeContent(std::map<uint32, WelcomeInfo>* special_welcome);
    bool GetEnterWelcome(const EnterRoomUserInfo& enterinfo, std::wstring* chatmessage);

private:
    bool SaveWelcomeContent(const std::map<uint32, WelcomeInfo>& special_welcome) const;
    bool LoadWelcomeContent(std::map<uint32, WelcomeInfo>* special_welcome);
    bool welcomeflag_ = false;
    base::Lock welcome_lock_;
    std::map<uint32, WelcomeInfo> welcome_info_map_;
};

class NetworkHelper
    : public std::enable_shared_from_this <NetworkHelper>
{
public:
    NetworkHelper();
    ~NetworkHelper();

    bool Initialize();// 启动线程
    void Finalize();// 结束线程

    static void AddRef() {}
    static void Release() {}

    void SetAntiStrategy(std::shared_ptr<AntiStrategy> antiStrategy);
    void SetGiftStrategy(std::shared_ptr<GiftStrategy> giftStrategy);
    void SetGiftThanks(bool enable);
    void SetEnterRoomStrategy(std::shared_ptr<EnterRoomStrategy> enterRoomStrategy);
    void SetRoomWelcome(bool enable);
    void SetRoomRepeatChat(bool enable, const std::wstring& seconds, const std::wstring& chatmsg);

    void SetNotify(notifyfn fn);
    void RemoveNotify();

    void SetNotify201(notify201 fn);
    void RemoveNotify201();

    void SetNotify501(notify501 fn);
    void RemoveNotify501();

    void SetNotify502(notify502 fn);
    void RemoveNotify502();

    void SetNotify601(notify601 fn);
    void RemoveNotify601();

    bool Login(const std::wstring& username, const std::wstring& password,
               const std::wstring& verifycode,std::string* errormsg);
    bool LoginGetVerifyCode(std::vector<uint8>* picture);

    bool GetCurrentUserDisplay(std::wstring* display);
    bool EnterRoom(const std::wstring& roomid);
    bool EnterRoom(uint32 roomid);
    bool GetViewerList(uint32 roomid,
        std::vector<RowData>* enterRoomUserInfoRowdata);

    void SetHandleChatUsers(bool kick);
    // 判断用户是否有操作权限，暂时实现为只有公会成员才能操作。
    bool GetActionPrivilege(std::wstring* message);

    bool KickoutUsers(KICK_TYPE kicktype, uint32 roomid, 
        const EnterRoomUserInfo& enterRoomUserInfo);
    bool BanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool UnbanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool Worship(uint32 roomid, uint32 fanxingid);

    bool SendChatMessage(uint32 roomid, const std::string& message);

    void SetRobotHandle(bool enable);
    void SetRobotApiKey(const std::wstring& apikey);
    bool SendChatMessageRobot(const RoomChatMessage& roomChatMessage);

private:
    void NotifyCallback(const std::wstring& message);
    void NotifyCallback601(uint32 roomid, const RoomGiftInfo601& roomgiftinfo);
    void NotifyCallback201(const EnterRoomUserInfo& enterRoomUserInfo);
    void NotifyCallback501(const EnterRoomUserInfo& enterRoomUserInfo,
        const RoomChatMessage& roomChatMessage);
    void TryHandleUser(const EnterRoomUserInfo& enterRoomUserInfo);
    void TryHandle501Msg(const EnterRoomUserInfo& enterRoomUserInfo);

    void RobotHandleEnterRoom(const EnterRoomUserInfo& enterRoomUserInfo);
    void RobotHandleChatMessage(const EnterRoomUserInfo& enterRoomUserInfo,
        const RoomChatMessage& roomChatMessage);

    void DoSetRoomRepeatChat(bool enable, uint32 seconds, const std::wstring& chatmsg);
    void DoChatRepeat(const std::wstring& chatmsg);

    std::map<uint32, EnterRoomUserInfo> enterRoomUserInfoMap_;
    notifyfn notify_;
    notify201 notify201_;
    notify501 notify501_;
    bool handleall501_ = false;
    bool robotstate_ = false;
    notify502 notify502_;
    notify601 notify601_;
    uint32 roomid_ = 0;
    std::unique_ptr<User> user_;
    std::unique_ptr<Authority> authority_;
    std::shared_ptr<AntiStrategy> antiStrategy_;
    std::shared_ptr<GiftStrategy> giftStrategy_;
    std::shared_ptr<EnterRoomStrategy> enterRoomStrategy_;
    std::unique_ptr<TcpManager> tcpmanager_;

    scoped_ptr<base::Thread> workThread_;
    base::RepeatingTimer<NetworkHelper> chatRepeatingTimer_;
};

