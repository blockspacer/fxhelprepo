#include "MessageNotifyManager.h"
#include <assert.h>
#include <memory>

#include "IpProxy.h"
#include "TcpManager.h"
#include "EncodeHelper.h"
#include "ServerHelper.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"
#include "third_party/chromium/base/message_loop/message_loop.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/base64.h"
#include "third_party/chromium/base/time/time.h"


namespace
{
static int threadindex = 0;
const uint32 char_millisecond_space = 2500;// 发言间隔使用2.5秒

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
            assert(false);
            return false;
        }

        Json::Value jvdata(Json::ValueType::objectValue);
        Json::Value data = content.get(std::string("data"), jvdata);
        if (data.isNull())
        {
            assert(false);
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
        assert(false);
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
            assert(false);
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
        assert(false);
        return false;
    }
    return true;
}

// 房间歌曲信息，忽略
bool CommandHandle_315(const Json::Value& jvalue, std::string* outmsg)
{
    return false;
}

bool CommandHandle_501(const Json::Value& jvalue, 
    EnterRoomUserInfo* enterRoomUserInfo, RoomChatMessage* roomChatMessage, 
    std::string* outmsg)
{
    // 房间聊天消息
    try
    {
        Json::Value jvContent(Json::ValueType::objectValue);
        Json::Value  content = jvalue.get("content", jvContent);
        if (content.isNull())
        {
            assert(false);
            return false;
        }

        Json::Value jvString("");
        uint32 receiverid = GetInt32FromJsonValue(content, "receiverid");
        uint32 senderid = GetInt32FromJsonValue(content, "senderid");
        uint32 senderkugouid = GetInt32FromJsonValue(content, "senderkugouid");
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

        roomChatMessage->roomid = GetInt32FromJsonValue(jvalue, "roomid");
        roomChatMessage->senderid = senderid;
        roomChatMessage->sendername = sendername;
        roomChatMessage->receiverid = receiverid;
        roomChatMessage->receivername = receivername;
        roomChatMessage->chatmessage = chatmsg;
        roomChatMessage->issecrect = !!issecrect;
    }
    catch (...)
    {
        assert(false);
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
            assert(false);
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
        assert(false);
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
            assert(false);
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
        assert(false);
        return false;
    }
    return true;
}
// 房间抢座信息
bool CommandHandle_606(const Json::Value& jvalue, std::string* outmsg)
{
    return false;
}

// 屠龙消息通知
bool CommandHandle_620(const Json::Value& jvalue, 
    std::vector<BetShowData>* bet_show_datas, BetResult* bet_result)
{
    std::string errmsg;
    //LOG(INFO) << L"++++++++++++++++ tulong message +++++++++++++++++++++++++++++";
    Json::Value jvContent(Json::ValueType::objectValue);
    Json::Value  content = jvalue.get("content", jvContent);
    if (content.isNull())
    {
        assert(false);
        return false;
    }

    Json::Value jvString("");
    uint32 actionId = GetInt32FromJsonValue(content, "actionId");
    if (actionId == 1)
    {
        Json::Value jvdata(Json::ValueType::arrayValue);
        Json::Value data = content.get(std::string("data"), jvdata);
        if (data.isNull())
        {
            assert(false);
            return false;
        }
        if (!data.isArray())
        {
            assert(false);
            return false;
        }

        for (auto bet_data : data)
        {
            BetShowData bet_show_data;
            auto members = bet_data.getMemberNames();
            for (auto member : members)
            {
                if (member.compare("betGid")==0)
                {
                    bet_show_data.bet_gid = GetInt32FromJsonValue(bet_data, member);
                }
                else if (member.compare("odds") == 0)
                {
                    bet_show_data.odds = GetInt32FromJsonValue(bet_data, member);
                }
                else if (member.compare("count") == 0)
                {
                    //bet_show_data.count = GetInt32FromJsonValue(bet_data, member);
                }
            }
            bet_show_datas->push_back(bet_show_data);
        }

        errmsg = base::WideToUTF8(L"屠龙数据更新");
    }
    else if (actionId == 3)
    {
        Json::Value jvdata(Json::ValueType::objectValue);
        Json::Value data = content.get(std::string("data"), jvdata);
        if (data.isNull())
        {
            assert(false);
            return false;
        }

        uint32 result = 0;
        uint32 random = 0;
        for (auto member : data.getMemberNames())
        {
            if (member.compare("result")==0)
            {
                result = GetInt32FromJsonValue(data, member);
            }
            else if ((member.compare("randmon") == 0)// 因为繁星开发人员写错，这里只能检测这个字段
                || (member.compare("random") == 0))
            {
                random = GetInt32FromJsonValue(data, member);
            }
        }
        bet_result->result = result;
        bet_result->random = random;

        errmsg = base::WideToUTF8(L"屠龙结果: result( ") + base::UintToString(result)
            + " ), random( " + base::UintToString(random);
    }
    else
    {
        return false;
    }
    return true;
}

};
MessageNotifyManager::MessageNotifyManager()
    :tcpClient_843_(new TcpClient),
    tcpClient_8080_(new TcpClient),
    notify201_(nullptr),
    notify501_(nullptr),
    notify601_(nullptr),
    baseThread_("NetworkHelperThread" + base::IntToString(threadindex)),
    chat_message_space_(base::TimeDelta::FromMilliseconds(char_millisecond_space)),
    last_chat_time_(base::Time::Now())
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

    // 这个时候，tcpManager_的线程已经结束，不要再抛任务了
    //tcpManager_->RemoveClient(tcphandle_843_);
    //tcpManager_->RemoveClient(tcphandle_8080_);
    if (newRepeatingTimer_.IsRunning())
        newRepeatingTimer_.Stop();

    if (baseThread_.IsRunning())
        baseThread_.Stop();
    
    tcpClient_843_->Finalize();
    tcpClient_8080_->Finalize();  
}

void MessageNotifyManager::SetTcpManager(TcpManager* tcpManager)
{
    tcpManager_ = tcpManager;
}

void MessageNotifyManager::SetServerIp(const std::string& serverip)
{
    std::vector<std::string> hostips;
    if (!ServerHelper::GetChatServerIp(&hostips))
        return;

    if (hostips.empty())
        return;
    
    serverip_ = *hostips.begin();
}

void MessageNotifyManager::SetIpProxy(const IpProxy& ipproxy)
{
    assert(ipproxy.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE);
    ipProxy_.SetProxyType(ipproxy.GetProxyType());
    ipProxy_.SetProxyIp(ipproxy.GetProxyIp());
    ipProxy_.SetProxyPort(ipproxy.GetProxyPort());
}

void MessageNotifyManager::SetNotify201(Notify201 notify201)
{
    notify201_ = notify201;
    baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSetNotify201, this, notify201));
}

void MessageNotifyManager::SetNotify501(Notify501 notify501)
{
    baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSetNotify501, this, notify501));
}

void MessageNotifyManager::SetNotify601(Notify601 notify601)
{
    notify601_ = notify601;
    baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSetNotify601, this, notify601));
}

void MessageNotifyManager::SetNormalNotify(NormalNotify normalNotify)
{
    baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSetNormalNotify, this, normalNotify));
}

void MessageNotifyManager::DoSetNotify201(Notify201 notify201)
{
    notify201_ = notify201;
}

void MessageNotifyManager::DoSetNotify501(Notify501 notify501)
{
    notify501_ = notify501;
}

void MessageNotifyManager::DoSetNotify601(Notify601 notify601)
{
    notify601_ = notify601;
}

void MessageNotifyManager::DoSetNormalNotify(NormalNotify normalNotify)
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
            std::string extentinfo = rootdata.get("ext", "").asString();
            std::string decodeext = UrlDecode(extentinfo);

            Json::Value jvDefault;
            Json::Value extdata(Json::objectValue);
            if (reader.parse(decodeext, extdata, false))
            {
				auto extdata_members = extdata.getMemberNames();
				for (const auto& extdata_member : extdata_members)
				{
					if (extdata_member.compare("stli") == 0)
					{
						Json::Value stli = extdata.get(extdata_member, jvDefault);
						auto stli_members = stli.getMemberNames();
						for (const auto& member : stli_members)
						{
							if (member.compare("isAdmin") == 0)
							{
								uint32 isAdmin = stli.get(member, 0).asInt();
								enterRoomUserInfo.isAdmin = !!isAdmin;
							}
						}
					}
					else if (extdata_member.compare("vipData") == 0)
					{
						Json::Value vipData = extdata.get(extdata_member, jvDefault);
						auto vipData_members = vipData.getMemberNames();
						for (const auto& member : vipData_members)
						{
							if (member.compare("v") == 0)
							{
								uint32 vip_v = GetInt32FromJsonValue(vipData, member);
								enterRoomUserInfo.vip_v = (vip_v == 2); // 2是白金vip, 进入房间是隐身的
							}
						}// for vipData_members
					}
				}// for extdata_members
            }//if reader

            auto jvArray = rootdata.get("userGuard", jvDefault);
            if (jvArray.isArray())
            {
                for (auto guard : jvArray)
                {
                    assert(false && L"参考一下这个守护的数据");
                }
            }

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
        else if (cmd==620)
        {
            std::vector<BetShowData> bet_show_datas;
            BetResult bet_result;
            if (CommandHandle_620(rootdata, &bet_show_datas, &bet_result))
            {
                if (!bet_show_datas.empty())
                {
                    if (bet_show_datas_.empty())
                    {
                        bet_show_datas_ = bet_show_datas;
                    }
                    else
                    {
                        auto show_data = bet_show_datas.begin();
                        auto show_data_ = bet_show_datas_.begin();
                        for (; (show_data != bet_show_datas.end()) 
                            && (show_data_ != bet_show_datas_.end());
                            show_data++, show_data_++)
                        {
                            if ((show_data->bet_gid != show_data_->bet_gid)
                                || (show_data->odds != show_data_->odds))
                            {
                                assert(false);
                                LOG(ERROR) << L"两次通知的开奖数据不一致，有问题";
                                break;
                            }
                        }
                    }
                    LOG(INFO) << L"tulong +++++++++++++++++++++++++++++++++++++++++++++";
                }
                else // 开奖结果数据
                {
                    LOG(INFO) << L"tulong result = " + base::UintToString16(bet_result.result) +
                        L" random = " + base::UintToString16(bet_result.random);
                }
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
        {
            RoomChatMessage roomChatMessage;
            if (CommandHandle_501(rootdata, &enterRoomUserInfo, &roomChatMessage, &outmsg))
            {
                if (notify501_)
                    notify501_(enterRoomUserInfo, roomChatMessage);
            }
            break;
        }
        case 601:
            CommandHandle_601(rootdata, &outmsg);
            break;
        case 602:
            CommandHandle_602(rootdata, &outmsg);
            if (normalNotify_)
                normalNotify_(L"房间状态正常");

            break;
        case 606:
            CommandHandle_606(rootdata, &outmsg);
            break;
        case 620:
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

bool MessageNotifyManager::NewConnect843(
    uint32 roomid, uint32 userid,
    const std::string& usertoken)
{
    tcpManager_->AddClient(
        std::bind(&MessageNotifyManager::NewConnect843Callback, 
        std::weak_ptr<MessageNotifyManager>(shared_from_this()),
        std::placeholders::_1, std::placeholders::_2), 
        ipProxy_, serverip_, port843,
        std::bind(&MessageNotifyManager::NewData843Callback, 
        std::weak_ptr<MessageNotifyManager>(shared_from_this()), 
        roomid, userid, usertoken, 
        std::placeholders::_1, std::placeholders::_2));
    return true;
}

void MessageNotifyManager::NewConnect843Callback(std::weak_ptr<MessageNotifyManager> weakptr, 
    bool result, TcpHandle handle)
{
    auto obj = weakptr.lock();
    if (!obj)
        return;

    obj->baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoNewConnect843Callback, 
                   obj.get(), result, handle));
}

void MessageNotifyManager::NewConnect8080Callback(std::weak_ptr<MessageNotifyManager> weakptr, 
    uint32 roomid, uint32 userid,
    const std::string& usertoken,
    bool result, TcpHandle handle)
{
    auto obj = weakptr.lock();
    if (!obj)
        return;

    obj->baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoNewConnect8080Callback,
        obj.get(), roomid, userid, usertoken, result, handle));
}

void MessageNotifyManager::NewData843Callback(std::weak_ptr<MessageNotifyManager> weakptr, 
    uint32 roomid, uint32 userid,
    const std::string& usertoken, bool result, const std::vector<uint8>& data)
{
    auto obj = weakptr.lock();
    if (!obj)
        return;

    obj->baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoNewData843Callback,
        obj.get(), roomid, userid, usertoken, result, data));
}
void MessageNotifyManager::NewData8080Callback(std::weak_ptr<MessageNotifyManager> weakptr, 
    bool result, const std::vector<uint8>& data)
{
    auto obj = weakptr.lock();
    if (!obj)
        return;

    obj->baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoNewData8080Callback,
        obj.get(), result, data));
}

void MessageNotifyManager::DoNewConnect843Callback(bool result, TcpHandle handle)
{
    if (!result)
    {
        assert(false && L"连接错误，应该结束MessageNotifyManager的流程了");
        return;
    }

    std::string str = "<policy-file-request/>";
    std::vector<char> data;
    data.assign(str.begin(), str.end());
    data.push_back(0);
    tcphandle_843_ = handle;
    if (!tcpManager_->Send(handle, data,
        std::bind(&MessageNotifyManager::NewSendDataCallback,
        this, tcphandle_843_, std::placeholders::_1)))
    {
        assert(false && L"发送数据错误，要结束流程");
    }
}


bool MessageNotifyManager::NewConnect8080(uint32 roomid, uint32 userid,
                                          const std::string& usertoken)
{
    tcpManager_->AddClient(
        std::bind(&MessageNotifyManager::NewConnect8080Callback, 
        std::weak_ptr<MessageNotifyManager>(shared_from_this()), roomid,
        userid, usertoken, std::placeholders::_1, std::placeholders::_2),
        ipProxy_, serverip_, port8080,
        std::bind(&MessageNotifyManager::NewData8080Callback, 
        std::weak_ptr<MessageNotifyManager>(shared_from_this()), 
        std::placeholders::_1,
        std::placeholders::_2));
    return true;
}

void MessageNotifyManager::DoNewConnect8080Callback(uint32 roomid, uint32 userid,
                                               const std::string& usertoken,
                                               bool result, TcpHandle handle)
{
    if (!result)
    {
        assert(false && L"连接错误，应该结束MessageNotifyManager的流程了");
        return;
    }

    tcphandle_8080_ = handle;
    uint32 keytime = static_cast<uint32>(base::Time::Now().ToDoubleT());
    std::vector<uint8> data_for_send;
    cmd201package package = {
        201, roomid, userid, usertoken };
    GetFirstPackage(package, &data_for_send);
    std::vector<char> data_8080;
    data_8080.assign(data_for_send.begin(), data_for_send.end());
    tcpManager_->Send(tcphandle_8080_, data_8080,
        std::bind(&MessageNotifyManager::NewSendDataCallback,
        this, tcphandle_8080_, std::placeholders::_1));

    if (newRepeatingTimer_.IsRunning())
        newRepeatingTimer_.Stop();

    newRepeatingTimer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(10), 
        base::Bind(&MessageNotifyManager::DoNewSendHeartBeat, this, tcphandle_8080_));

    return;
}

void MessageNotifyManager::DoNewData843Callback(uint32 roomid, uint32 userid,
    const std::string& usertoken, bool result, const std::vector<uint8>& data)
{
    tcpManager_->RemoveClient(tcphandle_843_);
    if (!result)
    {
        //assert(false && L"连接错误，应该结束MessageNotifyManager的流程了");
        return;
    }

    NewConnect8080(roomid, userid, usertoken);
}

void MessageNotifyManager::DoNewData8080Callback(bool result, const std::vector<uint8>& data)
{
    if (!result)
    {
        newRepeatingTimer_.Stop();
        tcpManager_->RemoveClient(tcphandle_8080_);
        return;
    }

    if (data.empty())
        return;
    
    std::vector<char> temp(data.begin(), data.end());
    Notify(temp);
}

void MessageNotifyManager::DoNewSendHeartBeat(TcpHandle handle)
{
    std::string heartbeat = "HEARTBEAT_REQUEST";
    heartbeat.append("\n");
    std::vector<char> heardbeatvec;
    heardbeatvec.assign(heartbeat.begin(), heartbeat.end());
    tcpManager_->Send(handle, heardbeatvec, 
        std::bind(&MessageNotifyManager::NewSendDataCallback,
        this, handle, std::placeholders::_1));
}

void MessageNotifyManager::DoNewSendChatMessage(const std::vector<char>& msg)
{
    // 间隔小于设置的值，重新投递消息
    if (base::Time::Now() - last_chat_time_ <= chat_message_space_)
    {
        baseThread_.message_loop_proxy()->PostDelayedTask(FROM_HERE,
            base::Bind(&MessageNotifyManager::DoNewSendChatMessage, this, msg), 
            base::Time::Now() - last_chat_time_);
        return;
    }

    last_chat_time_ = base::Time::Now();

    tcpManager_->Send(tcphandle_8080_, msg, 
        std::bind(&MessageNotifyManager::NewSendDataCallback,
        this, tcphandle_8080_, std::placeholders::_1));
    
}

void MessageNotifyManager::NewSendDataCallback(TcpHandle handle, bool result)
{
    std::wstring state = L"发送数据正常";
    if (!result)
    {
        state = L"发送数据异常，请重新进入房间";
        newRepeatingTimer_.Stop();
        tcpManager_->RemoveClient(handle);
    }

    if (normalNotify_)
        normalNotify_(state);
}

bool MessageNotifyManager::NewSendChatMessage(const std::string& nickname, uint32 richlevel,
                                              const std::string& message)
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

    std::wstring ws_message = base::UTF8ToWide(message);
    LOG(INFO) << __FUNCTION__ << L" " << base::SysWideToMultiByte(ws_message, 936);

    return baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoNewSendChatMessage, this, msg));
}

bool MessageNotifyManager::NewSendChatMessageRobot(const RoomChatMessage& roomChatMessage)
{
    Json::FastWriter writer;
    Json::Value root(Json::objectValue);
    root["cmd"] = 501;
    root["nickname"] = roomChatMessage.sendername;
    root["richlevel"] = base::UintToString(roomChatMessage.richlevel);
    root["receiverid"] = roomChatMessage.receiverid;
    root["receivername"] = roomChatMessage.receivername;
    root["chatmsg"] = roomChatMessage.chatmessage;
    root["issecrect"] = roomChatMessage.issecrect;

    std::string data = writer.write(root);
    std::vector<char> msg;
    msg.assign(data.begin(), data.end());

    std::wstring message = base::UTF8ToWide(roomChatMessage.chatmessage);
    LOG(INFO) << __FUNCTION__ << L" " << base::SysWideToMultiByte(message, 936);

    return baseThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoNewSendChatMessage, this, msg));
}