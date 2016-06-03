#include "MessageNotifyManager.h"
#include <assert.h>

#include "TcpClient.h"
#include "EncodeHelper.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/message_loop/message_loop.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/base64.h"
#include "third_party/chromium/base/time/time.h"


namespace
{
static int threadindex = 0;
const char* targetip = "114.54.2.204";
//const char* targetip = "114.54.2.205";
const uint16 port843 = 843;
const uint16 port8080 = 8080;
struct cmd201package
{
    uint32 cmd;
    uint32 roomid;
    uint32 userid;
    std::string usertoken;
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
    root["kugouid"] = package.userid;
    root["token"] = package.usertoken;
    root["appid"] = 1010;
    std::string data = writer.write(root);
    packagedata->assign(data.begin(), data.end());

    return true;
}

struct cmd201package_notlogin
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

bool GetFirstPackage_NotLogin(const cmd201package_notlogin& package,
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
    root["appid"] = 1010;
    std::string data = writer.write(root);
    packagedata->assign(data.begin(), data.end());

    return true;
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
        std::string nickname = content.get("nickname", jvString).asString();
        uint32 richlevel = GetInt32FromJsonValue(content, "richlevel");
        uint32 userid = GetInt32FromJsonValue(content, "userid");
        *outmsg = base::WideToUTF8(L"用户信息通知") + nickname;
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

bool CommandHandle_501(const Json::Value& jvalue, EnterRoomUserInfo* enterRoomUserInfo, std::string* outmsg)
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
		uint32 senderrichlevel = GetInt32FromJsonValue(content, "senderrichlevel");
        uint32 issecrect = GetInt32FromJsonValue(content, "issecrect");//是否私聊
        std::string chatmsg = content.get("chatmsg", jvString).asString();
        std::string sendername = content.get("sendername", jvString).asString();
        std::string receivername = content.get("receivername", jvString).asString();
        *outmsg = base::WideToUTF8(L"聊天消息:") + chatmsg;

		
		enterRoomUserInfo->roomid = GetInt32FromJsonValue(jvalue, "roomid");
		enterRoomUserInfo->unixtime = GetInt32FromJsonValue(jvalue, "time");
		enterRoomUserInfo->nickname = sendername;
		enterRoomUserInfo->richlevel = senderrichlevel;
		enterRoomUserInfo->userid = senderid;
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

        *outmsg = base::WideToUTF8(L"头条信息和跑道信息");
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
MessageNotifyManager::MessageNotifyManager()
    :tcpClient_843_(new TcpClient),
    tcpClient_8080_(new TcpClient),
    notify201_(nullptr),
    notify501_(nullptr),
    notify601_(nullptr),
    baseThread_("NetworkHelperThread" + base::IntToString(threadindex))
{
}

MessageNotifyManager::~MessageNotifyManager()
{
    
}

bool MessageNotifyManager::Initialize()
{
    tcpClient_843_->Initialize();
    tcpClient_8080_->Initialize();
    baseThread_.Start();
    return true;
}
void MessageNotifyManager::Finalize()
{
    if (repeatingTimer_.IsRunning())
        repeatingTimer_.Stop();
    
    baseThread_.Stop();
    tcpClient_843_->Finalize();
    tcpClient_8080_->Finalize();  
}

void MessageNotifyManager::SetServerIp(const std::string& serverip)
{
    serverip_ = serverip;
}

void MessageNotifyManager::SetNotify201(Notify201 notify201)
{
    notify201_ = notify201;
}

void MessageNotifyManager::SetNotify501(Notify201 notify501)
{
    notify501_ = notify501;
}

void MessageNotifyManager::SetNotify601(Notify601 notify601)
{
    notify601_ = notify601;
}

void MessageNotifyManager::SetNormalNotify(NormalNotify normalNotify)
{
    normalNotify_ = normalNotify;
}

//"cmd" : 601,
//"roomid" : "1096718",
//"senderid" : "22202340",
//"receiverid" : 0,
//"time" : "1458141348",
//"content" : {
//    "actionId" : 0,
//        "roomid" : "1096718",
//        "token" : "",
//        "giftname" : "\u6cd5\u62c9\u5229",
//        "senderid" : "22202340",
//        "senderkgid" : "12013116",
//        "sendername" : "\u4e0d\u5e0c\u671b\u88ab\u4eba\u8bb0\u4f4f",
//        "senderrichlevel" : "16",
//        "receiverid" : "197471408",
//        "receiverkgid" : 781510668,
//        "receivername" : "\u6843\u5b50_ing",
//        "receiverrichlevel" : "2",
//        "giftid" : "41",
//        "num" : "1",
//        "type" : 2,
void MessageNotifyManager::Notify(const std::vector<char>& data)
{
    // 需要处理粘包问题
    std::string datastr(data.begin(), data.end());
    std::vector<std::string> packages = HandleMixPackage(datastr);

    for (auto package : packages)
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
            
        uint32 time = GetInt32FromJsonValue(rootdata, "time");
        if (cmd == 601)
        {
            Json::Value jvContent(Json::ValueType::objectValue);
            Json::Value  content = rootdata.get("content", jvContent);
            if (!content.isNull())
            {
                RoomGiftInfo601 roomgiftinfo;
                roomgiftinfo.time = time;
                roomgiftinfo.roomid = roomid;
                roomgiftinfo.senderid = GetInt32FromJsonValue(content, "senderid");
                roomgiftinfo.sendername = content.get("sendername", "").asString();
                roomgiftinfo.receiverid = GetInt32FromJsonValue(content, "receiverid");
                roomgiftinfo.receivername = content.get("receivername", "").asString();
                roomgiftinfo.giftid = GetInt32FromJsonValue(content, "giftid");
                roomgiftinfo.giftname = content.get("giftname", "").asString();
                roomgiftinfo.gitfnumber = GetInt32FromJsonValue(content, "num");
                roomgiftinfo.tips = content.get("tip", "").asString();
                roomgiftinfo.happyobj = GetInt32FromJsonValue(content, "happyobj");
                roomgiftinfo.happytype = GetInt32FromJsonValue(content, "happytype");
                roomgiftinfo.token = content.get("token", "").asString();
                if (notify601_)
                {
                    notify601_(roomgiftinfo);
                }                
            }
        }
        else if (cmd == 201)
        {
            EnterRoomUserInfo enterRoomUserInfo;
            Json::Value jvContent(Json::ValueType::objectValue);
            enterRoomUserInfo.roomid = GetInt32FromJsonValue(rootdata,"roomid");
            enterRoomUserInfo.unixtime = GetInt32FromJsonValue(rootdata, "time");
            Json::Value  content = rootdata.get("content", jvContent);
                
            if (!content.isNull())
            {
                Json::Value jvString("");
                enterRoomUserInfo.nickname = content.get("nickname", jvString).asString();
                enterRoomUserInfo.richlevel = GetInt32FromJsonValue(content, "richlevel");
                enterRoomUserInfo.userid = GetInt32FromJsonValue(content, "userid");
            }
            if (notify201_)
            {
                notify201_(enterRoomUserInfo);
            }
        }

		EnterRoomUserInfo enterRoomUserInfo;
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
			
			if (CommandHandle_501(rootdata, &enterRoomUserInfo, &outmsg))
            {
                if (notify501_)
                    notify501_(enterRoomUserInfo);
            }
            
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

#ifdef _DEBUG
        if (normalNotify_)
        {
            // 处理数据包,转换成输出界面信息
            std::wstring wstr = base::UTF8ToWide(package);
            normalNotify_(wstr);
            if (!outmsg.empty())
            {
                wstr = base::UTF8ToWide(outmsg);
                normalNotify_(wstr);
            }
        }
#endif
    }
}

bool MessageNotifyManager::Connect843()
{
    return baseThread_.message_loop_proxy()->PostTask(
        FROM_HERE, base::Bind(
        &MessageNotifyManager::DoConnect843, this));
    return true;
}

void MessageNotifyManager::DoConnect843()
{
    // 843端口的连接数据没什么用
    //tcpClient_843_->Connect(targetip, port843);
    tcpClient_843_->Connect(serverip_, port843);
    std::string str = "<policy-file-request/>";
    std::vector<char> data;
    data.assign(str.begin(), str.end());
    data.push_back(0);
    tcpClient_843_->Send(data);
    std::vector<char> responsedata;
    tcpClient_843_->Recv(&responsedata);
    tcpClient_843_->Finalize();
}

bool MessageNotifyManager::Connect8080(uint32 roomid, uint32 userid,
    const std::string& usertoken)
{
    baseThread_.message_loop()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoConnect8080, this,
        roomid, userid, usertoken));

    return true;
}

void MessageNotifyManager::DoConnect8080(uint32 roomid, uint32 userid, 
    const std::string& usertoken)
{
    Packet_ = "";
    position_ = 0;
    //if (!tcpClient_8080_->Connect(targetip, port8080))
    if (!tcpClient_8080_->Connect(serverip_, port8080))
    {
        assert(false && L"socket连接失败");
        return ;
    }
    uint32 keytime = static_cast<uint32>(base::Time::Now().ToDoubleT());
    std::vector<uint8> data_for_send;
    cmd201package package = { 
        201, roomid, userid, usertoken};
    GetFirstPackage(package, &data_for_send);
    std::vector<char> data_8080;
    data_8080.assign(data_for_send.begin(), data_for_send.end());
    //data_8080.push_back(0);//这是必须加这个字节的
    tcpClient_8080_->Send(data_8080);

    // 启动发送心跳的timer;
    if (repeatingTimer_.IsRunning())
        repeatingTimer_.Stop();
    
    //MessageNotifyManager::DoSendHeartBeat();
    // 默认是每10秒发送一次心跳
    repeatingTimer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(10), this,
        &MessageNotifyManager::DoSendHeartBeat);

    DoRecv();
    return ;
}

bool MessageNotifyManager::Connect8080_NotLogin(uint32 roomid, uint32 userid,
    const std::string& nickname, uint32 richlevel, uint32 ismaster,
    uint32 staruserid, const std::string& key,/* uint64 keytime, */
    const std::string& ext)
{
    baseThread_.message_loop()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoConnect8080_NotLogin, this,
        roomid, userid, nickname, richlevel,
        ismaster, staruserid, key, ext));

    return true;
}

void MessageNotifyManager::DoConnect8080_NotLogin(uint32 roomid, uint32 userid, 
    const std::string& nickname, uint32 richlevel, uint32 ismaster, 
    uint32 staruserid, const std::string& key, const std::string& ext)
{
    std::string decodestr = UrlDecode(ext);// 测试使用
    Packet_ = "";
    position_ = 0;
    //if (!tcpClient_8080_->Connect(targetip, port8080))
    if (!tcpClient_8080_->Connect(serverip_, port8080))
    {
        assert(false && L"socket连接失败");
        return;
    }
    uint32 keytime = static_cast<uint32>(base::Time::Now().ToDoubleT());
    std::vector<uint8> data_for_send;
    cmd201package_notlogin package = {
        201, roomid, userid, nickname, richlevel, ismaster, staruserid, key,
        keytime, ext };
    GetFirstPackage_NotLogin(package, &data_for_send);
    std::vector<char> data_8080;
    data_8080.assign(data_for_send.begin(), data_for_send.end());
    data_8080.push_back(0);//这是必须加这个字节的
    tcpClient_8080_->Send(data_8080);

    // 启动发送心跳的timer;
    if (repeatingTimer_.IsRunning())
        repeatingTimer_.Stop();

    // 默认是每10秒发送一次心跳
    repeatingTimer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(10), this,
        &MessageNotifyManager::DoSendHeartBeat);

    DoRecv();
    return;
}

bool MessageNotifyManager::SendChatMessage(const std::string& nickname, 
    uint32 richlevel, const std::string& message)
{
    baseThread_.message_loop()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSendChatMessage, this,
        nickname, richlevel, message));
    return true;
}

void MessageNotifyManager::DoSendChatMessage(const std::string& nickname,
    uint32 richlevel, const std::string& message)
{
    Json::FastWriter writer;
    Json::Value root(Json::objectValue);
    root["cmd"] = 501;
    root["nickname"] = nickname;
    root["richlevel"] = base::UintToString(richlevel);
    root["receiverid"] = 0;
    root["receivername"] = "";
    root["chatmsg"] = message;
    root["issecrect"] = 0;

    std::string data = writer.write(root);
    std::vector<char> msg;
    msg.assign(data.begin(), data.end());
    tcpClient_8080_->Send(msg);
}

void MessageNotifyManager::DoSendHeartBeat()
{
    std::string heartbeat = "HEARTBEAT_REQUEST";
    heartbeat.append("\n");
    std::vector<char> heardbeatvec;
    heardbeatvec.assign(heartbeat.begin(), heartbeat.end());
    tcpClient_8080_->Send(heardbeatvec);
    if (normalNotify_)
    {
        normalNotify_(L"房间状态正常");
    }  
}

void MessageNotifyManager::DoRecv()
{
    std::vector<char> buffer;
    if (!tcpClient_8080_->Recv(&buffer))
    {
        if (repeatingTimer_.IsRunning())
            repeatingTimer_.Stop();
        tcpClient_8080_->Finalize();
        normalNotify_(L"Tcp Break Error!");
        return;// 如果返回错误，结束流程
    }

    baseThread_.message_loop()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoRecv, this));

    if (buffer.empty())
        return;

    Notify(buffer);
}

// 在网络积压数据包的时候，可能返回多个完整的数据包
std::vector<std::string> MessageNotifyManager::HandleMixPackage(const std::string& package)
{
    std::vector<std::string> retVec;
    auto it = package.begin();
    auto sentinel = it;
    while (it != package.end())
    {
        switch (*it)
        {
        case '{':
            position_++;
            if (position_ == 1)
            {
                sentinel = it;
            }
            break;
        case '}':
            position_--;
            if (position_ == 0)
            {
                Packet_ += std::string(sentinel, it + 1);
                retVec.push_back(Packet_);
                Packet_ = "";
            }
            break;
        default:
            break;
        }
        it++;
    }

    if (position_ != 0) // 如果到结束还不是一个完整的json串，要等下一个数据包
    {
        Packet_ = std::string(sentinel, it);
    }

    return retVec;
}

