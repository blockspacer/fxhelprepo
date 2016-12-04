#pragma once
#include <string>
#include "third_party/chromium/base/basictypes.h"

struct WorshipAuthority
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

    bool LoadUserTrackerAuthority(WorshipAuthority* authority);
    bool GetTrackerAuthorityDisplayInfo(const WorshipAuthority& authority,
        std::wstring* display);
};

