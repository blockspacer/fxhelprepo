// TestProject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>
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

std::string RSAEncryptString(std::istream* pubFilename, const std::string& seed, const std::string& message)
{
    FileSource pubFile(*pubFilename, true, new HexDecoder);
    RSAES_OAEP_SHA_Encryptor pub(pubFile);

    RandomPool randPool;
    randPool.IncorporateEntropy((byte *)seed.c_str(), seed.length());

    std::string result;
    StringSource(message, true, new PK_EncryptorFilter(randPool, pub, new HexEncoder(new StringSink(result))));
    return result;
}

std::string RSADecryptString(std::istream* privFilename, const std::string& ciphertext)
{
    FileSource privFile(*privFilename, true, new HexDecoder);
    RSAES_OAEP_SHA_Decryptor priv(privFile);

    std::string result;
    StringSource(ciphertext, true, new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(result))));
    return result;
}

void GenerateRSAKey(unsigned int keyLength, std::ostream* privFilename, std::ostream* pubFilename, const std::string& seed)
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

void RSATest()
{   
    std::string seed = CryptoPP::IntToString(time(NULL));
    seed.resize(16);
    s_globalRNG.SetKeyWithIV((byte *)seed.data(), 16, (byte *)seed.data());
    std::stringbuf publickey;
    std::stringbuf privatekey;
    std::ostream oss_publickey(&publickey);
    std::ostream oss_privatekey(&privatekey);
    GenerateRSAKey(512, &oss_privatekey, &oss_publickey, seed);

    std::istream iss_publickey(&publickey);
    std::istream iss_privatekey(&privatekey);

    //std::string publicfile = "e:/public.txt";
    //std::string privatefile = "e:/private.txt";
    //GenerateRSAKey(512, privatefile, publicfile, seed);


    std::string message = "123456";
   
    std::string ciphertext = RSAEncryptString(&iss_publickey, seed, message);
    std::string decrypted = RSADecryptString(&iss_privatekey, ciphertext.c_str());
}

int _tmain(int argc, _TCHAR* argv[])
{
    base::AtExitManager atExitManager;
    RSATest();
    //CurlWrapper::CurlInit();
    ////SingleUserSingleRoomTest();
    //MultiUserMultiRoomTest();

    //CurlWrapper::CurlCleanup();
	return 0;
}

