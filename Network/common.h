#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "third_party/chromium/base/basictypes.h"

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
    int64 msgid;
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

//typedef int SocketHandle;
// 201��Ϣ�����������ȥ�����ݰ�
struct EnterRoomUserInfo
{
    std::string nickname = "";
    uint32 richlevel = 0;
    uint32 roomid = 0;
    uint32 unixtime = 0;
    uint32 userid = 0;
    bool vip_v = false; // vipData��v�ֶ��� 1Ϊ��ͨvip, 2Ϊ�׽�vip�����Կ��������ܵġ�
    bool visable = true; // �Ƿ�����Ĭ���ǿɼ���������������򲻿ɼ�
    bool isAdmin = false;
    bool isUserGuard = false;
    bool isGoldFans = false;
};
