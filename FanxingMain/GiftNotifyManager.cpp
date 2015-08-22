#include "GiftNotifyManager.h"
#include <assert.h>
#include "TcpClient.h"
#include "EncodeHelper.h"

#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/json/json.h"



namespace
{   
const char* targetip = "42.62.68.50";
const uint16 port843 = 843;
const uint16 port8080 = 8080;
struct cmd201package
{
    uint32 cmd;
    uint32 roomid;
    uint32 userid;
    std::string nickname;
    uint32 richlevel;
    uint32 ismaster;
    uint32 staruserid;
    std::string key;
    uint64 keytime;
    std::string ext;
};
// 这个数据包应该从http请求那边返回过来设置
bool GetFirstPackage(const cmd201package& package, 
    std::vector<uint8> *packagedata)
{
    uint32 nowtime = static_cast<uint32>(base::Time::Now().ToDoubleT());

    Json::FastWriter writer;
    Json::Value root(Json::objectValue);
    root["cmd"];
    root["roomid"];
    root["userid"];
    root["nickname"];
    root["richlevel"];
    root["ismaster"];
    root["staruserid"];
    root["key"];
    root["keytime"];
    root["ext"];
    return false;
}

};
GiftNotifyManager::GiftNotifyManager()
    :tcpClient_843_(new TcpClient),
    tcpClient_8080_(new TcpClient)
{

}

GiftNotifyManager::~GiftNotifyManager()
{

}

bool GiftNotifyManager::Connect843()
{
    tcpClient_843_->Connect(targetip, port843);
    std::string str = "<policy-file-request/>";
    std::vector<char> data;
    data.assign(str.begin(), str.end());
    data.push_back(0);
    return tcpClient_843_->Send(data);
}

bool GiftNotifyManager::Connect8080(uint32 roomid, uint32 userid, 
    const std::string& nickname, uint32 richlevel, uint32 ismaster, 
    uint32 staruserid, const std::string& key, uint64 keytime, 
    const std::string& ext)
{
    std::string decodestr = UrlDecode(ext);// 测试使用
       
    if (!tcpClient_8080_->Connect(targetip, port8080))
    {
        assert(false && L"socket连接失败");
        return false;
    }
    
    std::vector<uint8> data_for_send;
    cmd201package package = { 
        201, roomid, userid, nickname, richlevel, ismaster, staruserid, key, 
        keytime, ext };
    GetFirstPackage(package, &data_for_send);
    std::vector<char> data_8080;
    data_8080.assign(data_for_send.begin(), data_for_send.end());
    data_8080.push_back(0);//这是必须加这个字节的
    tcpClient_8080_->Send(data_8080);
    return true;
}
