#include "GiftNotifyManager.h"
#include <assert.h>

#include "TcpClient.h"
#include "EncodeHelper.h"
#include "Thread.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/message_loop/message_loop.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/base64.h"


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

std::string Packet = "";
int pos = 0;
// 在网络积压数据包的时候，可能返回多个完整的数据包
std::vector<std::string> HandleMixPackage(const std::string& package)
{
    std::vector<std::string> retVec;
    auto it = package.begin();
    auto sentinel = it;
    while (it!=package.end())
    { 
        switch (*it)
        {
        case '{':
            pos++;
            if (pos==1)
            {
                sentinel = it;
            }
            break;
        case '}':
            pos--;
            if (pos == 0)
            {
                Packet += std::string(sentinel, it+1);
                retVec.push_back(Packet);
                Packet = "";
            }
            break;
        default:
            break;
        }
        it++;    
    }

    if(pos!=0) // 如果到结束还不是一个完整的json串，要等下一个数据包
    {
        Packet = std::string(sentinel,it);
    }

    return retVec;
}

uint32 GetInt32FromJsonValue(const Json::Value& jvalue, const std::string& name)
{
    uint32 ret = 0;
    Json::Value jvdefault(Json::ValueType::objectValue);
    auto getdata = jvalue.get(name, jvdefault);
    if (getdata.isInt())
    {
        ret = getdata.asInt();
    }
    else if (getdata.isString())
    {
        base::StringToUint(getdata.asString(), &ret);
    }

    return ret;
}

bool CommandHandle_100(const Json::Value& jvalue, std::string* outmsg)
{
    // 全站广播消息
    try
    {
        Json::Value jvContent(Json::ValueType::objectValue);
        Json::Value  content = jvalue.get("content", jvContent);
        if (content.isNull())
        {
            return false;
        }

        Json::Value jvdata(Json::ValueType::objectValue);
        Json::Value data = jvdata.get(std::string("data"), jvdata);
        if (data.isNull())
        {
            return false;
        }

        uint32 roomid = GetInt32FromJsonValue(data, "roomId");
        uint32 userid = GetInt32FromJsonValue(data, "userId");
        Json::Value jvString("");
        std::string  innercontent = data.get("content", jvString).asString();
        std::string nickname = data.get("nickName", jvString).asString();
        std::string starnickname = data.get("starNickName", jvString).asString();
        *outmsg = nickname + base::WideToUTF8(L"在") + starnickname + base::WideToUTF8(L"的直播间发广播:") + innercontent;
    }
    catch (...)
    {
        return false;
    }
    return true;
}


bool CommandHandle_201(const Json::Value& jvalue, std::string* outmsg)
{
    try
    {
        Json::Value jvContent(Json::ValueType::objectValue);
        Json::Value  content = jvalue.get("content", jvContent);
        if (content.isNull())
        {
            return false;
        }
        Json::Value jvString("");
        std::string nickname = content.get("nickName", jvString).asString();
        uint32 richlevel = GetInt32FromJsonValue(content, "richlevel");
        uint32 userid = GetInt32FromJsonValue(content, "userid");
        *outmsg = base::WideToUTF8(L"用户信息通知");
    }
    catch (...)
    {
        return false;
    }
    return true;
}

// 房间歌曲信息，忽略
bool CommandHandle_315(const Json::Value& jvalue, std::string* outmsg)
{
    return false;
}

bool CommandHandle_501(const Json::Value& jvalue, std::string* outmsg)
{
    // 房间聊天消息
    try
    {
        Json::Value jvContent(Json::ValueType::objectValue);
        Json::Value  content = jvalue.get("content", jvContent);
        if (content.isNull())
        {
            return false;
        }

        Json::Value jvString("");
        uint32 receiverid = GetInt32FromJsonValue(content, "receiverid");
        uint32 senderid = GetInt32FromJsonValue(content, "senderid");
        uint32 issecrect = GetInt32FromJsonValue(content, "issecrect");//是否私聊
        std::string chatmsg = content.get("chatmsg", jvString).asString();
        std::string sendername = content.get("sendername", jvString).asString();
        std::string receivername = content.get("receivername", jvString).asString();
        *outmsg = base::WideToUTF8(L"聊天消息:") + chatmsg;
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool CommandHandle_602(const Json::Value& jvalue, std::string* outmsg)
{
    // 头条信息和次头条信息
    try
    {
        Json::Value jvContent(Json::ValueType::objectValue);
        Json::Value  content = jvalue.get("content", jvContent);
        if (content.isNull())
        {
            return false;
        }

        Json::Value jvString("");
        uint32 roomid = GetInt32FromJsonValue(content, "roomId");     
        std::string type = content.get("type", jvString).asString();
        std::string sendername = content.get("sendername", jvString).asString();
        std::string receivername = content.get("receivername", jvString).asString();
        std::string giftname = content.get("giftname", jvString).asString();
        uint32 num = GetInt32FromJsonValue(content, "num");
        uint32 coin = GetInt32FromJsonValue(content, "coin");
        uint32 addtime = GetInt32FromJsonValue(content, "addTime");
        uint32 istop = GetInt32FromJsonValue(content, "istop");

        *outmsg = base::WideToUTF8(L"头条信息和次头条信息");
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool CommandHandle_601(const Json::Value& jvalue, std::string* outmsg)
{
    // 房间礼物数据信息
    try
    {
        Json::Value jvContent(Json::ValueType::objectValue);
        Json::Value  content = jvalue.get("content", jvContent);
        if (content.isNull())
        {
            return false;
        }

        Json::Value jvString("");
        uint32 actionId = GetInt32FromJsonValue(content, "actionId");
        std::string token = content.get("token", jvString).asString();
        std::string sendername = content.get("sendername", jvString).asString();
        std::string receivername = content.get("receivername", jvString).asString();

        uint32 giftid = GetInt32FromJsonValue(content, "giftid");
        std::string giftname = content.get("giftname", jvString).asString();
        uint32 num = GetInt32FromJsonValue(content, "num");
        std::string tips = content.get("tip",jvString).asString();

        uint32 happyObj = GetInt32FromJsonValue(content, "happyObj");//是否是幸运礼物

        *outmsg = base::WideToUTF8(L"房间大礼物数据 ") + tips;
    }
    catch (...)
    {
        return false;
    }
    return true;
}
// 房间抢座信息
bool CommandHandle_606(const Json::Value& jvalue, std::string* outmsg)
{
    return false;
}

};
GiftNotifyManager::GiftNotifyManager()
    :tcpClient_843_(new TcpClient),
    tcpClient_8080_(new TcpClient),
    notify601_(nullptr),
    //thread_(new Thread),
    //stdthread_(nullptr),
    //stdfuture_(nullptr),
    //stdpromise_(nullptr),
    //stdcv_(nullptr),
    //alive(false),
    baseThread_("NetworkHelperThread")
{
}

GiftNotifyManager::~GiftNotifyManager()
{
    
}

bool GiftNotifyManager::Initialize()
{
    tcpClient_843_->Initialize();
    tcpClient_8080_->Initialize();
    baseThread_.Start();
    return true;
}
void GiftNotifyManager::Finalize()
{
    tcpClient_843_->Finalize();
    tcpClient_8080_->Finalize();
    baseThread_.Stop();
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
    // 需要处理粘包问题
    std::string datastr(data.begin(), data.end());
    std::vector<std::string> packages = HandleMixPackage(datastr);

    for (auto package : packages)
    {
        try
        {
            Json::Reader reader;
            Json::Value rootdata(Json::objectValue);
            if (!reader.parse(package, rootdata, false))
            {
                return;
            }

            // 暂时没有必要检测status的值
            Json::Value jvCmd(Json::ValueType::intValue);
            uint32 cmd = GetInt32FromJsonValue(rootdata, "cmd");
            uint32 roomid = GetInt32FromJsonValue(rootdata, "roomid");
            uint32 senderid = GetInt32FromJsonValue(rootdata, "senderid");
            uint32 receiverid = GetInt32FromJsonValue(rootdata, "receiverid");
            uint32 time = GetInt32FromJsonValue(rootdata, "time");
            if (cmd == 601)
            {
                Json::Value jvContent(Json::ValueType::objectValue);
                Json::Value  content = rootdata.get("content", jvContent);
                if (!content.isNull())
                {
                    Json::Value jvKey(Json::ValueType::stringValue);
                    std::string key = content.get(std::string("token"), jvKey).asString();
                    if (!key.empty())
                    {
                        notify601_(key);
                    }
                }
            }

            std::string outmsg;
            switch (cmd)
            {
            case 100:
                break;
            case 201:
                CommandHandle_201(rootdata, &outmsg);
                break;
            case 315:
                CommandHandle_315(rootdata, &outmsg);
                break;
            case 501:
                CommandHandle_501(rootdata, &outmsg);
                break;
            case 601:
                CommandHandle_601(rootdata, &outmsg);
                break;
            case 602:
                CommandHandle_602(rootdata, &outmsg);
                break;
            case 606:
                CommandHandle_606(rootdata, &outmsg);
                break;
            default:
                break;
            }

            // 处理数据包,转换成输出界面信息
            std::wstring wstr = base::UTF8ToWide(package);
            normalNotify_(wstr);
            if (!outmsg.empty())
            {
                wstr = base::UTF8ToWide(outmsg);
                normalNotify_(wstr);
            }
        }
        catch (...)
        {
        }
    }
}

bool GiftNotifyManager::Connect843()
{
    return baseThread_.message_loop_proxy()->PostTask(
        FROM_HERE, base::Bind(
        &GiftNotifyManager::DoConnect843, this));
    return true;
}

void GiftNotifyManager::DoConnect843()
{
    // 843端口的连接数据没什么用
    //tcpClient_843_->SetNotify((NotifyFunction)TcpNotify, this);
    tcpClient_843_->Connect(targetip, port843);
    std::string str = "<policy-file-request/>";
    std::vector<char> data;
    data.assign(str.begin(), str.end());
    data.push_back(0);
    tcpClient_843_->Send(data);
}

bool GiftNotifyManager::Connect8080(uint32 roomid, uint32 userid,
    const std::string& nickname, uint32 richlevel, uint32 ismaster,
    uint32 staruserid, const std::string& key,/* uint64 keytime, */
    const std::string& ext)
{
    baseThread_.message_loop()->PostTask(FROM_HERE,
        base::Bind(&GiftNotifyManager::DoConnect8080, this,
                roomid,userid,nickname,richlevel,
                ismaster,staruserid,key,ext));

    return true;
}


void GiftNotifyManager::DoConnect8080(uint32 roomid, uint32 userid, 
    const std::string& nickname, uint32 richlevel, uint32 ismaster, 
    uint32 staruserid, const std::string& key,/* uint64 keytime, */
    const std::string& ext)
{
    std::string decodestr = UrlDecode(ext);// 测试使用
    Packet = "";
    pos = 0;
    tcpClient_8080_->SetNotify((NotifyFunction)TcpNotify, this);
    if (!tcpClient_8080_->Connect(targetip, port8080))
    {
        assert(false && L"socket连接失败");
        return ;
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

    // 启动发送心跳的timer;
    if (repeatingTimer_.IsRunning())
        repeatingTimer_.Stop();
    
    // 默认是每10秒发送一次心跳
    repeatingTimer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(10000), this,
        &GiftNotifyManager::DoSendHeartBeat);

    return ;
}

void GiftNotifyManager::DoSendHeartBeat()
{
    std::string heartbeat = "HEARTBEAT_REQUEST";
    heartbeat.append("\r\n");
    std::vector<char> heardbeadvec;
    heardbeadvec.assign(heartbeat.begin(), heartbeat.end());
    tcpClient_8080_->Send(heardbeadvec);
    normalNotify_(L"Send Heartbeat");
}



