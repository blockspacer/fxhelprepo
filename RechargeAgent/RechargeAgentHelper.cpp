#include "stdafx.h"
#include "RechargeAgentHelper.h"
#include "Network/CurlWrapper.h"
#include "Network/CookiesHelper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_util.h"

RechargeAgentHelper::RechargeAgentHelper()
    :curlWrapper_(new CurlWrapper)
    , cookiesHelper_(new CookiesHelper)
{
}


RechargeAgentHelper::~RechargeAgentHelper()
{
}

bool RechargeAgentHelper::Initialize()
{
    CurlWrapper::CurlInit();
    return true;
}
void RechargeAgentHelper::Finalize()
{
    CurlWrapper::CurlCleanup();
}

bool RechargeAgentHelper::Login(const std::string& account,
                                const std::string& password,
                                const std::string& verifycode,
                                std::wstring* errorcode)
{
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_POST;
    request.url = "http://fanxing.kugou.com/fxAgent/fxAgentLogin.php";
    request.referer = "http://fanxing.kugou.com/fxAgent/fxAgentLogin.php";
    request.cookies = cookiesHelper_->GetCookies("_m_u_");
    if (request.cookies.empty())
    {
        *errorcode = L"无法获取验证码的cookies";
        return false;
    }
    
    request.queries["m"] = "public";
    request.queries["f"] = "login";
    std::string post = "name=" + UrlEncode(account) + "&password="+password+"&verify="+verifycode;
    request.postdata.assign(post.begin(), post.end());
    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    std::string data(response.content.begin(), response.content.end());
    if (data.find(base::WideToUTF8(L"登录成功"))==std::string::npos)
    {
        return false;
    }
    
    return true;
}

bool RechargeAgentHelper::GetVerifyCode(std::vector<uint8>* picture)
{
    HttpRequest request;
    request.url = "http://fanxing.kugou.com/fxAgent/fxAgentLoginCode.php";
    request.queries["m"] = "public";
    request.queries["f"] = "loginCode";
    request.queries["w"] = "90";
    request.queries["h"] = "45";
    request.queries["n"] = "5";
    request.referer = "http://fanxing.kugou.com/fxAgent/fxAgentLogin.php";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;

    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
    {
        return false;
    }

    // Set-Cookie: _m_u_=php_5763e796d4ea83.82300832; expires=Sun, 17-Jul-2016 12:05:42 GMT; Max-Age=2592000; path=/
    for (const auto& it : response.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    *picture = response.content;
    return true;
}

bool RechargeAgentHelper::SaveVerifyCodeImage(const std::vector<uint8>& image,
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
