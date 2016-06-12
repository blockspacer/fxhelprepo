#include "stdafx.h"
#include <fstream>
#include "RegisterHelper.h"
#include "Network/Network.h"
#include "Network/CurlWrapper.h"
#include "Network/CookiesHelper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_util.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

namespace
{
    static int index = 0;
}
RegisterHelper::RegisterHelper()
    :curlWrapper_(new CurlWrapper)
    , cookiesHelper_(new CookiesHelper)
    , accountFile_(new base::File)
{
    namepost = { "qqcom", "163com", "sinacom",
        "hotmailcom", "yahoocom", "gmailcom", "tomcom", "126com" };
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
    
    base::FilePath pathname = dirPath.Append(L"VerifyCode");
    if (!base::DirectoryExists(pathname))
    {
        base::CreateDirectory(pathname);
    }
    std::wstring filename = base::UTF8ToWide(timestr + ".png");
    base::FilePath filepath = pathname.Append(filename);
    base::File pngfile(filepath,
        base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    pngfile.Write(0, (char*)image.data(), image.size());
    pngfile.Close();
    *path = filepath.value();
    return true;
}

bool RegisterHelper::SaveAccountToFile(const std::wstring& username,
    const std::wstring& password, const std::string& cookies)
{
    std::string utf8username = base::WideToUTF8(username);
    std::string utf8password = base::WideToUTF8(password);
    accountFile_->WriteAtCurrentPos((char*)utf8username.data(), utf8username.size());
    accountFile_->WriteAtCurrentPos("\t", 1);
    accountFile_->WriteAtCurrentPos((char*)utf8password.data(), utf8password.size());
    accountFile_->WriteAtCurrentPos("\t", 1);
    accountFile_->WriteAtCurrentPos((char*)cookies.data(), cookies.size());
    accountFile_->WriteAtCurrentPos("\n", 1);
    return true;
}

bool RegisterHelper::LoadAccountFromFile(
    std::vector<std::pair<std::wstring, std::wstring>>* accountinfo)
{
    return false;
}

bool RegisterHelper::LoadIpProxy(std::vector<IpProxy>* ipproxys)
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"Proxy.txt";
    base::FilePath pathname = dirPath.Append(filename);

    std::ifstream ovrifs;
    ovrifs.open(pathname.value());
    if (!ovrifs)
        return false;

    std::stringstream ss;
    ss << ovrifs.rdbuf();
    if (ss.str().empty())
        return false;

    std::string data = ss.str();
    try
    {
        Json::Reader reader;
        Json::Value root(Json::objectValue);
        if (!Json::Reader().parse(data.c_str(), root))
        {
            assert(false && L"Json::Reader().parse error");
            return false;
        }

        if (!root.isArray())
        {
            assert(false && L"root is not array");
            return false;
        }

        for (const auto& value : root)//for data
        {
            Json::Value temp;
            uint32 proxytype = GetInt32FromJsonValue(value, "proxytype");
            uint32 proxyport = GetInt32FromJsonValue(value, "port");
            std::string proxyip = value.get("ip", "").asString();
            IpProxy proxy;
            proxy.SetProxyType(static_cast<IpProxy::PROXY_TYPE>(proxytype));
            proxy.SetProxyIp(proxyip);
            proxy.SetProxyPort(proxyport);
            ipproxys->push_back(proxy);
        }
    }
    catch (...)
    {
        assert(false && L"读取错误");
        return false;
    }
    return true;
}

bool RegisterHelper::SaveIpProxy(const std::vector<IpProxy>& ipproxys)
{
    Json::FastWriter writer;
    Json::Value root(Json::arrayValue);

    for (const auto& proxy : ipproxys)
    {
        Json::Value jproxy(Json::objectValue);
        jproxy["proxytype"] = static_cast<uint32>(proxy.GetProxyType());
        jproxy["ip"] = proxy.GetProxyIp();
        jproxy["port"] = proxy.GetProxyPort();
        root.append(jproxy);
    }

    std::string writestring = writer.write(root);

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"Proxy.txt";
    base::FilePath pathname = dirPath.Append(filename);
    std::ofstream ofs(pathname.value(), std::ios_base::out);
    if (!ofs)
        return false;

    ofs << writestring;
    ofs.flush();
    ofs.close();
    return true;
}

std::wstring RegisterHelper::GetNewName() const
{
    std::string timestring = GetNowTimeString().substr(4);
    uint32 time32 = 0;
    base::StringToUint(timestring, &time32);
    std::string post = namepost[time32%namepost.size()];
    std::string pre;
    pre.push_back('A' + (time32 % 25));
    return base::UTF8ToWide(pre + timestring + post).substr(0,19);
}

std::wstring RegisterHelper::GetPassword() const
{
    std::string timestring = GetNowTimeString();
    return base::UTF8ToWide(timestring);
}


//GET /reg/web/verifycode/t=1460916352057 HTTP/1.1
//Host: www.kugou.com
//Connection: keep-alive
//Accept: image/webp,*/*;q=0.8
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Referer: http://www.kugou.com/reg/web/
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8

bool RegisterHelper::RegisterGetVerifyCode(
    const IpProxy& ipproxy, std::vector<uint8>* picture)
{
    // 之前使用过的数字验证码
    //std::string url = "http://www.kugou.com/reg/web/verifycode/t=" + GetNowTimeString();
    std::string url = "http://verifycode.service.kugou.com/v1/get_img_code";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://www.kugou.com/reg/web/";
    request.queries["type"] = "RegCheckCode";
    request.queries["appid"] = "1014";
    request.queries["codetype"] = "1";
    request.queries["t"] = GetNowTimeString();
    if (ipproxy.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        request.ipproxy = ipproxy;
    }
    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    // Set-Cookie: CheckCode=czozMjoiMTFmNzY4MDIyODhlODEyZmY0MjQxZjMzODI2MDJmYTQiOw%3D%3D; path=/
    // Set-Cookie: RegCheckCode=2f72ef708dabebb11790a7831bcc1cd8
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
bool RegisterHelper::RegisterCheckUserExist(
    const IpProxy& ipproxy, const std::wstring& username)
{
    std::string url = "http://www.kugou.com/reg/web/checkusername/";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://www.kugou.com/reg/web/";
    request.queries["userName"] = UrlEncode(base::WideToUTF8(username));
    request.queries["t"] = GetNowTimeString();
    if (ipproxy.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        request.ipproxy = ipproxy;
    }
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
bool RegisterHelper::RegisterCheckPassword(const IpProxy& ipproxy, 
    const std::wstring& username, const std::wstring& password)
{
    std::string url = "http://userinfo.user.kugou.com/check_str";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://www.kugou.com/reg/web/";
    request.queries["str"] = UrlEncode(base::WideToUTF8(password));
    request.queries["appid"] = "1014";
    request.queries["callback"] = "checkPwWithPort";
    if (ipproxy.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        request.ipproxy = ipproxy;
    }
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
bool RegisterHelper::RegisterUser(
    const IpProxy& ipproxy, const std::wstring& username,
    const std::wstring& password, const std::string& verifycode,
    std::string* cookies, std::wstring* errormsg)
{
    std::string url = "https://reg-user.kugou.com/v2/reg/";
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = "http://www.kugou.com/reg/web/";
    request.cookies = cookiesHelper_->GetCookies("RegCheckCode");
    request.queries["regtype"] = "username";
    request.queries["appid"] = "1014";
    request.queries["code"] = verifycode;
    request.queries["expire_day"] = "1";
    request.queries["username"] = UrlEncode(base::WideToUTF8(username));
    request.queries["sex"] = "1";
    request.queries["password"] = MakeMd5FromString(base::WideToUTF8(password));
    request.queries["nickname"] = base::WideToUTF8(username);
    request.queries["security_email"] = "";
    request.queries["id_card"] = "";
    request.queries["truename"] = "";
    request.queries["callback"] = "RegByUserNameCallbackFn";
    request.queries["codetype"] = "1";
    if (ipproxy.GetProxyType()!=IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        request.ipproxy = ipproxy;
    }
    
    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string content;
    content.assign(response.content.begin(), response.content.end());
    content = PickJson(content);
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(content, rootdata, false))
    {
        return false;
    }

    // 暂时没有必要检测status的值
    std::string utf8ErrorMsg = rootdata.get("errorMsg", "").asString();
    std::wstring wErrorMsg = base::UTF8ToWide(utf8ErrorMsg);
    if (!wErrorMsg.empty())
    {
        LOG(ERROR) << wErrorMsg;
        *errormsg = wErrorMsg;
        return false;
    }
    if (content.find(R"("nickname")") == std::string::npos)
    {
        return false;
    }

    // Set-Cookie: KuGoo=KugooID=801240286&KugooPwd=563A0A9D74800D644BD72A561180B675&NickName=%u0066%u0061%u006e%u0078%u0069%u006e%u0067%u0074%u0065%u0073%u0074%u0030%u0031%u0031&Pic=&RegState=1&RegFrom=&t=31d9b29c7975f07a2e79c47661fa2dfd19c3387274d087cc0b4309560bf27662&a_id=1014&ct=1460916377&UserName=%u0066%u0061%u006e%u0078%u0069%u006e%u0067%u0074%u0065%u0073%u0074%u0030%u0031%u0031; expires=Mon, 18-Apr-2016 18:06:17 GMT; path=/; domain=.kugou.com
    for (const auto& it : response.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    *cookies = cookiesHelper_->GetCookies("KuGoo");

    if (cookies->empty())
        return false;
        
    return true;
}

