#include "stdafx.h"
#include <fstream>
#include <sstream>

#include "AuthorityHelper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_enumerator.h"

#include "third_party/json/json.h"

namespace
{
    const char* privatekey = "30820153020100300D06092A864886F70D01010105000482013D30820139020100024100D1EA0092572B1E16BFE101422D1195C3DE889B77CB12C7FEED64C98A4AB61BA24DA648264C79301D1B39557FB9440CCE10DF8D5DB0B5AF394F056EBFD822EAAB02011102402B37B4D2D5B60DB963BD622436748FC67194987A8BB10B0ED685B105E2348D37A1795C6835699E4EEDE0CC2175A4B946D372ACD19E8AF3FA268081CAC746B809022100E8B6B0BFC4DAC088EDAC1AC0DCD2B850373073A0BE8286F94AF95014F270AB73022100E6EB4447CD544A14CEB7121C57E3876EFB38F6E75DD962A5922E3A88D4A3E5E9022100BFA5827FCF4ABCACFFF725174C714C7E4B91502A064D601898189C4D7C5CC96D02203655797A4E6E2F8C6CDFC806AB449856593A9472ACAB9EBD8BCEA45C6E44AE9102210088298E6DDC17F4B5EC749A96AB5BA80F0CBC2E5D18B481C362E46FA1EB210A09";
    bool GetEncrptyFileName(base::FilePath* outpath)
    {
        base::FilePath path;
        PathService::Get(base::DIR_EXE, &path);

        base::FileEnumerator fileEnumerator(path, false,
            base::FileEnumerator::FILES, FILE_PATH_LITERAL("Authority*.txt"));

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

bool AuthorityHelper::Load(Authority* authority)
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
    }
    catch (...)
    {
        return false;
    }
    return true;
}
