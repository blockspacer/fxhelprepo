#pragma once
#include <string>
#include <vector>
#include "third_party/chromium/base/basictypes.h"

struct RoomInfo
{
    uint32 phone_room_id = 0;
    uint32 room_id = 0;
    std::wstring public_msg;
    std::wstring private_msg;
    uint32 billboard_month_1 = 0;
    uint32 billboard_month_2 = 0;
    uint32 billboard_month_3 = 0;
    uint32 billboard_month_4 = 0;
    uint32 billboard_month_5 = 0;
    uint32 billboard_all_1 = 0;
    uint32 billboard_all_2 = 0;
    uint32 billboard_all_3 = 0;
    uint32 billboard_all_4 = 0;
    uint32 billboard_all_5 = 0;
};

struct ClanInfo
{
    uint32 clan_id = 0;
    std::wstring clan_name;
    uint32 clan_singer_count = 0;
    uint32 clan_level = 0;
    std::wstring date;
};

struct UserInfo
{
    uint32 fanxing_id = 0;
    uint32 kugou_id = 0;
    uint32 clan_id = 0;
    std::wstring nickname;
    uint32 rich_level = 0;
    uint32 star_level = 0;
    uint32 fans_count = 0;
    uint32 follow_count = 0;
    std::wstring sex;
    std::wstring location;

};

struct SingerInfo
{
    UserInfo user_info;
    RoomInfo room_info;
    bool star_singer;
    std::wstring tags; // Ö÷²¥±êÇ©
    std::wstring last_online;
    uint32 last_online_day;
    std::wstring date;
};
