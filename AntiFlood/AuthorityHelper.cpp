#include "stdafx.h"
#include <fstream>
#include <sstream>

#include "AuthorityHelper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_enumerator.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

#include "third_party/json/json.h"

namespace
{
    const char* privatekey = "30820274020100300D06092A864886F70D01010105000482025E3082025A02010002818100AF09717C117DEEB146357595C7FC4A7AAB4501523B96EF3784615F20818C4DC6C0A242E900C0B0AFDB758F4F2D5536701787B6668506D88BB20C3BB4B71823113CB238E62C58E275638E51F6E43D6A6A32B15152472857CEEDEAED02CD43EAA896B4F7A5D567D09051BE006D2B6BF8D0B0717CA641F096617A1BB3E7105595EB02011102818001B74EEAA0CC875C1746CC7268DCD38DC06C991C69FEF84E59241785569A793BAC8E23CB1193752EE68B31175D4EA3ABC3FED21A1A66F81572AFCE63A7712D852C1B0CC0B2ABA8970301EBE5963FA82E5EE89FCADD96B4E473C2C0BA6E3FD0FE2EFBA1505BE9A09A6D51C2CE1C6AF87C7AF00CC83D401596D6976EA9F04FED7D024100EC3E9888C5486943265BF9D168C6318429122D8D1058ABD102C08EC396C4BC7CD85E57C4CBCC5224A18E8A36B2B57A9BE97142914A487A9A34CD793A82192037024100BDAC8B9636AB4D070A6E5AAB9E1A366C38EF78F0ECC398F7CB9193F74911E8E5061459DC6A8580E4219BD81726177C9BCB5B224C901F81ADC4F822F6CE63D5ED02407D121484A4ADDD5FC9038441AFF0749142EB8186EA894BE71F931E678C0DCD330922E32BF36C2B7CCE002B0DE623D77FB7D28CA74571AA51A37BD6C49F3A7A7702407ABAF0E8B9F65F048E4767D8755C415515C821145CF708A0568B5FBE2038C3DF7C6785708129354851FB6DB4A02D50A10B1CD9F54E3281161609F881949AF3D50241008ECE5287A13D616AA0C3CADCFB61FB62C2ACA1716ACE08D1A9123FC096D473A13CF665C6CC73ED6BE41CF4C4352D3E424638DF421292B1EB90CDD58C50DEDBA1";
    bool GetEncrptyFileName(base::FilePath* outpath)
    {
        base::FilePath path;
        PathService::Get(base::DIR_EXE, &path);

        base::FileEnumerator fileEnumerator(path, false,
            base::FileEnumerator::FILES, FILE_PATH_LITERAL("Authority*.auth"));

        base::FilePath filepath = fileEnumerator.Next();
        while (!filepath.value().empty())
        {
            std::wstring basename = filepath.BaseName().value();
            if (basename.find(L"Authority") == 0)
            {
                *outpath = filepath;
                return true;
            }
            filepath = fileEnumerator.Next();
        }
        return false;
    }
}

AuthorityHelper::AuthorityHelper()
{
}

AuthorityHelper::~AuthorityHelper()
{
}

bool AuthorityHelper::LoadAntiFloodAuthority(AntiFloodAuthority* authority)
{
    base::FilePath filepath;
    if (!GetEncrptyFileName(&filepath))
        return false;

    std::ifstream ovrifs;
    ovrifs.open(filepath.value());
    if (!ovrifs)
        return false;

    std::stringstream ss;
    ss << ovrifs.rdbuf();
    if (ss.str().empty())
        return false;

    std::string ciphertext = ss.str();
    std::stringbuf buf(std::string(privatekey).c_str());
    std::istream iss(&buf);
    std::string plaintext = RSADecryptString(&iss, ciphertext);
    Json::Reader reader;
    Json::Value root(Json::objectValue);
    try
    {
        if (!reader.parse(plaintext.c_str(), root))
        {
            assert(false && L"Json::Reader().parse error");
            return false;
        }

        if (!root.isObject())
        {
            assert(false && L"root is not object");
            return false;
        }
      
        authority->userid = GetInt32FromJsonValue(root, "userid");
        authority->roomid = GetInt32FromJsonValue(root, "roomid");
        authority->clanid = GetInt32FromJsonValue(root, "clanid");
        authority->kickout = GetInt32FromJsonValue(root, "kickout");
        authority->banchat = GetInt32FromJsonValue(root, "banchat");
        authority->antiadvance = GetInt32FromJsonValue(root, "antiadvance");
        std::string expire = root.get("expiretime", "0").asString();
        base::StringToUint64(expire, &authority->expiretime);
        authority->serverip = root.get("serverip", "").asString();
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool AuthorityHelper::GetAuthorityDisplayInfo(std::wstring* display)
{
    AntiFloodAuthority authority;
    if (!LoadAntiFloodAuthority(&authority))
        return false;

    std::wstring userid = base::UintToString16(authority.userid);
    std::wstring roomid = base::UintToString16(authority.roomid);
    base::Time expired = base::Time::FromInternalValue(authority.expiretime);
    std::string expiredstr = MakeFormatDateString(expired) + " " +
        MakeFormatTimeString(expired);
    std::wstring expiredwstr = base::UTF8ToWide(expiredstr);
    std::wstring clanid = base::UTF8ToWide(base::UintToString(authority.clanid));
    *display = L"授权信息:";
    if (authority.clanid)
    {
        *display += std::wstring(L" 公会ID: ") + clanid;
    }
    if (authority.userid)
    {
        *display += std::wstring(L" 繁星号: ") + userid;
    }
    if (authority.roomid)
    {
        *display += std::wstring(L" 房间号: ") + roomid;
    }

    *display += L" 到期时间: " + expiredwstr;
    return true;
}
