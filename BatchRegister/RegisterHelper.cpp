#include "stdafx.h"
#include "RegisterHelper.h"
#include "Network/Network.h"
#include "Network/CurlWrapper.h"
#include "Network/CookiesHelper.h"
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
    , cookiesHelper_(new CookiesHelper)
    , accountFile_(new base::File)
{

}

RegisterHelper::~RegisterHelper()
{
    accountFile_->Close();
}

bool RegisterHelper::Initialize()
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"AccountInfo" + 
        base::UTF8ToWide(MakeFormatDateString(base::Time::Now())) + L".txt";
    base::FilePath pathname = dirPath.Append(filename);
    accountFile_->Initialize(pathname,
        base::File::FLAG_OPEN_ALWAYS | base::File::FLAG_APPEND);
    
    if (!accountFile_->IsValid())
    {
        return false;
    }
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
    accountFile_->WriteAtCurrentPos((char*)utf8username.data(), utf8username.size());
    accountFile_->WriteAtCurrentPos("\t", 1);
    accountFile_->WriteAtCurrentPos((char*)utf8password.data(), utf8password.size());
    accountFile_->WriteAtCurrentPos("\n", 1);
    return true;
}

bool RegisterHelper::LoadAccountFromFile(
    std::vector<std::pair<std::wstring, std::wstring>>* accountinfo)
{
    return false;
}

std::wstring RegisterHelper::GetNewName() const
{
    return std::wstring(L"nowreg")+ base::UTF8ToWide(GetNowTimeString());
}

std::wstring RegisterHelper::GetPassword() const
{
    return L"supperpwd123";
}


//GET /reg/web/verifycode/t=1460916352057 HTTP/1.1
//Host: www.kugou.com
//Connection: keep-alive
//Accept: image/webp,*/*;q=0.8
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Referer: http://www.kugou.com/reg/web/
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8

bool RegisterHelper::RegisterGetVerifyCode(std::vector<uint8>* picture)
{
    std::string url = "http://www.kugou.com/reg/web/verifycode/t=" + GetNowTimeString();
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://www.kugou.com/reg/web/";

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    //Set-Cookie: CheckCode=czozMjoiMTFmNzY4MDIyODhlODEyZmY0MjQxZjMzODI2MDJmYTQiOw%3D%3D; path=/
    for (const auto& it : response.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    *picture = response.content;
    return true;
}

//GET /reg/web/checkusername/?userName=fanxingtest011&t=1460916361431 HTTP/1.1
//Host: www.kugou.com
//Connection: keep-alive
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Accept: */*
//Referer: http://www.kugou.com/reg/web/
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8
//Cookie: CheckCode=czozMjoiMTFmNzY4MDIyODhlODEyZmY0MjQxZjMzODI2MDJmYTQiOw%3D%3D
bool RegisterHelper::RegisterCheckUserExist(const std::wstring& username)
{
    std::string url = "http://www.kugou.com/reg/web/checkusername/";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://www.kugou.com/reg/web/";
    request.queries["userName"] = UrlEncode(base::WideToUTF8(username));
    request.queries["t"] = GetNowTimeString();
    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string content;
    content.assign(response.content.begin(), response.content.end());
    if (content.find(R"({"status":1})") == std::string::npos)
    {
        return false;
    }
    return true;
}

//GET /check_str?str=1233211234567&appid=1014&callback=checkPwWithPort HTTP/1.1
//Host: userinfo.user.kugou.com
//Connection: keep-alive
//Accept: */*
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Referer: http://www.kugou.com/reg/web/
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8
bool RegisterHelper::RegisterCheckPassword(const std::wstring& username, const std::wstring& password)
{
    std::string url = "http://userinfo.user.kugou.com/check_str";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://www.kugou.com/reg/web/";
    request.queries["str"] = UrlEncode(base::WideToUTF8(password));
    request.queries["appid"] = "1014";
    request.queries["callback"] = "checkPwWithPort";
    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string content;
    content.assign(response.content.begin(), response.content.end());
    if (content.find(R"("status":1)") == std::string::npos)
    {
        return false;
    }
    return true;
}

//POST /reg/web/regbyusername HTTP/1.1
//Host: www.kugou.com
//Connection: keep-alive
//Content-Length: 88
//Origin: http://www.kugou.com
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Content-Type: application/x-www-form-urlencoded
//Accept: */*
//Referer: http://www.kugou.com/reg/web/
//Accept-Encoding: gzip, deflate
//Accept-Language: zh-CN,zh;q=0.8
//Cookie: CheckCode=czozMjoiMTFmNzY4MDIyODhlODEyZmY0MjQxZjMzODI2MDJmYTQiOw%3D%3D
// PostData = userName=fanxingtest011&pwd=1233211234567&rePwd=1233211234567&verifyCode=upfill&UM_Sex=1
bool RegisterHelper::RegisterUser(const std::wstring& username,
    const std::wstring& password, const std::wstring& verifycode)
{

    std::string postdata = "userName=" + UrlEncode(base::WideToUTF8(username))
        + "&pwd=" + base::WideToUTF8(password) + "&rePwd="
        + base::WideToUTF8(password) + "&verifyCode="
        + base::WideToUTF8(verifycode) + "&UM_Sex=1";
    std::string url = "http://www.kugou.com/reg/web/regbyusername";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_POST;
    request.url = url;
    request.referer = "http://www.kugou.com/reg/web/";
    request.headers["Origin"] = "http://www.kugou.com";
    request.cookies = cookiesHelper_->GetCookies("CheckCode");
    request.postdata.assign(postdata.begin(), postdata.end());
    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string content;
    content.assign(response.content.begin(), response.content.end());
    if (content.find(R"("status":1)") == std::string::npos)
    {
        return false;
    }

    // Set-Cookie: KuGoo=KugooID=801240286&KugooPwd=563A0A9D74800D644BD72A561180B675&NickName=%u0066%u0061%u006e%u0078%u0069%u006e%u0067%u0074%u0065%u0073%u0074%u0030%u0031%u0031&Pic=&RegState=1&RegFrom=&t=31d9b29c7975f07a2e79c47661fa2dfd19c3387274d087cc0b4309560bf27662&a_id=1014&ct=1460916377&UserName=%u0066%u0061%u006e%u0078%u0069%u006e%u0067%u0074%u0065%u0073%u0074%u0030%u0031%u0031; expires=Mon, 18-Apr-2016 18:06:17 GMT; path=/; domain=.kugou.com
    for (const auto& it : response.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    return true;
}

