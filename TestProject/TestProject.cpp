// TestProject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>
#include "Network/EncodeHelper.h"
#include "Network/User.h"
#include "Network/CurlWrapper.h"

#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_util.h"
#include "third_party/chromium/base/command_line.h"
#include "third_party/chromium/base/at_exit.h"

#include "third_party/cryptopp/cryptlib.h"
#include "third_party/cryptopp/rsa.h"
#include "third_party/cryptopp/aes.h"
#include "third_party/cryptopp/modes.h"
#include "third_party/cryptopp/filters.h"
#include "third_party/cryptopp/files.h"
#include "third_party/cryptopp/hex.h"
#include "third_party/cryptopp/randpool.h"

using namespace CryptoPP;

void SingleUserSingleRoomTest()
{
    bool result = true;
    User user;
    result &= user.Login("fanxingtest001", "123321");
    result &= user.EnterRoom(1084594);

    while (1);
}

void SingleUserMultiRoomTest(std::shared_ptr<User> user)
{
    bool result = true;
    result &= user->Login();
    result &= user->EnterRoom(1084594);
    result &= user->EnterRoom(1053564);
}

void MultiUserMultiRoomTest()
{
    std::vector<std::pair<std::string, std::string>> uservector;
    uservector.push_back(std::make_pair("fanxingtest001", "123321"));

    std::vector<std::shared_ptr<User>> userlist;
    for (const auto& it : uservector)
    {
        std::string username = it.first;
        std::string password = it.second;
        std::shared_ptr<User> shared_user(new User(username, password));
        userlist.push_back(shared_user);
        SingleUserMultiRoomTest(shared_user);
    }
    while (1);
}

void InitAppLog()
{
    CommandLine::Init(0, NULL);
    base::FilePath path;
    PathService::Get(base::DIR_APP_DATA, &path);
    path = path.Append(L"FanXingHelper").Append(L"fanxinghelper.log");
    logging::LoggingSettings setting;
    setting.logging_dest = logging::LOG_TO_ALL;
    setting.lock_log = logging::LOCK_LOG_FILE;
    setting.log_file = path.value().c_str();
    setting.delete_old = logging::APPEND_TO_OLD_LOG_FILE;
    logging::InitLogging(setting);
    logging::SetLogItems(false, true, true, true);
}

static CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption s_globalRNG;

RandomNumberGenerator & GlobalRNG()
{
    return s_globalRNG;
}

std::string MyRSAEncryptString(std::istream* pubFilename, const std::string& seed, const std::string& message)
{
    FileSource pubFile(*pubFilename, true, new HexDecoder);
    RSAES_OAEP_SHA_Encryptor pub(pubFile);

    RandomPool randPool;
    randPool.IncorporateEntropy((byte *)seed.c_str(), seed.length());

    std::string result;
    StringSource(message, true, new PK_EncryptorFilter(randPool, pub, new HexEncoder(new StringSink(result))));
    return result;
}

std::string MyRSADecryptString_(std::istream* privFilename, const std::string& ciphertext)
{
    using namespace CryptoPP;
    FileSource privFile(*privFilename, true, new HexDecoder);
    RSAES_OAEP_SHA_Decryptor priv(privFile);

    std::string result;
    StringSource(ciphertext.c_str(), true, new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(result))));
    return result;
}

std::string MyRSADecryptString(std::istream* privFilename, const std::string& ciphertext)
{
    std::string decrypted;
    std::string part = ciphertext;
    auto pos = 0;
    uint32 maxcipher = 256;
    while (part.length() > maxcipher)
    {
        std::string temp = ciphertext.substr(pos, maxcipher);
        decrypted += MyRSADecryptString_(privFilename, temp);
        pos += maxcipher;
        part = ciphertext.substr(pos);
    }

    if (!part.empty())
    {
        decrypted += MyRSADecryptString_(privFilename, part.c_str());
    }

    return decrypted;
}

void MyGenerateRSAKey(unsigned int keyLength, std::ostream* privFilename, std::ostream* pubFilename, const std::string& seed)
{
    RandomPool randPool;
    randPool.IncorporateEntropy((byte *)seed.c_str(), seed.length());

    RSAES_OAEP_SHA_Decryptor priv(randPool, keyLength);
    HexEncoder privFile(new FileSink(*privFilename));
    priv.DEREncode(privFile);
    privFile.MessageEnd();

    RSAES_OAEP_SHA_Encryptor pub(priv);
    HexEncoder pubFile(new FileSink(*pubFilename));
    pub.DEREncode(pubFile);
    pubFile.MessageEnd();
}

bool RSAOut()
{
    std::string seed = CryptoPP::IntToString(time(NULL));
    seed.resize(16);
    s_globalRNG.SetKeyWithIV((byte *)seed.data(), 16, (byte *)seed.data());

    std::ifstream ovrifs;
    ovrifs.open("f:/Hack/FXHelper/Debug/Authority.txt");
    if (!ovrifs)
        return false;

    std::stringstream ss;
    ss << ovrifs.rdbuf();
    if (ss.str().empty())
        return false;
    const char* privatekey = "30820274020100300D06092A864886F70D01010105000482025E3082025A02010002818100B8C0E6F601742DA36EF6CD701ABEACA193960089489C8C5A71C8F7E919C133954C163FA003CD2D5EA5B5B7D7B6702B14A065839F558B6862D2680E63838DC377F23CFD4CB133C384E6335983373260A1667298DCBDB9084890C5FFB9388C447F7D0239189139A0378AEA4CB211BBCF2C5E6574D870544086A0101F8F1CCF347D0201110281801304CC7B34DB04B2B4D5A434F3B1BEF283E60795A95B68CD133E19852C101FA9B8C60E143CA06A5145C3A5BF9E13137E5BCE36F60C921D91B3C6F26C205DA6F1D1C93807332293B38E3383A0E7BE3D6C9ED46EBE56825A864A73929942A233393D21E9FB1C9B6A96BE1143C86EE1C72F2C9980CB7FF9C7D5F90D77699D082361024100D2B6F88F9A1A0E62A5B46EF5B97DEC12B05D0B574F4DA8DBCBFE7620673C55BEC74EB1199FEDF8F4F69CB043A3E10E89D06843B6268AABEC0912392FC97B728D024100E0759B529412F5BFB68A7E2944C58D941CE159BA260B5DC1F1896735B7BA8F701AB33952FFF82E1C8281531C81DBC3FCDCBC4DFBB7BCDAC5239FA781F02945B10240252F590A485EF37AD1F2AA2B5CF81A99E2E33E3C95862CDB7E5A14D88AB04B5DE6EFC4E66784591C2B85100BEFBE4DDC15B80BF2F7BE1E56D46CA0AE147F3255024100C60D6AEE82A7514ECE3DF6D91E9031A0CE30401CB828258D024C0FC5FC776F81088005675A5373FB09BD677381A3CB0C4A4BCC5693100C5397E73972A6BB014102407467886A1E0710353A336D3B3D29236E026492126914745EAF3E2388ECFCE1832D6B54C268B71675CA5223EA726F01525B31D9416E94E814E308E159DAC61401";

    std::string ciphertext = ss.str();
    std::stringbuf buf(std::string(privatekey).c_str());
    std::ifstream issprivatekey;
    issprivatekey.open("f:/Hack/FXHelper/Debug/privatekey.txt");
    if (issprivatekey.bad())
    {
        return false;
    }
    
    std::string plaintext = MyRSADecryptString(&issprivatekey, ciphertext);
    return true;
}

void RSATest()
{   
    std::string seed = CryptoPP::IntToString(time(NULL));
    seed.resize(16);
    s_globalRNG.SetKeyWithIV((byte *)seed.data(), 16, (byte *)seed.data());
    std::stringbuf publickey;
    std::stringbuf privatekey;
    std::ostream oss_publickey(&publickey);
    std::ostream oss_privatekey(&privatekey);
    MyGenerateRSAKey(1024, &oss_privatekey, &oss_publickey, seed);

    std::istream iss_publickey(&publickey);
    std::istream iss_privatekey(&privatekey);

    //std::string publicfile = "e:/public.txt";
    //std::string privatefile = "e:/private.txt";
    //GenerateRSAKey(512, privatefile, publicfile, seed);


    std::string message = "123456";
    std::string mymessage = R"({"userid":1,"roomid":2,"clanid":3,"kickout":1,"banchat":1,"antiadvance":1})";
    std::string ciphertext = MyRSAEncryptString(&iss_publickey, seed, mymessage);
    std::string decrypted = MyRSADecryptString(&iss_privatekey, ciphertext);

    std::ofstream privatekeyofs("d:/privatekey.txt");
    privatekeyofs << privatekey.str();
    privatekeyofs.close();

    std::ofstream publickeyofs("d:/publickey.txt");
    publickeyofs << publickey.str();
    publickeyofs.close();

    std::ofstream ciphertextofs("d:/Authority.txt");
    ciphertextofs << ciphertext.c_str();
    ciphertextofs.close();
}

int _tmain(int argc, _TCHAR* argv[])
{
    base::AtExitManager atExitManager;
    RSAOut();
    //RSATest();
    //CurlWrapper::CurlInit();
    ////SingleUserSingleRoomTest();
    //MultiUserMultiRoomTest();

    //CurlWrapper::CurlCleanup();
	return 0;
}

