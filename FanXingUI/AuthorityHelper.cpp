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
    const char* privatekey = "30820274020100300D06092A864886F70D01010105000482025E3082025A02010002818100B8C0E6F601742DA36EF6CD701ABEACA193960089489C8C5A71C8F7E919C133954C163FA003CD2D5EA5B5B7D7B6702B14A065839F558B6862D2680E63838DC377F23CFD4CB133C384E6335983373260A1667298DCBDB9084890C5FFB9388C447F7D0239189139A0378AEA4CB211BBCF2C5E6574D870544086A0101F8F1CCF347D0201110281801304CC7B34DB04B2B4D5A434F3B1BEF283E60795A95B68CD133E19852C101FA9B8C60E143CA06A5145C3A5BF9E13137E5BCE36F60C921D91B3C6F26C205DA6F1D1C93807332293B38E3383A0E7BE3D6C9ED46EBE56825A864A73929942A233393D21E9FB1C9B6A96BE1143C86EE1C72F2C9980CB7FF9C7D5F90D77699D082361024100D2B6F88F9A1A0E62A5B46EF5B97DEC12B05D0B574F4DA8DBCBFE7620673C55BEC74EB1199FEDF8F4F69CB043A3E10E89D06843B6268AABEC0912392FC97B728D024100E0759B529412F5BFB68A7E2944C58D941CE159BA260B5DC1F1896735B7BA8F701AB33952FFF82E1C8281531C81DBC3FCDCBC4DFBB7BCDAC5239FA781F02945B10240252F590A485EF37AD1F2AA2B5CF81A99E2E33E3C95862CDB7E5A14D88AB04B5DE6EFC4E66784591C2B85100BEFBE4DDC15B80BF2F7BE1E56D46CA0AE147F3255024100C60D6AEE82A7514ECE3DF6D91E9031A0CE30401CB828258D024C0FC5FC776F81088005675A5373FB09BD677381A3CB0C4A4BCC5693100C5397E73972A6BB014102407467886A1E0710353A336D3B3D29236E026492126914745EAF3E2388ECFCE1832D6B54C268B71675CA5223EA726F01525B31D9416E94E814E308E159DAC61401";
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
