#include "GiftNotifyManager.h"
#include <assert.h>
#include "TcpClient.h"
#include "EncodeHelper.h"
#include "Thread.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"



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
    uint32 keytime;
    std::string ext;
};
// 这个数据包应该从http请求那边返回过来设置
bool GetFirstPackage(const cmd201package& package, 
    std::vector<uint8> *packagedata)
{
    // 10位的时间截
    //uint32 nowtime = static_cast<uint32>(base::Time::Now().ToDoubleT());

    Json::FastWriter writer;
    Json::Value root(Json::objectValue);
    root["cmd"] = package.cmd;
    root["roomid"] = package.roomid;
    root["userid"] = package.userid;
    root["nickname"] = package.nickname;
    root["richlevel"] = package.richlevel;
    root["ismaster"] = package.ismaster;
    root["staruserid"] = package.staruserid;
    root["key"] = package.key;
    root["keytime"] = package.keytime;
    root["ext"] = package.ext;
    std::string data = writer.write(root);
    packagedata->assign(data.begin(), data.end());

    return true;
}
void TcpNotify(void* privatedata, const std::vector<char>& data)
{
    GiftNotifyManager* manager = static_cast<GiftNotifyManager*>(privatedata);
    manager->Notify(data);
    return;
}

DWORD HeartBeatFunc(LPVOID lpParam)
{
    auto p = reinterpret_cast<GiftNotifyManager*>(lpParam);
    p->SendHeartBeat();
    return 0;
}

};
GiftNotifyManager::GiftNotifyManager()
    :tcpClient_843_(new TcpClient),
    tcpClient_8080_(new TcpClient),
    notify601_(nullptr),
    thread_(new Thread),
    alive(false)
{
    thread_->Init((run_fn)(HeartBeatFunc), this);
}

GiftNotifyManager::~GiftNotifyManager()
{

}

void GiftNotifyManager::Set601Notify(Notify601 notify601)
{
    notify601_ = notify601;
}

void GiftNotifyManager::SetNormalNotify(NormalNotify normalNotify)
{
    normalNotify_ = normalNotify;
}

void GiftNotifyManager::Notify(const std::vector<char>& data)
{
    alive = true;
    std::string str(data.begin(), data.end());
    // 解析json数据，拿到命令号
    if (str.find(R"("cmd":601)")>0)
    {
        //解析json数据
        Json::Reader reader;
        Json::Value rootdata(Json::objectValue);
        if (!reader.parse(str, rootdata, false))
        {
            return ;
        }

        // 暂时没有必要检测status的值
        Json::Value jvCmd(Json::intValue);
        int cmd = rootdata.get(std::string("cmd"), jvCmd).asInt();
        Json::Value jvUserid(Json::stringValue);
        std::string strUserid = rootdata.get(std::string("userid"), jvUserid).asString();
        uint32 userid = 0;
        base::StringToUint(strUserid, &userid);
        Json::Value jvKey(Json::stringValue);
        std::string key = rootdata.get(std::string("key"), jvKey).asString();
        notify601_(userid, key);
    }
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(str, rootdata, false))
    {
        return;
    }
    // 暂时没有必要检测status的值
    Json::Value jvCmd(Json::intValue);
    int cmd = rootdata.get(std::string("cmd"), jvCmd).asInt();

    normalNotify_(str);
}

bool GiftNotifyManager::Connect843()
{
    tcpClient_843_->SetNotify((NotifyFunction)TcpNotify, this);
    tcpClient_843_->Connect(targetip, port843);
    std::string str = "<policy-file-request/>";
    std::vector<char> data;
    data.assign(str.begin(), str.end());
    data.push_back(0);
    return tcpClient_843_->Send(data);
}

bool GiftNotifyManager::Connect8080(uint32 roomid, uint32 userid, 
    const std::string& nickname, uint32 richlevel, uint32 ismaster, 
    uint32 staruserid, const std::string& key,/* uint64 keytime, */
    const std::string& ext)
{
    std::string decodestr = UrlDecode(ext);// 测试使用
       
    tcpClient_8080_->SetNotify((NotifyFunction)TcpNotify, this);
    if (!tcpClient_8080_->Connect(targetip, port8080))
    {
        assert(false && L"socket连接失败");
        return false;
    }
    uint32 keytime = static_cast<uint32>(base::Time::Now().ToDoubleT());
    std::vector<uint8> data_for_send;
    cmd201package package = { 
        201, roomid, userid, nickname, richlevel, ismaster, staruserid, key, 
        keytime, ext };
    GetFirstPackage(package, &data_for_send);
    std::vector<char> data_8080;
    data_8080.assign(data_for_send.begin(), data_for_send.end());
    data_8080.push_back(0);//这是必须加这个字节的
    tcpClient_8080_->Send(data_8080);
    thread_->Start();
    return true;
}

bool GiftNotifyManager::SendHeartBeat()
{
    std::string heartbeat = "HEARTBEAT_REQUEST";
    heartbeat.append("\r\n");
    std::vector<char> heardbeadvec;
    heardbeadvec.assign(heartbeat.begin(), heartbeat.end());
    while (1)
    {
        Sleep(10000);
        if (alive)
        {
            tcpClient_8080_->Send(heardbeadvec);
        }
    }

    return true;
}


