#pragma once
#include <string>
#include "third_party/chromium/base/basictypes.h"

struct AntiFloodAuthority
{
    uint32 userid = 0;
    uint32 roomid = 0;
    uint32 clanid = 0;
    uint32 kickout = 0;
    uint32 banchat = 0;
    uint32 antiadvance = 0;
    uint64 expiretime = 0;
    std::string serverip = "";
};

struct UserTrackerAuthority
{
    uint32 user_id = 0;
    uint64 expiretime = 0;
    std::string tracker_host = "";// 保证未授权的情况没有配置使用目标服务器
};

// 提供授权文件明文格式读写功能
class AuthorityHelper
{
public:
    AuthorityHelper();
    ~AuthorityHelper();

    bool LoadUserTrackerAuthority(UserTrackerAuthority* authority);
    bool GetTrackerAuthorityDisplayInfo(const UserTrackerAuthority& authority, 
        std::wstring* display);
    // 修改授权文件使其无法正常读取
    bool DestoryTrackAuthority();
};

