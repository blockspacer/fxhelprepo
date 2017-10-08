#pragma once
#include <string>
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/callback.h"

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

struct FamilyDataAuthority
{
    std::string username = 0;
    uint64 expiretime = 0;
    std::string family_data_host = "";// ��֤δ��Ȩ�����û������ʹ��Ŀ�������
};

// �ṩ��Ȩ�ļ����ĸ�ʽ��д����
class AuthorityHelper
{
public:
    AuthorityHelper();
    ~AuthorityHelper();

    // ����������ݵĹ���
    bool LoadUserAuthority(const std::wstring& filename_pre, 
        const base::Callback<void(const std::string&)> callback);

    bool LoadFailyaDataAuthority(FamilyDataAuthority* authority);
    bool GetFailyaDataAuthorityDisplayInfo(const FamilyDataAuthority& authority,
        std::wstring* display);
};

