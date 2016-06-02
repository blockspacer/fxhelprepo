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


class TcpClient;

// 201消息回来解析后出去的数据包
struct EnterRoomUserInfo 
{
    std::string nickname = "";
    uint32 richlevel = 0;
    uint32 roomid = 0;
    uint32 unixtime = 0;
    uint32 userid = 0;
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
typedef std::function<void(const EnterRoomUserInfo& enterRoomUserInfo)> Notify501;

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
    void SetServerIp(const std::string& serverip);
    void SetNotify201(Notify201 notify201);
    void SetNotify501(Notify501 notify201);
    void SetNotify601(Notify601 notify601);
    void SetNormalNotify(NormalNotify normalNotify);

    // 固定的请求，不需要带其他参数
    bool Connect843();

    // 需要一个从http请求中获取回来的key才能取得正确的连接以及接收数据
    //{
    //    "cmd" : 201,
    //        "roomid" : 1017131,
    //        "userid" : "40919199",
    //        "nickname" : "............", // 直接是昵称的二进制数据，可以写中文然后放二进制解析
    //        "richlevel" : 2,
    //        "ismaster" : 0,
    //        "staruserid" : "17895233",
    //        "key" : "78ea361aa7cb33331c3c8f11cd29cdda",
    //        "keytime" : 1439814773,
    //        "ext" : "%7B%22ui%22%3A0%2C%22isSpRoom%22%3A0%2C%22stli%22%3A%7B%22st%22%3A%222%22%2C%22sl%22%3A0%7D%2C%22userGuard%22%3A%5B%5D%2C%22starCard%22%3A0%2C%22external%22%3A0%2C%22exMemo%22%3A%22%22%2C%22worship%22%3A3%2C%22z%22%3A0%2C%22isGoldFans%22%3A0%2C%22token%22%3A%22c920ae50679115ada1ef5f89b350829466b035f515c4ac4693f965e5321b41b3%22%2C%22kugouId%22%3A454395944%7D"
    //}

    //
    //ext数据也是由进入房间时的http请求RoomService.RoomService&mtd=enterRoom&args=返回获得的数据，解码后数据是：
    //{
    //    " ui " : 0,
    //        " isSpRoom " : 0,
    //        " stli " : {
    //        " st " : " 2 ",
    //            " sl " : 0
    //    },
    //    " userGuard " : [],
    //    " starCard " : 0,
    //    " external " : 0,
    //    " exMemo " : " ",
    //    " worship " : 3,
    //    " z " : 0,
    //    " isGoldFans " : 0,
    //    " token " : " c920ae50679115ada1ef5f89b350829466b035f515c4ac4693f965e5321b41b3 ",
    //    " kugouId " : 454395944
    //}
    bool Connect8080(uint32 roomid, uint32 userid, const std::string& usertoken);

    bool Connect8080_NotLogin(uint32 roomid, uint32 userid, const std::string& nickname,
        uint32 richlevel, uint32 ismaster, uint32 staruserid,
        const std::string& key, const std::string& ext);

    bool SendChatMessage(const std::string& nickname, uint32 richlevel,
        const std::string& message);
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

    base::Thread baseThread_;
    base::RepeatingTimer<MessageNotifyManager> repeatingTimer_;
    std::string serverip_ = "192.168.0.1";
    std::string Packet_ = "";
    int position_ = 0;

    std::unique_ptr<TcpClient> tcpClient_8080_;
    std::unique_ptr<TcpClient> tcpClient_843_;
    Notify201 notify201_;
    Notify501 notify501_;
    Notify601 notify601_;
    NormalNotify normalNotify_;
};

