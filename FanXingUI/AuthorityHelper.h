#pragma once
#include "third_party/chromium/base/basictypes.h"

struct Authority
{
    uint32 userid;
    uint32 roomid;
    uint32 clanid;
    uint32 kickout;
    uint32 banchat;
    uint32 antiadvance;
};


class AuthorityHelper
{
public:
    AuthorityHelper();
    ~AuthorityHelper();

    bool Load(Authority* authority);
};

