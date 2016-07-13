#pragma once
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include <condition_variable>
#include <functional>
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/timer/timer.h"
#include "third_party/chromium/base/threading/thread.h"
#include "Network/IpProxy.h"
#include "Network/TcpClient.h"

class TcpClient;
class TcpManager;

// 201消息回来解析后出去的数据包
struct EnterRoomUserInfo 
{
    std::string nickname = "";
    uint32 richlevel = 0;
    uint32 roomid = 0;
    uint32 unixtime = 0;
    uint32 userid = 0;
};

struct RoomChatMessage
{
    uint32 roomid = 0;
    uint32 senderid = 0;
    uint32 richlevel = 0;
    std::string sendername = "";
    uint32 receiverid = 0;
    std::string receivername = "";
    bool issecrect = false;
    std::string chatmessage = "";
};

struct RoomGiftInfo601
{
    uint32 time;
    uint32 roomid;
    uint32 senderid;
    std::string sendername;
    uint32 receiverid;
    std::string receivername;
    uint32 giftid;
    std::string giftname;
    uint32 gitfnumber;
    std::string tips;
    uint32 happyobj;
    uint32 happytype;
    std::string token;
};

typedef std::function<void(const RoomGiftInfo601& roomgiftinfo)> Notify601;
typedef std::function<void(const EnterRoomUserInfo& enterRoomUserInfo)> Notify201;
typedef std::function<void(const EnterRoomUserInfo& enterRoomUserInfo,
    const RoomChatMessage& roomChatMessage)> Notify501;

typedef std::function<void(const std::wstring& data)> NormalNotify;
class MessageNotifyManager 
    : public std::enable_shared_from_this <MessageNotifyManager>
{
public:
    MessageNotifyManager();
    ~MessageNotifyManager();

    static void AddRef() {}
    static void Release() {}

    bool Initialize();
    void Finalize();
    void SetTcpManager(TcpManager* tcpManager);
    void SetServerIp(const std::string& serverip);
    void SetIpProxy(const IpProxy& ipproxy);
    void SetNotify201(Notify201 notify201);
    void SetNotify501(Notify501 notify201);
    void SetNotify601(Notify601 notify601);
    void SetNormalNotify(NormalNotify normalNotify);
    
    bool Connect843();// 固定的请求，不需要带其他参数
    bool Connect8080(uint32 roomid, uint32 userid, const std::string& usertoken);

    bool Connect8080_NotLogin(uint32 roomid, uint32 userid, const std::string& nickname,
        uint32 richlevel, uint32 ismaster, uint32 staruserid,
        const std::string& key, const std::string& ext);

    bool SendChatMessage(const std::string& nickname, uint32 richlevel,
        const std::string& message);

    // 使用新的高效的多路tcp请求分发模式
    bool NewConnect843(uint32 roomid, uint32 userid,
        const std::string& usertoken);
    bool NewConnect8080(uint32 roomid, uint32 userid, const std::string& usertoken);
    bool NewSendChatMessage(const std::string& nickname, uint32 richlevel,
                         const std::string& message);

    bool NewSendChatMessageRobot(const RoomChatMessage& roomChatMessage);

private:

    void DoConnect843();
    void DoConnect8080(uint32 roomid, uint32 userid,
        const std::string& usertoken);

    void DoConnect8080_NotLogin(uint32 roomid, uint32 userid, 
        const std::string& nickname,
        uint32 richlevel, uint32 ismaster, uint32 staruserid,
        const std::string& key, const std::string& ext);

    void DoSendChatMessage(const std::string& nickname, uint32 richlevel,
        const std::string& message);

    void DoSendHeartBeat();
    void DoRecv();

    void Notify(const std::vector<char>& data);
    std::vector<std::string> HandleMixPackage(const std::string& package);

    // 使用新的高效的多路tcp请求分发模式
    void NewConnect843Callback(bool result, TcpHandle handle);
    void NewConnect8080Callback(uint32 roomid, uint32 userid,
                             const std::string& usertoken,
                             bool result, TcpHandle handle);
    
    void NewData843Callback(uint32 roomid, uint32 userid,
        const std::string& usertoken, bool result, const std::vector<uint8>& data);
    void NewData8080Callback(bool result, const std::vector<uint8>& data);

    void NewSendHeartBeat(TcpHandle handle);
    void NewSendDataCallback(TcpHandle handle, bool result);

    base::Thread baseThread_;
    base::RepeatingTimer<MessageNotifyManager> repeatingTimer_;
    std::string serverip_ = "192.168.0.1";
    IpProxy ipProxy_;
    std::string Packet_ = "";
    int position_ = 0;

    std::unique_ptr<TcpClient> tcpClient_8080_;
    std::unique_ptr<TcpClient> tcpClient_843_;

    TcpHandle tcphandle_8080_ = -1;
    TcpHandle tcphandle_843_ = -1;

    Notify201 notify201_;
    Notify501 notify501_;
    Notify601 notify601_;
    NormalNotify normalNotify_;

    TcpManager* tcpManager_;
    base::RepeatingTimer<MessageNotifyManager> newRepeatingTimer_;
};

