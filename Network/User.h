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
    uint32 gift_award = 0; // ��Ʊ
    uint32 gift_single = 0; // ����Ʊ
};


class TcpClientController;
class CurlWrapper;
class CookiesHelper;
class Room;
// ���๦��
// ���ݴ������ݳ�ʼ���û�ʹ��ҳ�������: �û���+����/cookie·����ʽ
// ���ﲻҪ���κ��̱߳��湦��
class User
{
public:
    User();
    //User(const std::string& username, const std::string& password);
    ~User();

    bool Initialize(const scoped_refptr<base::TaskRunner>& runner);
    void Finalize();

    // ���ò���
    void SetUsername(const std::string& username);
    std::string GetUsername() const;
    
    void SetPassword(const std::string& password);
    std::string GetPassword() const;

    void SetIpProxy(const IpProxy& ipproxy);
    IpProxy GetIpProxy() const;

    void SetCookies(const std::string& cookies);
    std::string GetCookies() const;

    void SetRoomServerIp(const std::string& serverip);

    void SetTcpManager(TcpClientController* tcpManager);

    //���÷���������Ϣ�ص�����,����Ľ�������Ϊ����Ҫ�������ģ�鴦��
    void SetNormalNotify(NormalNotify normalNotify);
    void SetNotify201(Notify201 notify201);
    void SetNotify501(Notify501 notify501);
    void SetNotify601(Notify601 notify601);
    void SetNotify620(Notify620 notify620);

    // ������Ϊ
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

    // ���������Ϊ�˲������������Ӷ���ȡ������30���������ݣ���Ϊ��׷��ָ���û�
    bool OpenRoomAndGetConsumerList(uint32 roomid,
        std::vector<ConsumerInfo>* consumer_infos);

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

    // ���Ҷ���
    bool RetrieveHappyFreeCoin(uint32 roomid, const std::string& gift_token, 
        uint32* coin, std::string* errormsg);

    bool GetGiftList(uint32 roomid, std::string* content);
    bool GetViewerList(uint32 roomid, 
        std::vector<EnterRoomUserInfo>* enterRoomUserInfo);

	bool KickoutUser(KICK_TYPE kicktype, uint32 roomid, 
        const EnterRoomUserInfo& enterRoomUserInfo);
    bool BanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool UnbanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);

    // �����ع���
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

    TcpClientController* tcpManager_;
    std::string username_;
    std::string password_;
    std::string serverip_;
    IpProxy ipproxy_;

    // ͼ�������ʹ�õĽӿ�key
    std::string apikey_;

    // ��¼����ܻ�õ��û���Ϣ
    std::string nickname_ = "";
    uint32 richlevel_ = 0;
    uint32 kugouid_ = 0;
    uint32 fanxingid_ = 0;
    uint32 clanid_ = 0;
    uint32 coin_ = 0;
    std::string usertoken_ = "";
    std::string userkey_ = "";
    uint32 servertime_ = 0xFFFFFFFF; // ���ֵ������Ȩ�޿��Ƶ�ʱ���ж�ʹ��

    std::unique_ptr<CurlWrapper> curlWrapper_ = nullptr;
    std::unique_ptr<CookiesHelper> cookiesHelper_ = nullptr;
    std::map<uint32, std::shared_ptr<Room>> rooms_;

    NormalNotify normalNotify_;
    Notify201 notify201_;
    Notify501 notify501_;
    Notify601 notify601_;
    Notify620 notify_620_;

    // ��Ȼ���󣬼�¼��ǰ��ѵĴ�Ʊ�͵���Ʊ����
    uint32 awards_ticket_count_;
    uint32 single_ticket_count_;

    scoped_refptr<base::TaskRunner> runner_;

    std::string kg_mid_;
};

