#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
#include "Network/MessageNotifyManager.h"
#include "EnterRoomStrategy.h"
#include "Network/CurlWrapper.h"
#include "Network/common.h"
#include "AuthorityHelper.h"

class CurlWrapper;
class GiftNotifyManager;
class GiftInfoHelper;
class GiftInfo;
class User;
//class TcpClientController;
class WebsocketClientController;

typedef std::function<void(MessageLevel, const std::wstring&)> notifyfn;

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

    bool AntiStrategy::LoadAntiSetting(std::vector<RowData>* rowdatas);
    bool AntiStrategy::SaveAntiSetting() const;

    HANDLE_TYPE GetUserHandleType(uint32 rich_level, const std::string& nickname) const;
    HANDLE_TYPE GetMessageHandleType(uint32 receiveid, uint32 rich_level, const std::string& message) const;
    HANDLE_TYPE GetHandleType(uint32 rich_level) const;
    void SetHandleType(HANDLE_TYPE handletype);
    void SetHandleRichLevel(uint32 rich_level);

    bool AddSensitive(const std::string& sensitive);
    bool RemoveSensitive(const std::string& sensitive);
    bool AddNickname(const std::string& vestname);
    bool RemoveNickname(const std::string& vestname);
    bool AddReceiveId(const std::string& receiveid);
    bool RemoveReceiveId(const std::string& receiveid);

private:
    std::set<std::string> vestnames_;
    std::set<std::string> sensitives_;
    std::set<uint32> receiveids_;
    HANDLE_TYPE handletype_ = HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;
    uint32 rich_level_ = 3;
};

class GiftStrategy
    :public std::enable_shared_from_this <GiftStrategy>
{
public:
    GiftStrategy();
    ~GiftStrategy();

    bool Initialize(const std::string& content);
    void SetThanksFlag(bool enable);
    void SetGiftValue(uint32 gift_value);

    // �����Լ������Լ����������������ʾ������һ��ʱ��
    void SetGiftDisplayValue(uint32 gift_value, uint32 seconds);
    bool GetBanDisplaySeconds(const RoomGiftInfo601& giftinfo, uint32* seconds, 
        uint32* ban_gift_value);
    bool GetGiftThanks(const RoomGiftInfo601& giftinfo, std::wstring* chatmessage);
private:

    struct GiftInfo
    {
        uint32 giftid = 0;
        std::string giftname = "";
        uint32 price = 0;
        double exchange = false;
        uint32 category = 0;
        std::string categoryname = "";
    };

    std::map<uint32, GiftInfo> giftmap_;
    std::map<uint32, std::string> category_;
    std::vector<uint32> ban_gift_setting_values_;

    uint32 thank_gift_value_ = 0;
    bool thanksflag_ = false;
    uint32 ban_gift_value_ = 0;
    uint32 ban_gift_seconds_ = 0;
};

class NetworkHelper
    : public std::enable_shared_from_this <NetworkHelper>
{
public:
    NetworkHelper();
    ~NetworkHelper();

    bool Initialize();// �����߳�
    void Finalize();// �����߳�

    static void AddRef() {}
    static void Release() {}

    void SetAntiStrategy(std::shared_ptr<AntiStrategy> antiStrategy);
    void SetGiftStrategy(std::shared_ptr<GiftStrategy> giftStrategy);
    void SetGiftThanks(bool enable);
    void SetEnterRoomStrategy(std::shared_ptr<EnterRoomStrategy> enterRoomStrategy);
    void SetRoomWelcome(bool enable);
    void SetRoomRepeatChat(bool enable, const std::wstring& seconds, const std::wstring& chatmsg);

    void SetNotify(notifyfn fn);
    void SetRetriveGiftCoin(base::Callback<void(uint32)>& callback);
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

    bool LoginWithCookies(const std::string& cookies,
        std::string* errormsg);

    bool GetCurrentUserDisplay(std::wstring* display);
    bool EnterRoom(const std::wstring& roomid);
    bool EnterRoom(uint32 roomid);
    bool GetViewerList(uint32 roomid,
        std::vector<RowData>* enterRoomUserInfoRowdata);

    bool GetGiftList(uint32 roomid, std::string* content);

    void SetHandleChatUsers(bool kick);
    // �ж��û��Ƿ��в���Ȩ�ޣ���ʱʵ��Ϊֻ�й����Ա���ܲ�����
    bool GetActionPrivilege(std::wstring* message);

    bool KickoutUsers(KICK_TYPE kicktype, uint32 roomid, 
        const EnterRoomUserInfo& enterRoomUserInfo);
    bool BanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool UnbanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool SetRoomGiftNotifyLevel(uint32 roomid, uint32 gift_value);

    bool SendChatMessage(uint32 roomid, const std::string& message);

    void SetRobotHandle(bool enable);
    void SetRobotApiKey(const std::wstring& apikey);
    bool SendChatMessageRobot(const RoomChatMessage& roomChatMessage);

    void GetCityRankInfos(uint32 roomid);

private:
    void NotifyCallback(MessageLevel level, const std::wstring& message);
    void NotifyCallback601(uint32 roomid, const RoomGiftInfo601& roomgiftinfo);
    void NotifyCallback201(const EnterRoomUserInfo& enterRoomUserInfo);
    void NotifyCallback501(const EnterRoomUserInfo& enterRoomUserInfo,
        const RoomChatMessage& roomChatMessage);
    void ConnectionBreakCallback();

    void SetHandleRichLevel(uint32 rich_level);
    void TryHandleUser(const EnterRoomUserInfo& enterRoomUserInfo);
    void TryHandle501Msg(const EnterRoomUserInfo& enterRoomUserInfo,
        const RoomChatMessage& roomChatMessage);

    void RobotHandleEnterRoom(const EnterRoomUserInfo& enterRoomUserInfo);
    void RobotHandleChatMessage(const EnterRoomUserInfo& enterRoomUserInfo,
        const RoomChatMessage& roomChatMessage);

    void DoSetRoomRepeatChat(bool enable, uint32 seconds, const std::wstring& chatmsg);
    void DoChatRepeat(const std::wstring& chatmsg);
    void DoGetCityRankInfos(uint32 roomid);

    std::map<uint32, EnterRoomUserInfo> enterRoomUserInfoMap_;
    notifyfn notify_;
    notify201 notify201_;
    notify501 notify501_;
    base::Callback<void(uint32)> retrive_gift_coin_callback_;

    bool handleall501_ = false;
    bool robotstate_ = false;
    notify502 notify502_;
    notify601 notify601_;
    uint32 roomid_ = 0;
    uint32 singer_clanid_ = 0;
    std::unique_ptr<User> user_;
    std::unique_ptr<AntiFloodAuthority> authority_;
    std::shared_ptr<AntiStrategy> antiStrategy_;
    std::shared_ptr<GiftStrategy> giftStrategy_;
    std::shared_ptr<EnterRoomStrategy> enterRoomStrategy_;
    //std::unique_ptr<TcpClientController> tcp_client_controller_;
    std::unique_ptr<WebsocketClientController> websocket_client_controller_;

    scoped_ptr<base::Thread> workThread_;
    base::RepeatingTimer<NetworkHelper> chatRepeatingTimer_;
};

