#pragma once
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "third_party/chromium/base/basictypes.h"

class TcpClient;
class Thread;

typedef std::function<void(const std::string& key)> Notify601;
typedef std::function<void(const std::wstring& data)> NormalNotify;
class GiftNotifyManager
{
public:
    GiftNotifyManager();
    ~GiftNotifyManager();

    void Set601Notify(Notify601 notify601);
    void SetNormalNotify(NormalNotify normalNotify);
    void Notify(const std::vector<char>& data);
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
    bool Connect8080(uint32 roomid, uint32 userid, const std::string& nickname, 
        uint32 richlevel, uint32 ismaster, uint32 staruserid,
        const std::string& key,/*uint64 keytime, */const std::string& ext);

    bool SendHeartBeat();

private:
    bool alive;
    std::unique_ptr<Thread> thread_;
    std::unique_ptr<TcpClient> tcpClient_8080_;
    std::unique_ptr<TcpClient> tcpClient_843_;
    Notify601 notify601_;
    NormalNotify normalNotify_;
};

