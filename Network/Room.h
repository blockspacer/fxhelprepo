#pragma once
#include <string>
#include <memory>

#include "third_party/chromium/base/basictypes.h"
#include "CurlWrapper.h"
#include "GiftNotifyManager.h"

class CurlWrapper;
class GiftNotifyManager;
class CookiesHelper;
class Room
{
public:
    explicit Room(uint32 roomid);
    ~Room();
    
    bool Enter(const std::string& cookies, const std::string& usertoken, uint32 userid);
    void SetNormalNotify(NormalNotify normalNotify);
    void SetNotify201(Notify201 notify201);
    // 中断接收数据的连接
    bool Exit();

    bool GetViewerList(const std::string& cookies, 
        std::vector<EnterRoomUserInfo>* enterRoomUserInfo);
	bool KickOutUser(KICK_TYPE kicktype, const std::string& cookies,
        const EnterRoomUserInfo& enterRoomUserInfo);

private:
    bool OpenRoom(const std::string& cookies);

    //bool GetCurrentUserInfo(const std::string& cookies,
    //    uint32* userid, std::string* nickname, uint32* richlevel);

    bool EnterRoom(const std::string& cookies, uint32 userid, const std::string& usertoken);

    bool GetSingerInfo();

    bool ConnectToNotifyServer_(uint32 roomid, uint32 userid,
        const std::string& usertoken);

    uint32 roomid_;
    uint32 singerid_;
    std::unique_ptr<CurlWrapper> curlWrapper_;
    std::unique_ptr<GiftNotifyManager> giftNotifyManager_;
    std::unique_ptr<CookiesHelper> cookiesHelper_;
};

