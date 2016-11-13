#pragma once
#include <string>
#include <memory>
#include "IpProxy.h"
#include "third_party/chromium/base/basictypes.h"
#include "CurlWrapper.h"
#include "MessageNotifyManager.h"

class CurlWrapper;
class MessageNotifyManager;
class CookiesHelper;
class Room
{
public:
    explicit Room(uint32 roomid);
    ~Room();
    
    void SetIpProxy(const IpProxy& ipproxy);
    void SetRoomServerIp(const std::string& serverip);
    void SetTcpManager(TcpManager* tcpManager);

    // 需要获得房间信息来做下一步操作的函数, 传出singer_clanid是为判断授权使用
    bool EnterForOperation(const std::string& cookies, 
        const std::string& usertoken, uint32 userid, uint32* singer_clanid);

    // 不需要获取房间信息，仅仅为了连接，不做业务操作的进房请求
    bool EnterForAlive(const std::string& cookies, const std::string& usertoken, uint32 userid);
    void SetNormalNotify(NormalNotify normalNotify);
    void SetNotify201(Notify201 notify201);
    void SetNotify501(Notify501 notify501);
    void SetNotify601(Notify601 notify601);

    void SetNotify620(Notify620 notify_620);
    // 中断接收数据的连接
    bool Exit();

    bool GetGiftList(const std::string& cookies, std::string* content);

    // GET /UServices/GiftService/GiftService/sendGift?d=1476689413506&args=["141023689","869",1,"1070190",false]&_=1476689413506 HTTP/1.1
    bool SendGift(const std::string& cookies, uint32 gift_id, uint32 gift_count);

    bool GetViewerList(const std::string& cookies, 
        std::vector<EnterRoomUserInfo>* enterRoomUserInfo);
	bool KickOutUser(KICK_TYPE kicktype, const std::string& cookies,
        const EnterRoomUserInfo& enterRoomUserInfo);

    bool BanChat(const std::string& cookies, const EnterRoomUserInfo& enterRoomUserInfo);
    bool UnbanChat(const std::string& cookies, const EnterRoomUserInfo& enterRoomUserInfo);

    bool SendChatMessage(const std::string& nickname, uint32 richlevel,
        const std::string& message);

    bool SendChatMessage(const RoomChatMessage& roomChatMessage);
private:
    bool OpenRoom(const std::string& cookies);
    bool GetStarInfo(const std::string& cookies);
    bool EnterRoom(const std::string& cookies, uint32 userid, const std::string& usertoken);
    bool GetStarGuard();
    void TranferNotify601(const RoomGiftInfo601& roomgiftinfo);

    bool ConnectToNotifyServer_(uint32 roomid, uint32 userid,
        const std::string& usertoken);

    IpProxy ipproxy_;
    uint32 roomid_ = 0;
    uint32 singerid_ = 0;
    std::string nickname_;
    uint32 clanid_ = 0;
    std::vector<uint32> guarduserids_;
    Notify601 notify601transfer_;
    std::unique_ptr<CurlWrapper> curlWrapper_;
    std::shared_ptr<MessageNotifyManager> messageNotifyManager_;
    std::unique_ptr<CookiesHelper> cookiesHelper_;
};

