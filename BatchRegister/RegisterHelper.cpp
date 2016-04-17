#include "stdafx.h"
#include "RegisterHelper.h"
#include "Network/Network.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

RegisterHelper::RegisterHelper()
    :curlWrapper_(new CurlWrapper)
{
    
}

RegisterHelper::~RegisterHelper()
{
}

bool RegisterHelper::Initialize()
{
    return NetworkInitialize();
}
void RegisterHelper::Finalize()
{
    NetworkFainalize();
}

bool RegisterHelper::SaveVerifyCodeImage(const std::vector<uint8>& image,
    std::wstring* path)
{
    if (image.empty() || !path)
    {
        return false;
    }

    uint64 time = base::Time::Now().ToInternalValue();
    std::string timestr = base::Uint64ToString(time);
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = base::UTF8ToWide(timestr + ".png");
    base::FilePath pathname = dirPath.Append(filename);
    base::File pngfile(pathname,
        base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    pngfile.Write(0, (char*)image.data(), image.size());
    pngfile.Close();
    *path = pathname.value();
    return true;
}

bool RegisterHelper::SaveAccountToFile(const std::wstring& username,
    const std::wstring& password)
{
    std::string utf8username = base::WideToUTF8(username);
    std::string utf8password = base::WideToUTF8(password);

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    base::FilePath pathname = dirPath.Append(L"accountinfo.txt");
    base::File accountfile(pathname,
        base::File::FLAG_OPEN_ALWAYS | base::File::FLAG_APPEND | base::File::FLAG_WRITE);
    accountfile.WriteAtCurrentPos((char*)utf8username.data(), utf8username.size());
    accountfile.WriteAtCurrentPos("\t", 1);
    accountfile.WriteAtCurrentPos((char*)utf8password.data(), utf8password.size());
    accountfile.WriteAtCurrentPos("\n", 1);
    accountfile.Close();
    return true;
}

bool RegisterHelper::LoadAccountFromFile(
    std::vector<std::pair<std::wstring, std::wstring>>* accountinfo)
{
    return false;
}


bool RegisterHelper::RegisterGetVerifyCode(std::vector<uint8>* picture)
{
    curlWrapper_->RegisterGetVerifyCode(picture);
    return false;
}

bool RegisterHelper::RegisterCheckUserExist(const std::wstring& username)
{
    return curlWrapper_->RegisterCheckUserExist(WideToUtf8(username));
}

bool RegisterHelper::RegisterCheckUserInfo(const std::wstring& username, const std::wstring& password)
{
    std::string utf8username = base::WideToUTF8(username);
    std::string utf8password = base::WideToUTF8(password);
    return curlWrapper_->RegisterCheckUserInfo(utf8username, utf8password);
}

bool RegisterHelper::RegisterCheckVerifyCode(const std::wstring& verifycode)
{
    std::string utf8verifycode = base::WideToUTF8(verifycode);
    return curlWrapper_->RegisterCheckVerifyCode(utf8verifycode);
}

bool RegisterHelper::RegisterUser(const std::wstring& username,
    const std::wstring& password, const std::wstring& verifycode)
{
    std::string utf8username = base::WideToUTF8(username);
    std::string utf8password = base::WideToUTF8(password);
    std::string utf8verifycode = base::WideToUTF8(verifycode);
    return curlWrapper_->RegisterUser(utf8username, utf8password, utf8verifycode);
}

