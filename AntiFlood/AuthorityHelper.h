#pragma once
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
    std::string serverip = "192.168.0.1";
};

class AuthorityHelper
{
public:
    AuthorityHelper();
    ~AuthorityHelper();

    bool LoadAntiFloodAuthority(AntiFloodAuthority* authority);
    bool GetAuthorityDisplayInfo(std::wstring* display);
};

