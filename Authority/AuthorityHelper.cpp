#include "stdafx.h"
#include <fstream>
#include "AuthorityHelper.h"
#include "Network/EncodeHelper.h"
#undef max
#undef min
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_enumerator.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

namespace
{
    const wchar_t* antiflood_pre = L"Authority_";
    const wchar_t* tracker_pre = L"Tracker_";

    bool GetEncrptyFileName(const std::wstring& pre, base::FilePath* outpath)
    {
        base::FilePath path;
        PathService::Get(base::DIR_EXE, &path);

        std::wstring key = pre + L"*";
        base::FileEnumerator fileEnumerator(path, false,
            base::FileEnumerator::FILES, key.c_str());

        base::FilePath filepath = fileEnumerator.Next();
        while (!filepath.value().empty())
        {
            std::wstring basename = filepath.BaseName().value();
            if (basename.find(pre) == 0)
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
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    base::FilePath pathname;
    if (!GetEncrptyFileName(antiflood_pre, &pathname))
        return false;

    std::ifstream cipherifs;
    cipherifs.open(pathname.value());
    if (!cipherifs)
        return false;

    std::stringstream ss;
    ss << cipherifs.rdbuf();
    if (ss.str().empty())
        return false;

    pathname = dirPath.Append(L"privatekey.txt");
    std::ifstream privatekeyifs;
    privatekeyifs.open(pathname.value());
    if (!privatekeyifs)
        return false;

    std::string plaintext = RSADecryptString(&privatekeyifs, ss.str());
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(plaintext, rootdata, false))
        return false;

    if (!rootdata.isObject())
        return false;

    authority->userid = GetInt32FromJsonValue(rootdata, "userid");
    authority->roomid = GetInt32FromJsonValue(rootdata, "roomid");
    authority->clanid = GetInt32FromJsonValue(rootdata, "clanid");
    authority->kickout = GetInt32FromJsonValue(rootdata, "kickout");
    authority->banchat = GetInt32FromJsonValue(rootdata, "banchat");
    authority->antiadvance = GetInt32FromJsonValue(rootdata, "antiadvance");
    authority->serverip = rootdata.get("serverip", "").asString();
    std::string expire = rootdata.get("expiretime", "0").asString();
    base::StringToUint64(expire, &authority->expiretime);

    return true;
}
bool AuthorityHelper::SaveAntiFloodAuthority(const AntiFloodAuthority& authority)
{
    Json::FastWriter writer;
    Json::Value root(Json::objectValue);
    root["userid"] = authority.userid;
    root["roomid"] = authority.roomid;
    root["clanid"] = authority.clanid;
    root["kickout"] = authority.kickout;
    root["banchat"] = authority.banchat;
    root["antiadvance"] = authority.antiadvance;
    root["expiretime"] = base::Uint64ToString(authority.expiretime);
    root["serverip"] = authority.serverip;
    root["trackerhost"] = "visitor.fanxing.kugou.com"; // 临时工具使用

    std::string writestring = writer.write(root);

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring publickeyfilename = L"publickey.txt";
    base::FilePath pathname = dirPath.Append(publickeyfilename);
    std::ifstream ifs(pathname.value());
    if (ifs.bad())
        return false;

    std::string ciphertext = RSAEncryptString(&ifs, writestring);

    std::wstring Authorityfilename = antiflood_pre;
    Authorityfilename += base::UintToString16(authority.userid) + L"_";
    Authorityfilename += base::UintToString16(authority.roomid) + L"_";
    Authorityfilename += base::UintToString16(authority.clanid);
    Authorityfilename += L".auth";
    std::ofstream ofs(dirPath.Append(Authorityfilename).value(), std::ios_base::out);
    if (!ofs)
        return false;

    ofs << ciphertext;
    ofs.flush();
    ofs.close();
    return true;
}

bool AuthorityHelper::LoadUserTrackerAuthority(UserTrackerAuthority* authority)
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    base::FilePath pathname;
    if (!GetEncrptyFileName(tracker_pre, &pathname))
        return false;

    std::ifstream cipherifs;
    cipherifs.open(pathname.value());
    if (!cipherifs)
        return false;

    std::stringstream ss;
    ss << cipherifs.rdbuf();
    if (ss.str().empty())
        return false;

    pathname = dirPath.Append(L"privatekey.txt");
    std::ifstream privatekeyifs;
    privatekeyifs.open(pathname.value());
    if (!privatekeyifs)
        return false;

    std::string plaintext = RSADecryptString(&privatekeyifs, ss.str());
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(plaintext, rootdata, false))
        return false;

    if (!rootdata.isObject())
        return false;

    authority->user_id = GetInt32FromJsonValue(rootdata, "userid");
    authority->tracker_host = rootdata.get("trackerhost", "").asString();
    std::string expire = rootdata.get("expiretime", "0").asString();
    base::StringToUint64(expire, &authority->expiretime);

    return true;
}

bool AuthorityHelper::SaveUserTrackerAuthority(const UserTrackerAuthority& authority)
{
    Json::FastWriter writer;
    Json::Value root(Json::objectValue);
    root["userid"] = authority.user_id;
    root["expiretime"] = base::Uint64ToString(authority.expiretime);
    root["trackerhost"] = authority.tracker_host;

    std::string writestring = writer.write(root);

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring publickeyfilename = L"publickey.txt";
    base::FilePath pathname = dirPath.Append(publickeyfilename);
    std::ifstream ifs(pathname.value());
    if (ifs.bad())
        return false;

    std::string ciphertext = RSAEncryptString(&ifs, writestring);

    std::wstring Authorityfilename = L"Tracker_";
    Authorityfilename += base::UintToString16(authority.user_id);

    Authorityfilename += L".key";
    std::ofstream ofs(dirPath.Append(Authorityfilename).value(), std::ios_base::out);
    if (!ofs)
        return false;

    ofs << ciphertext;
    ofs.flush();
    ofs.close();
    return true;
}