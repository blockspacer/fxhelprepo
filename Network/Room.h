#pragma once
#include <string>
#include <memory>
#include "IpProxy.h"
#include "third_party/chromium/base/basictypes.h"
#include "CurlWrapper.h"
#include "MessageNotifyManager.h"


struct ConsumerInfo 
{
    uint32 fanxing_id;
    uint32 room_id;
    uint32 coin;
    std::string nickname;
    uint32 rich_level;
};

class CurlWrapper;
class MessageNotifyManager;
class CookiesHelper;
class Room
{
public:
    explicit Room(uint32 roomid);
    ~Room();
    
    bool Initialize(const scoped_refptr<base::TaskRunner>& runner);
    void Finalize();

    void SetIpProxy(const IpProxy& ipproxy);
    void SetRoomServerIp(const std::string& serverip);
    void SetTcpManager(TcpClientController* tcpManager);

    uint32 GetSingerId() const{
        return singerid_;
    };
    std::string GetSingerName() const{
        return nickname_;
    }


    // ��Ҫ��÷�����Ϣ������һ�������ĺ���, ����singer_clanid��Ϊ�ж���Ȩʹ��
    bool EnterForOperation(const std::string& cookies, 
        const std::string& usertoken, uint32 userid, uint32* singer_clanid,
        const base::Callback<void()>& conn_break_callback);

    // ����Ҫ��ȡ������Ϣ������Ϊ�����ӣ�����ҵ������Ľ�������
    bool EnterForAlive(const std::string& cookies, const std::string& usertoken, uint32 userid,
        const base::Callback<void()>& conn_break_callback);
    void SetNormalNotify(NormalNotify normalNotify);
    void SetNotify201(Notify201 notify201);
    void SetNotify501(Notify501 notify501);
    void SetNotify601(Notify601 notify601);

    void SetNotify620(Notify620 notify_620);
    // �жϽ������ݵ�����
    bool Exit();

    bool GetGiftList(const std::string& cookies, std::string* content);

    // GET /UServices/GiftService/GiftService/sendGift?d=1476689413506&args=["141023689","869",1,"1070190",false]&_=1476689413506 HTTP/1.1
    bool SendGift(const std::string& cookies, uint32 gift_id, uint32 gift_count,
                  std::string* errormsg);

    bool SendStar(const std::string& cookies, uint32 roomid, uint32 count,
        std::string* errormsg);

    bool RealSingLike(const std::string& cookies, 
        uint32 user_kugou_id, const std::string& user_token,
        const std::wstring& song_name, std::string* errormsg);

    bool RobVotes(uint32* award_count, uint32* single_count,
                  std::string* errormsg);
    // ���������Ϊ�˲������������Ӷ���ȡ����������б�ʹ�ã���Ϊ��׷��ָ���û�
    bool OpenRoomAndGetViewerList(const std::string& cookies,
        std::vector<EnterRoomUserInfo>* enterRoomUserInfoList);

    bool GetViewerList(const std::string& cookies,
        std::vector<EnterRoomUserInfo>* enterRoomUserInfoList);

    // ���������Ϊ�˲������������Ӷ���ȡ������30���������ݣ���Ϊ��׷��ָ���û�
    bool OpenRoomAndGetConsumerList(const std::string& cookies,
        std::vector<ConsumerInfo>* consumer_infos);

    bool GetConsumerList(const std::string& cookies,
        std::vector<ConsumerInfo>* consumer_infos);

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
    bool GetStarKugouId(const std::string& cookies);
    bool EnterRoom(const std::string& cookies, uint32 userid, const std::string& usertoken);
    bool GetStarGuard();
    void TranferNotify601(const RoomGiftInfo601& roomgiftinfo);

    bool ConnectToNotifyServer_(uint32 roomid, uint32 userid,
        const std::string& usertoken, const base::Callback<void()>& conn_break_callback);

    IpProxy ipproxy_;
    uint32 roomid_ = 0;
    uint32 singerid_ = 0;
    uint32 star_kugou_id_ = 0;
    std::string nickname_;
    uint32 clanid_ = 0;
    std::vector<uint32> guarduserids_;
    Notify601 notify601transfer_;
    std::unique_ptr<CurlWrapper> curlWrapper_;
    std::shared_ptr<MessageNotifyManager> messageNotifyManager_;
    std::unique_ptr<CookiesHelper> cookiesHelper_;
};

