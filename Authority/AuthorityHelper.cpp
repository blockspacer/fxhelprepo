#include "stdafx.h"
#include <fstream>
#include "AuthorityHelper.h"
#include "Network/EncodeHelper.h"
#undef max
#undef min
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

AuthorityHelper::AuthorityHelper()
{
}


AuthorityHelper::~AuthorityHelper()
{
}

bool AuthorityHelper::Load(Authority* authority)
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    base::FilePath pathname = dirPath.Append(L"Authority_.auth");

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
    std::string expire = rootdata.get("expiretime", "0").asString();
    base::StringToUint64(expire, &authority->expiretime);

    return true;
}
bool AuthorityHelper::Save(const Authority& authority)
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

    std::string writestring = writer.write(root);

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring publickeyfilename = L"publickey.txt";
    base::FilePath pathname = dirPath.Append(publickeyfilename);
    std::ifstream ifs(pathname.value());
    if (ifs.bad())
        return false;

    std::string mymessage = R"({"userid":1,"roomid":2,"clanid":3,"kickout":1,"banchat":1,"antiadvance":1})";

    std::string ciphertext = RSAEncryptString(&ifs, writestring);

    std::wstring Authorityfilename = L"Authority_";
    //filename += base::UintToString16(authority.userid) + L"_";
    //filename += base::UintToString16(authority.roomid) + L"_";
    //filename += base::UintToString16(authority.clanid) + L"_";
    //filename += base::UintToString16(authority.kickout) + L"_";
    //filename += base::UintToString16(authority.banchat) + L"_";
    //filename += base::UintToString16(authority.antiadvance);
    Authorityfilename += L".auth";
    std::ofstream ofs(dirPath.Append(Authorityfilename).value(), std::ios_base::out);
    if (!ofs)
        return false;

    ofs << ciphertext;
    ofs.flush();
    ofs.close();
    return true;
}