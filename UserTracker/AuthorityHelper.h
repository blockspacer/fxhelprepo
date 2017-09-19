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
    std::string tracker_host = "";// ��֤δ��Ȩ�����û������ʹ��Ŀ�������
};

// �ṩ��Ȩ�ļ����ĸ�ʽ��д����
class AuthorityHelper
{
public:
    AuthorityHelper();
    ~AuthorityHelper();

    bool LoadUserTrackerAuthority(UserTrackerAuthority* authority);
    bool GetTrackerAuthorityDisplayInfo(const UserTrackerAuthority& authority, 
        std::wstring* display);
    // �޸���Ȩ�ļ�ʹ���޷�������ȡ
    bool DestoryTrackAuthority();
};

