#include "MessageNotifyManager.h"
#include <assert.h>
#include <memory>

#include "IpProxy.h"

#include "EncodeHelper.h"
#include "Network/WebsocketDefine.h"
#include "Network/WebsocketClientController.h"

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

struct cmd201package
{
    uint32 cmd;
    uint32 roomid;
    uint32 userid;
    std::string usertoken;
	std::string soctoken;
};
// 这个数据包应该从http请求那边返回过来设置
bool GetFirstPackage(const cmd201package& package, 
    std::vector<uint8> *packagedata)
{
    // 10位的时间截
    //uint32 nowtime = static_cast<uint32>(base::Time::Now().ToDoubleT());
    //std::string message = R"({"cmd":201,"roomid":1417487,"kugouid":615887139,
    //"token":"3808e2cb686c5f3accb4dcd831b2048f7202d77efe0617b58e54e3dc8a8c70e1",
    //    "appid":1010,"referer":0,"clientid":100,"v":20171111,
    //    "soctoken":"121252524f4993cb81342253d1f7e313352386ad29357db11867451"})";

    Json::FastWriter writer;
    Json::Value root(Json::objectValue);
    root["cmd"] = package.cmd;
    root["roomid"] = package.roomid;
    root["kugouid"] = package.userid;
    root["token"] = package.usertoken;
    root["appid"] = 1010;
    root["referer"] = 0;
    root["clientid"] = 100;
    root["v"] = 20171111;
	root["soctoken"] = package.soctoken;

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


bool CommandHandle_201(const Json::Value& jvalue, EnterRoomUserInfo* info, std::string* outmsg)
{
	if (!info)
		return false;
	
	// 两层connect
	Json::Value jvContent(Json::ValueType::objectValue);
	Json::Value  content = jvalue.get("content", jvContent);
	if (content.isNull())
	{
		assert(false);
		return false;
	}

	EnterRoomUserInfo enterRoomUserInfo;
	Json::Value jvString("");
	uint32 senderid = GetInt32FromJsonValue(content, "senderid");
	uint32 senderkugouid = GetInt32FromJsonValue(content, "senderkugouid");
	enterRoomUserInfo.userid = senderid;

	std::string extentinfo = content.get("ext", "").asString();
	std::string decodeext = UrlDecode(extentinfo);

	Json::Reader reader;
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
					else if (member.compare("c") == 0)
					{
						uint32 visable = GetInt32FromJsonValue(vipData, member);
						enterRoomUserInfo.visable = (visable == 0); // 0是不开隐身的，是可见的；1是开隐身的，是不可见的。
					}
				}// for vipData_members
			}
		}// for extdata_members
	}//if reader

	try
	{
        Json::Value jvContent(Json::ValueType::objectValue);
		Json::Value  inner_content = content.get("content", jvContent);
		if (inner_content.isNull())
        {
            assert(false);
            return false;
        }
        Json::Value jvString("");
		std::string nickname = inner_content.get("nickname", jvString).asString();
		enterRoomUserInfo.nickname = nickname;
		uint32 richlevel = GetInt32FromJsonValue(inner_content, "richlevel");
		enterRoomUserInfo.richlevel = richlevel;
		uint32 userid = GetInt32FromJsonValue(inner_content, "userid");
		DCHECK(enterRoomUserInfo.userid == userid);
        *outmsg = base::WideToUTF8(L"进入房间通知") + nickname;
    }
    catch (...)
    {
        assert(false);
        return false;
    }
    *info = enterRoomUserInfo;
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
        // 两层connect
        Json::Value jvContent(Json::ValueType::objectValue);
        Json::Value  outside_content = jvalue.get("content", jvContent);
        if (outside_content.isNull())
        {
            assert(false);
            return false;
        }

        uint32 roomid = GetInt32FromJsonValue(outside_content, "roomid");
        uint32 time = GetInt32FromJsonValue(outside_content, "time");

        // 两层connect
        Json::Value default_content(Json::ValueType::objectValue);
        Json::Value  content = outside_content.get("content", default_content);
        if (content.isNull())
        {
            assert(false);
            return false;
        }

        Json::Value jvString("");
        uint32 receiverid = GetInt32FromJsonValue(content, "receiverid");    
        uint32 receiverkugouid = GetInt32FromJsonValue(content, "receiverkugouid"); // 新增加
        uint32 receiverrichlevel = GetInt32FromJsonValue(content, "receiverrichlevel"); // 新增加
        std::string receivername = content.get("receivername", jvString).asString();

        uint32 senderid = GetInt32FromJsonValue(content, "senderid");
        uint32 senderkugouid = GetInt32FromJsonValue(content, "senderkugouid");
		uint32 senderrichlevel = GetInt32FromJsonValue(content, "senderrichlevel");
        std::string sendername = content.get("sendername", jvString).asString();
        uint32 issecrect = GetInt32FromJsonValue(content, "issecrect");//是否私聊
        std::string chatmsg = content.get("chatmsg", jvString).asString();

        *outmsg = base::WideToUTF8(L"聊天消息:") + chatmsg;

        enterRoomUserInfo->roomid = roomid;
        enterRoomUserInfo->unixtime = time;
		enterRoomUserInfo->nickname = sendername;
		enterRoomUserInfo->richlevel = senderrichlevel;
		enterRoomUserInfo->userid = senderid;

        roomChatMessage->roomid = roomid;
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

bool CommandHandle_601(const Json::Value& jvalue, RoomGiftInfo601* gift_info, std::string* outmsg)
{
    RoomGiftInfo601 temp;
    // 房间聊天消息
    try
    {
        // 两层connect
        Json::Value jvContent(Json::ValueType::objectValue);
        Json::Value  outside_content = jvalue.get("content", jvContent);
        if (outside_content.isNull())
        {
            assert(false);
            return false;
        }

        uint32 roomid = GetInt32FromJsonValue(outside_content, "roomid");
        uint32 time = GetInt32FromJsonValue(outside_content, "time");

        // 两层connect
        Json::Value default_content(Json::ValueType::objectValue);
        Json::Value  content = outside_content.get("content", default_content);
        if (content.isNull())
        {
            assert(false);
            return false;
        }

        Json::Value jvString("");

        uint32 actionId = GetInt32FromJsonValue(content, "actionId");
        std::string token = content.get("token", jvString).asString();
        uint32 senderid = GetInt32FromJsonValue(content, "senderid");
        std::string sendername = content.get("sendername", jvString).asString();
        uint32 receiverid = GetInt32FromJsonValue(content, "receiverid");
        std::string receivername = content.get("receivername", jvString).asString();

        uint32 giftid = GetInt32FromJsonValue(content, "giftid");
        std::string giftname = content.get("giftname", jvString).asString();
        uint32 gitfnumber = GetInt32FromJsonValue(content, "num");
        std::string tips = content.get("tip", jvString).asString();
        uint32 happytype = GetInt32FromJsonValue(content, "happyType");
        uint32 happyObj = GetInt32FromJsonValue(content, "happyObj");//是否是幸运礼物

        *outmsg = base::WideToUTF8(L"房间礼物数据 ") + tips;

        temp.time = time;
        temp.roomid = roomid;
        temp.senderid = senderid;
        temp.sendername = sendername;
        temp.receiverid = receiverid;
        temp.receivername = receivername;
        temp.giftid = giftid;
        temp.giftname = giftname;
        temp.gitfnumber = gitfnumber;
        temp.tips = tips;
        temp.happytype = happytype;
        temp.happyobj = happyObj;

    }
    catch (...)
    {
        assert(false);
        return false;
    }

    *gift_info = temp;
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
    //:tcpClient_843_(new TcpClient),
    //tcpClient_8080_(new TcpClient),
    :notify201_(nullptr),
    notify501_(nullptr),
    notify601_(nullptr),
    //baseThread_("NetworkHelperThread" + base::IntToString(threadindex)),
    runner_(nullptr),
    conn_break_callback_(),
    connected_(false),
    chat_message_space_(base::TimeDelta::FromMilliseconds(char_millisecond_space)),
    last_chat_time_(base::Time::Now())
{
}

MessageNotifyManager::~MessageNotifyManager()
{
    
}

bool MessageNotifyManager::Initialize(const scoped_refptr<base::TaskRunner>& runner)
{
    //tcpClient_843_->Initialize();
    //tcpClient_8080_->Initialize();
    runner_ = runner;
    //baseThread_.Start();
    return true;
}
void MessageNotifyManager::Finalize()
{
    if (repeatingTimer_.IsRunning())
        repeatingTimer_.Stop();

    // 这个时候，tcpManager_的线程已经结束，不要再抛任务了
    //tcpManager_->RemoveClient(SocketHandle_843_);
    //tcpManager_->RemoveClient(SocketHandle_8080_);
    if (newRepeatingTimer_.IsRunning())
        newRepeatingTimer_.Stop();

    //if (baseThread_.IsRunning())
    //    baseThread_.Stop();
    
    //tcpClient_843_->Finalize();
    //tcpClient_8080_->Finalize();
}

void MessageNotifyManager::SetWebsockClientController(
    WebsocketClientController* controller)
{
    websocket_client_controller_ = controller;
}

void MessageNotifyManager::SetServerIp(const std::string& serverip, uint16 port)
{
    serverip_ = serverip;
    port_ = port;
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
    runner_->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSetNotify201, this, notify201));
}

void MessageNotifyManager::SetNotify501(Notify501 notify501)
{
    runner_->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSetNotify501, this, notify501));
}

void MessageNotifyManager::SetNotify601(Notify601 notify601)
{
    notify601_ = notify601;
    runner_->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSetNotify601, this, notify601));
}

void MessageNotifyManager::SetNotify620(Notify620 notify_620)
{
    notify_620_ = notify_620;
    runner_->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSetNotify620, this, notify_620));
}

void MessageNotifyManager::SetNormalNotify(NormalNotify normalNotify)
{
    runner_->PostTask(FROM_HERE,
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

void MessageNotifyManager::DoSetNotify620(Notify620 notify620)
{
    notify_620_ = notify620;
}

void MessageNotifyManager::DoSetNormalNotify(NormalNotify normalNotify)
{
    normalNotify_ = normalNotify;
}

bool MessageNotifyManager::Connect(uint32 room_id, uint32 user_id,
    const std::string& usertoken, const std::string& soctoken,
    const base::Callback<void()>& conn_break_callback)
{
    DCHECK(room_id);
    DCHECK(user_id);
    DCHECK(!usertoken.empty());
    DCHECK(!soctoken.empty());

    message_count_ = 0;

	conn_break_callback_ = conn_break_callback;

    auto add_callback = std::bind(&MessageNotifyManager::AddClientConnectCallback,
		this, room_id, user_id, usertoken, soctoken, std::placeholders::_1, std::placeholders::_2);

    auto data_callback = std::bind(&MessageNotifyManager::ClientDataCallback,
        this, std::placeholders::_1, std::placeholders::_2);

	auto break_callback = std::bind(&MessageNotifyManager::ConnectBreakCallback,
		this, conn_break_callback, std::placeholders::_1);

    websocket_client_controller_->AddClient(add_callback, break_callback, ipProxy_, serverip_, port_, data_callback);

    return true;
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
void MessageNotifyManager::Notify(const std::vector<uint8>& data)
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
        //uint32 roomid = GetInt32FromJsonValue(rootdata, "roomid");
        //uint32 senderid = GetInt32FromJsonValue(rootdata, "senderid");
            
        uint32 time = GetInt32FromJsonValue(rootdata, "time");
        std::string str_display_time = MakeFormatTimeStringFromUnixTime(time);

    //    else if (cmd==620)
    //    {
    //        std::vector<BetShowData> bet_show_datas;
    //        BetResult bet_result;
    //        bet_result.time = time;
    //        if (CommandHandle_620(rootdata, &bet_show_datas, &bet_result))
    //        {
    //            if (!bet_show_datas.empty())
    //            {
    //                if (bet_show_datas_.empty())
    //                {
    //                    bet_show_datas_ = bet_show_datas;
    //                }
    //                else
    //                {
    //                    auto show_data = bet_show_datas.begin();
    //                    auto show_data_ = bet_show_datas_.begin();
    //                    for (; (show_data != bet_show_datas.end()) 
    //                        && (show_data_ != bet_show_datas_.end());
    //                        show_data++, show_data_++)
    //                    {
    //                        if ((show_data->bet_gid != show_data_->bet_gid)
    //                            || (show_data->odds != show_data_->odds))
    //                        {
    //                            assert(false);
    //                            LOG(ERROR) << L"两次通知的开奖数据不一致，有问题";
    //                            break;
    //                        }
    //                    }
    //                }
    //                //LOG(INFO) << L"tulong +++++++++++++++++++++++++++++++++++++++++++++";
    //            }
    //            else // 开奖结果数据
    //            {
    //                LOG(INFO) << L"tulong result = " + base::UintToString16(bet_result.result) +
    //                    L" random = " + base::UintToString16(bet_result.random);
    //            }
    //            if (notify_620_)
    //            {
    //                notify_620_(bet_result);
    //            }
    //        }
    //    }

		EnterRoomUserInfo enterRoomUserInfo;
        std::string outmsg;
        switch (cmd)
        {
        case 100:
            break;
        case 201:
			if (!notify201_)
				break;
			if (!CommandHandle_201(rootdata, &enterRoomUserInfo, &outmsg))
				break;

			normalNotify_(MessageLevel::MESSAGE_LEVEL_DISPLAY, base::UTF8ToWide(outmsg));
			notify201_(enterRoomUserInfo);
            break;
        case 315:
            //CommandHandle_315(rootdata, &outmsg);
            break;
        case 501:
        {
			if (!notify501_)
				break;

            RoomChatMessage roomChatMessage;
            if (!CommandHandle_501(rootdata, &enterRoomUserInfo, &roomChatMessage, &outmsg))
				break;

			notify501_(enterRoomUserInfo, roomChatMessage);
            break;
        }
        case 601:
        {
            RoomGiftInfo601 gift_info;
            CommandHandle_601(rootdata, &gift_info, &outmsg);
            if (notify601_)
            {
                notify601_(gift_info);
            }
            break;
        }
          
        case 602:
            //CommandHandle_602(rootdata, &outmsg);
            //{
            //    std::wstring wstr_display_time = base::UTF8ToUTF16(str_display_time);
            //    if (normalNotify_)
            //        normalNotify_(MessageLevel::MESSAGE_LEVEL_ONCE, wstr_display_time + L" 房间状态正常");
            //}
            break;
        case 606:
            //CommandHandle_606(rootdata, &outmsg);
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
            normalNotify_(MessageLevel::MESSAGE_LEVEL_DEBUG, wstr);
            if (!outmsg.empty())
            {
                wstr = base::UTF8ToWide(outmsg);
                normalNotify_(MessageLevel::MESSAGE_LEVEL_DEBUG, wstr);
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

//bool MessageNotifyManager::NewConnect843(
//    uint32 roomid, uint32 userid,
//    const std::string& usertoken,
//    const base::Callback<void()>& conn_break_callback)
//{
//    conn_break_callback_ = conn_break_callback;
//    bool result = tcp_client_controller_->AddClient(
//        std::bind(&MessageNotifyManager::NewConnect843Callback, 
//        std::weak_ptr<MessageNotifyManager>(shared_from_this()),
//        std::placeholders::_1, std::placeholders::_2), 
//        ipProxy_, serverip_, port843,
//        std::bind(&MessageNotifyManager::NewData843Callback, 
//        std::weak_ptr<MessageNotifyManager>(shared_from_this()), 
//        roomid, userid, usertoken, 
//        std::placeholders::_1, std::placeholders::_2));
//    return result;
//}
//
//void MessageNotifyManager::NewConnect843Callback(std::weak_ptr<MessageNotifyManager> weakptr, 
//    bool result, SocketHandle handle)
//{
//    auto obj = weakptr.lock();
//    if (!obj)
//        return;
//
//    obj->runner_->PostTask(FROM_HERE,
//        base::Bind(&MessageNotifyManager::DoNewConnect843Callback, 
//                   obj.get(), result, handle));
//}
//
//void MessageNotifyManager::NewConnect8080Callback(std::weak_ptr<MessageNotifyManager> weakptr, 
//    uint32 roomid, uint32 userid,
//    const std::string& usertoken,
//    bool result, SocketHandle handle)
//{
//    auto obj = weakptr.lock();
//    if (!obj)
//        return;
//
//    obj->runner_->PostTask(FROM_HERE,
//        base::Bind(&MessageNotifyManager::DoNewConnect8080Callback,
//        obj.get(), roomid, userid, usertoken, result, handle));
//}
//
//void MessageNotifyManager::NewData843Callback(std::weak_ptr<MessageNotifyManager> weakptr, 
//    uint32 roomid, uint32 userid,
//    const std::string& usertoken, bool result, const std::vector<uint8>& data)
//{
//    auto obj = weakptr.lock();
//    if (!obj)
//        return;
//
//    obj->runner_->PostTask(FROM_HERE,
//        base::Bind(&MessageNotifyManager::DoNewData843Callback,
//        obj.get(), roomid, userid, usertoken, result, data));
//}
//void MessageNotifyManager::NewData8080Callback(std::weak_ptr<MessageNotifyManager> weakptr, 
//    bool result, const std::vector<uint8>& data)
//{
//    auto obj = weakptr.lock();
//    if (!obj)
//        return;
//
//    obj->runner_->PostTask(FROM_HERE,
//        base::Bind(&MessageNotifyManager::DoNewData8080Callback,
//        obj.get(), result, data));
//}
//
//void MessageNotifyManager::NewSendDataCallback(
//    std::weak_ptr<MessageNotifyManager> weakptr, SocketHandle handle, bool result)
//{
//    auto obj = weakptr.lock();
//    if (!obj)
//        return;
//
//    obj->runner_->PostTask(FROM_HERE,
//        base::Bind(&MessageNotifyManager::DoNewSendDataCallback,
//        obj.get(), handle, result));
//}
//
//void MessageNotifyManager::DoNewConnect843Callback(bool result, SocketHandle handle)
//{
//    if (!result)
//    {
//        assert(false && L"连接错误，应该结束MessageNotifyManager的流程了");
//        connected_ = false;
//        conn_break_callback_.Run();
//        return;
//    }
//
//    std::string str = "<policy-file-request/>";
//    std::vector<uint8> data;
//    data.assign(str.begin(), str.end());
//    data.push_back(0);
//    SocketHandle_843_ = handle;
//    if (!tcp_client_controller_->Send(handle, data,
//        std::bind(&MessageNotifyManager::NewSendDataCallback,
//        std::weak_ptr<MessageNotifyManager>(shared_from_this()), 
//        SocketHandle_843_, std::placeholders::_1)))
//    {
//        assert(false && L"发送数据错误，要结束流程");
//        // 要向上层通知，确认是否出错重连
//        connected_ = false;
//        conn_break_callback_.Run();
//    }
//}
//
//
//bool MessageNotifyManager::NewConnect8080(uint32 roomid, uint32 userid,
//                                          const std::string& usertoken)
//{
//    tcp_client_controller_->AddClient(
//        std::bind(&MessageNotifyManager::NewConnect8080Callback, 
//        std::weak_ptr<MessageNotifyManager>(shared_from_this()), roomid,
//        userid, usertoken, std::placeholders::_1, std::placeholders::_2),
//        ipProxy_, serverip_, port8080,
//        std::bind(&MessageNotifyManager::NewData8080Callback, 
//        std::weak_ptr<MessageNotifyManager>(shared_from_this()), 
//        std::placeholders::_1,
//        std::placeholders::_2));
//    return true;
//}
//
//void MessageNotifyManager::DoNewConnect8080Callback(uint32 roomid, uint32 userid,
//                                               const std::string& usertoken,
//                                               bool result, SocketHandle handle)
//{
//    if (!result)
//    {
//        assert(false && L"连接错误，应该结束MessageNotifyManager的流程了");
//        // 要向上层通知，确认是否出错重连
//        connected_ = false;
//        conn_break_callback_.Run();
//        return;
//    }
//
//    connected_ = true; // 唯一标记连接成功的地方
//    SocketHandle_8080_ = handle;
//    uint32 keytime = static_cast<uint32>(base::Time::Now().ToDoubleT());
//    std::vector<uint8> data_for_send;
//    cmd201package package = {
//        201, roomid, userid, usertoken };
//    GetFirstPackage(package, &data_for_send);
//    std::vector<uint8> data_8080;
//    data_8080.assign(data_for_send.begin(), data_for_send.end());
//    tcp_client_controller_->Send(SocketHandle_8080_, data_8080,
//        std::bind(&MessageNotifyManager::NewSendDataCallback,
//        std::weak_ptr<MessageNotifyManager>(shared_from_this()), 
//        SocketHandle_8080_, std::placeholders::_1));
//
//    if (newRepeatingTimer_.IsRunning())
//        newRepeatingTimer_.Stop();
//
//    newRepeatingTimer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(10), 
//        base::Bind(&MessageNotifyManager::DoNewSendHeartBeat, this, SocketHandle_8080_));
//
//    return;
//}
//
//void MessageNotifyManager::DoNewData843Callback(uint32 roomid, uint32 userid,
//    const std::string& usertoken, bool result, const std::vector<uint8>& data)
//{
//    if (!result)
//    {
//        assert(false && L"连接错误，应该结束MessageNotifyManager的流程了");
//        // 要向上层通知，确认是否出错重连
//        connected_ = false;
//        conn_break_callback_.Run();
//        return;
//    }
//
//    NewConnect8080(roomid, userid, usertoken);
//}
//
//void MessageNotifyManager::DoNewData8080Callback(bool result, const std::vector<uint8>& data)
//{
//    if (!connected_) // 如果已经被标记为断开连接，那么不要回调了，此时上层的room可能已经被使用者删除
//        return;
//
//    //if (SocketHandle_843_ > 0)
//    //{
//    //    tcp_client_controller_->RemoveClient(SocketHandle_843_);
//    //    SocketHandle_843_ = 0;
//    //}
//
//    if (!result)
//    {
//        newRepeatingTimer_.Stop();
//        tcp_client_controller_->RemoveClient(SocketHandle_8080_);
//        // 要向上层通知，确认是否出错重连
//        connected_ = false;
//        conn_break_callback_.Run();
//        return;
//    }
//
//    if (data.empty())
//        return;
//    
//    std::vector<char> temp(data.begin(), data.end());
//    Notify(temp);
//}
//
//void MessageNotifyManager::DoNewSendHeartBeat(SocketHandle handle)
//{
//    std::string heartbeat = "HEARTBEAT_REQUEST";
//    heartbeat.append("\n");
//    std::vector<uint8> heardbeatvec;
//    heardbeatvec.assign(heartbeat.begin(), heartbeat.end());
//    tcp_client_controller_->Send(handle, heardbeatvec, 
//        std::bind(&MessageNotifyManager::NewSendDataCallback,
//        std::weak_ptr<MessageNotifyManager>(shared_from_this()),
//        handle, std::placeholders::_1));
//}
//
//void MessageNotifyManager::DoNewSendChatMessage(const std::vector<char>& msg)
//{
//    // 间隔小于设置的值，重新投递消息
//    if (base::Time::Now() - last_chat_time_ <= chat_message_space_)
//    {
//        runner_->PostDelayedTask(FROM_HERE,
//            base::Bind(&MessageNotifyManager::DoNewSendChatMessage, this, msg), 
//            base::Time::Now() - last_chat_time_);
//        return;
//    }
//
//    last_chat_time_ = base::Time::Now();
//
//    std::vector<uint8> data(msg.begin(), msg.end());
//    tcp_client_controller_->Send(SocketHandle_8080_, data,
//        std::bind(&MessageNotifyManager::NewSendDataCallback,
//        std::weak_ptr<MessageNotifyManager>(shared_from_this()),
//        SocketHandle_8080_, std::placeholders::_1));
//    
//}
//
//void MessageNotifyManager::DoNewSendDataCallback(SocketHandle handle, bool result)
//{
//    if (!connected_)
//        return;
//
//    std::wstring state = L"发送数据正常";
//    if (!result)
//    {
//        state = L"发送数据异常，请重新进入房间";
//        newRepeatingTimer_.Stop();
//        tcp_client_controller_->RemoveClient(handle);
//        // 要向上层通知，确认是否出错重连
//        conn_break_callback_.Run();
//    }
//
//    if (normalNotify_)
//        normalNotify_(MessageLevel::MESSAGE_LEVEL_ONCE, state);
//}

bool MessageNotifyManager::SendChatMessage(const std::string& nickname, uint32 richlevel,
                                              const std::string& message)
{
    //Json::FastWriter writer;
    //Json::Value root(Json::objectValue);
    //root["cmd"] = 501;
    ////root["nickname"] = nickname;
    ////root["richlevel"] = base::UintToString(richlevel);
    //root["receiverid"] = 0;
    ////root["receivername"] = "";
    //root["chatmsg"] = message;
    //root["issecrect"] = 0;
    //int64 sequenceid = 0;
    //base::StringToInt64(GetNowTimeString(), &sequenceid);
    //root["seq"] = double(sequenceid);
    //root["v"] = 20171111;

    //std::string data = writer.write(root);
    std::string data = "{\"cmd\":501";
    data += ",\"chatmsg\": \"" +message + "\"";
    data += ",\"receiverid\":0";
    data += ",\"issecrect\":0";
    data += ",\"seq\":" + GetNowTimeString();
    data += ",\"v\" : 20171111}";

    std::vector<uint8> msg;
    msg.assign(data.begin(), data.end());

    std::wstring ws_message = base::UTF8ToWide(message);
    LOG(INFO) << __FUNCTION__ << L" " << base::SysWideToMultiByte(ws_message, 936);

    runner_->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSendMessage,
        base::Unretained(this), msg));

    return true;
}

void MessageNotifyManager::DoSendMessage(const std::vector<uint8>& message)
{
    websocket_client_controller_->Send(websocket_handle_, message,
        std::bind(&MessageNotifyManager::DoSendDataCallback, this,
        websocket_handle_, std::placeholders::_1));
}

bool MessageNotifyManager::NewSendChatMessageRobot(const RoomChatMessage& roomChatMessage)
{
    Json::FastWriter writer;
    Json::Value root(Json::objectValue);
    root["cmd"] = 501;
    //root["nickname"] = roomChatMessage.sendername;
    //root["richlevel"] = base::UintToString(roomChatMessage.richlevel);
    root["receiverid"] = roomChatMessage.receiverid;
    //root["receivername"] = roomChatMessage.receivername;
    root["chatmsg"] = roomChatMessage.chatmessage;
    root["issecrect"] = roomChatMessage.issecrect ? "1" : "0";
    int64 sequenceid = 0;
    base::StringToInt64(GetNowTimeString(), &sequenceid);
    root["seq"] = double(sequenceid);;
    root["v"] = 20171111;

    std::string data = writer.write(root);
    std::vector<uint8> message;
    message.assign(data.begin(), data.end());

    std::wstring ws_message = base::UTF8ToWide(data);
    LOG(INFO) << __FUNCTION__ << L" " << base::SysWideToMultiByte(ws_message, 936);

    runner_->PostTask(FROM_HERE,
        base::Bind(&MessageNotifyManager::DoSendMessage,
        base::Unretained(this), message));

    return true;
}
//
//bool MessageNotifyManager::Connect(
//    uint32 room_id, uint32 user_id, const std::string& usertoken,
//    const std::string& soctoken, const base::Callback<void()>& conn_break_callback)
//{
//    conn_break_callback_ = conn_break_callback;
//    bool result = websocket_client_controller_->AddClient(
//        std::bind(&MessageNotifyManager::AddClientConnectCallback,
//        //std::weak_ptr<MessageNotifyManager>(shared_from_this()),
//        this,
//        std::placeholders::_1, std::placeholders::_2),
//        ipProxy_, serverip_, port843,
//        std::bind(&MessageNotifyManager::ClientDataCallback,
//        //std::weak_ptr<MessageNotifyManager>(shared_from_this()),
//        this,
//        room_id, user_id, usertoken,
//        std::placeholders::_1, std::placeholders::_2));
//
//    return result;
//}
//
//bool MessageNotifyManager::SendMessage(const std::string& nickname, uint32 richlevel,
//    const std::string& message)
//{
//    std::vector<uint8> data;
//    websocket_client_controller_->Send(websocket_handle_, data, 
//        std::bind(&MessageNotifyManager::DoSendDataCallback, this, 
//        std::placeholders::_1, std::placeholders::_2));
//    return true;
//}
//
//void MessageNotifyManager::AddClientConnectCallback(bool result, WebsocketHandle handle)
//{
//    if (!result)
//    {
//        assert(false && L"连接错误，应该结束MessageNotifyManager的流程了");
//        // 要向上层通知，确认是否出错重连
//        connected_ = false;
//        conn_break_callback_.Run();
//        return;
//    }
//
//    connected_ = true; // 唯一标记连接成功的地方
//
//    if (newRepeatingTimer_.IsRunning())
//        newRepeatingTimer_.Stop();
//
//    newRepeatingTimer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(10),
//        base::Bind(&MessageNotifyManager::DoNewSendHeartBeat, this, SocketHandle_8080_));
//
//    return;
//}
//
//void MessageNotifyManager::ClientDataCallback(
//    uint32 roomid, uint32 userid, const std::string& usertoken, bool result,
//    const std::vector<uint8>& data)
//{
//    // 处理第一个数据包到达，要发送数据
//    //uint32 keytime = static_cast<uint32>(base::Time::Now().ToDoubleT());
//    //std::vector<uint8> data_for_send;
//    //cmd201package package = {
//    //    201, roomid, userid, usertoken };
//    //GetFirstPackage(package, &data_for_send);
//    //std::vector<uint8> data_8080;
//    //data_8080.assign(data_for_send.begin(), data_for_send.end());
//    //tcp_client_controller_->Send(SocketHandle_8080_, data_8080,
//    //    std::bind(&MessageNotifyManager::NewSendDataCallback,
//    //    std::weak_ptr<MessageNotifyManager>(shared_from_this()),
//    //    SocketHandle_8080_, std::placeholders::_1));
//
//    return;
//}
//
//void MessageNotifyManager::DoSendDataCallback(WebsocketHandle handle, bool result)
//{
//    if (!connected_)
//        return;
//
//    std::wstring state = L"发送数据正常";
//    if (!result)
//    {
//        state = L"发送数据异常，请重新进入房间";
//        newRepeatingTimer_.Stop();
//        tcp_client_controller_->RemoveClient(handle);
//        // 要向上层通知，确认是否出错重连
//        conn_break_callback_.Run();
//    }
//
//    if (normalNotify_)
//        normalNotify_(MessageLevel::MESSAGE_LEVEL_ONCE, state);
//}

void MessageNotifyManager::AddClientConnectCallback(
	uint32 roomid, uint32 userid, const std::string& usertoken,
	const std::string& soctoken,
    bool result, WebsocketHandle handle)
{
    if (!result)
    {
        conn_break_callback_.Run();
		return;
    }

    websocket_handle_ = handle;

	if (!message_count_)
	{
		std::vector<uint8> data_for_send;
		cmd201package package = {
			201, roomid, userid, usertoken, soctoken };
		GetFirstPackage(package, &data_for_send);

		//std::string message = R"({"cmd":201,"roomid":1417487,"kugouid":615887139,"token":"3808e2cb686c5f3accb4dcd831b2048f7202d77efe0617b58e54e3dc8a8c70e1","appid":1010,"referer":0,"clientid":100,"v":20171111,"soctoken":"121252524f4993cb81342253d1f7e313352386ad29357db11867451"})";

		websocket_client_controller_->Send(websocket_handle_, data_for_send,
			std::bind(&MessageNotifyManager::DoSendDataCallback, this,
			websocket_handle_, std::placeholders::_1));

		runner_->PostTask(FROM_HERE,
			base::Bind(&MessageNotifyManager::StartSendHeartbeat,
			base::Unretained(this)));
	}
}

void MessageNotifyManager::ConnectBreakCallback(
	const base::Callback<void()>& conn_break_callback, WebsocketHandle handle)
{

}

void MessageNotifyManager::StartSendHeartbeat()
{
    if (newRepeatingTimer_.IsRunning())
        newRepeatingTimer_.Stop();

    newRepeatingTimer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(10),
        base::Bind(&MessageNotifyManager::SendHeartbeat, base::Unretained(this)));
}

void MessageNotifyManager::SendHeartbeat()
{
    std::string heartbeat = "HEARTBEAT_REQUEST";
    heartbeat.append("\n");
    std::vector<uint8> data(heartbeat.begin(), heartbeat.end());
    websocket_client_controller_->Send(websocket_handle_, data,
        std::bind(&MessageNotifyManager::DoSendDataCallback,this,
        websocket_handle_, std::placeholders::_1));
}

void MessageNotifyManager::ClientDataCallback(
    bool result, const std::vector<uint8>& data)
{
    Notify(data);
    message_count_++;
    LOG(INFO) << base::IntToString16(message_count_);
}

void MessageNotifyManager::DoSendDataCallback(WebsocketHandle handle, bool result)
{

}