#pragma once
#include <string>
#include <memory>

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
    
    void SetServerIp(const std::string& serverip);
    bool Enter(const std::string& cookies, const std::string& usertoken, uint32 userid);
    void SetNormalNotify(NormalNotify normalNotify);
    void SetNotify201(Notify201 notify201);
    void SetNotify501(Notify501 notify501);
    // 中断接收数据的连接
    bool Exit();

    bool GetViewerList(const std::string& cookies, 
        std::vector<EnterRoomUserInfo>* enterRoomUserInfo);
	bool KickOutUser(KICK_TYPE kicktype, const std::string& cookies,
        const EnterRoomUserInfo& enterRoomUserInfo);

    bool BanChat(const std::string& cookies, const EnterRoomUserInfo& enterRoomUserInfo);
    bool UnbanChat(const std::string& cookies, const EnterRoomUserInfo& enterRoomUserInfo);

    bool SendChatMessage(const std::string& nickname, uint32 richlevel,
        const std::string& message);
private:
    bool OpenRoom(const std::string& cookies);

    //bool GetCurrentUserInfo(const std::string& cookies,
    //    uint32* userid, std::string* nickname, uint32* richlevel);

    bool GetStarInfo(const std::string& cookies);
    bool EnterRoom(const std::string& cookies, uint32 userid, const std::string& usertoken);

    bool GetSingerInfo();

    bool ConnectToNotifyServer_(uint32 roomid, uint32 userid,
        const std::string& usertoken);

    uint32 roomid_ = 0;
    uint32 singerid_ = 0;
    std::string nickname_;
    uint32 clanid_ = 0;
    std::unique_ptr<CurlWrapper> curlWrapper_;
    std::unique_ptr<MessageNotifyManager> messageNotifyManager_;
    std::unique_ptr<CookiesHelper> cookiesHelper_;
};

