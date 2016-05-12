#include "stdafx.h"
#include <utility>
#include "HuyaLogic.h"
#include "Network/EncodeHelper.h"
#include "Network/CurlWrapper.h"
#include "Network/CookiesHelper.h"
#include "third_party/json/json.h"

#include "third_party/chromium/base/strings/utf_string_conversions.h"

namespace
{
    std::string RemoveChunk(const std::string& src, const std::string& chunk)
    {
        std::string result;
        int len = chunk.length();
        auto begin = 0;
        auto pos = src.find(chunk,begin);
        while (pos!=std::string::npos)
        {
            result += src.substr(begin, pos - begin);
            begin = pos + len;
            pos = src.find(chunk, begin);
        }
        result += src.substr(begin);
        return result;
    }
}
HuyaLogic::HuyaLogic()
    :cookiesHelper_(new CookiesHelper)
{
}


HuyaLogic::~HuyaLogic()
{
}

bool HuyaLogic::HuyaLogin(const std::wstring& account, const std::wstring& password,
                          std::wstring* errormsg)
{
    std::string utf8username = UrlEncode(base::WideToUTF8(account));
    std::string utf8passowrd = base::WideToUTF8(password);
    CurlWrapper curlWrapper;
    std::string url;
    std::string ttoken;
    if (!HuyaLoginStep1(&curlWrapper, &url, &ttoken))
        return false;
    
    url += "&regCallbackURL=http://www.huya.com/udb_web/udbport2.php?do=callback&UIStyle=xelogin&rdm=0.14782617054879665";
    if (!HuyaLoginStep2(&curlWrapper, url, ttoken))
        return false;
    
    if (!HuyaLoginStep3(&curlWrapper, url, ttoken, utf8username, utf8passowrd, errormsg))
        return false;
    
    return true;
}

bool HuyaLogic::HuyaLoginStep1(CurlWrapper* curlWrapper, std::string* url, 
                               std::string* ttoken) const
{
    HttpRequest httpRequest;
    httpRequest.url = "http://www.huya.com/udb_web/udbport2.php";
    httpRequest.queries["do"] = "authorizeEmbedURL";
    httpRequest.headers["Origin"] = "http://www.huya.com";
    httpRequest.referer = "http://www.huya.com/";
    httpRequest.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_POST;
    std::string data = "callbackURL=http%3A%2F%2Fwww.huya.com%2Fudb_web%2Fudbport2.php%3Fdo%3Dcallback";
    httpRequest.postdata.assign(data.begin(), data.end());
    httpRequest.cookies = "hiido_ui=0.584738260885661";
    httpRequest.cookies += std::string(";") + "__yamid_tt1=0.3749734029113536";
    httpRequest.cookies += std::string(";") + "__yamid_new=C71075CFD3C0000158DA17F9DB401BF8";
    httpRequest.cookies += std::string(";") + "__FEQUALITY__UUID=fcdc8e9f-116a-4814-91d8-3280a184ad98";
    httpRequest.cookies += std::string(";") + "h_unt=1463035242";
    httpRequest.cookies += std::string(";") + "__yasmid=0.3749734029113536";
    httpRequest.cookies += std::string(";") + "_yasids=__rootsid=C710C7C342C00001DBE25C3AD5F019AB";
    httpRequest.cookies += std::string(";") + "Hm_lvt_51700b6c722f5bb4cf39906a596ea41f=1462951174,1463035219,1463037105";
    httpRequest.cookies += std::string(";") + "Hm_lpvt_51700b6c722f5bb4cf39906a596ea41f=1463037105";
    httpRequest.cookies += std::string(";") + "PHPSESSID=8ss3f0p4i2sl3r957hbbdqiuh1";
    HttpResponse httpResponse;
    if (!curlWrapper->Execute(httpRequest, &httpResponse))
        return false;

    for (const auto& it : httpResponse.cookies)
    {
        assert(false && L"这里应该是不会设置cookie的");
        cookiesHelper_->SetCookies(it);
    }

    Json::Reader reader;
    Json::Value root(Json::objectValue);
    std::string content(httpResponse.content.begin(), httpResponse.content.end());
    std::string temp = RemoveChunk(content, "\\");
    temp = temp.substr(1, temp.length() - 2);
    if (!reader.parse(temp, root, false))
        return false;

    uint32 success = 0;
    auto memberNames = root.getMemberNames();
    for (const auto& member : memberNames)
    {
        if (member == "success")
            success = GetInt32FromJsonValue(root, "success");
        else if (member == "url")
            *url = root.get("url", "").asString();
        else if (member == "ttoken")
            *ttoken = root.get("ttoken", "").asString();
    }

    if (success != 1 || url->empty() || ttoken->empty())
        return false;

    return true;
}


bool HuyaLogic::HuyaLoginStep2(CurlWrapper* curlWrapper, const std::string& url, 
                               const std::string& ttoken) const
{
    HttpRequest httpRequest;
    httpRequest.url = url;
    httpRequest.headers["Origin"] = "http://www.huya.com";
    httpRequest.referer = "http://www.huya.com/";
    httpRequest.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;

    HttpResponse httpResponse;
    if (!curlWrapper->Execute(httpRequest, &httpResponse))
        return false;

    for (const auto& it : httpResponse.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }
    return true;
}

bool HuyaLogic::HuyaLoginStep3(CurlWrapper* curlWrapper, 
                               const std::string& referer,
                               const std::string& ttoken,
                               const std::string& username,
                               const std::string password,
                               std::wstring* errmsg) const
{
    HttpRequest httpRequest;
    httpRequest.url = "https://lgn.yy.com/lgn/oauth/x2/s/login_asyn.do";
    httpRequest.headers["Origin"] = "https://lgn.yy.com";
    httpRequest.referer = referer;
    httpRequest.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_POST;
    httpRequest.cookies = cookiesHelper_->GetCookies("LGNJSESSIONID");
    std::string postdata;
    postdata += "username=";
    postdata += username;
    postdata += "&password=";
    postdata += password;
    postdata += "&oauth_token=";
    postdata += ttoken;
    postdata += "&denyCallbackURL=";
    postdata += "&UIStyle=xelogin&appid=5216&mxc=&vk=&isRemMe=0&mmc=&vv=";

    httpRequest.postdata.assign(postdata.begin(), postdata.end());
    HttpResponse httpResponse;
    if (!curlWrapper->Execute(httpRequest, &httpResponse))
        return false;
    
    for (const auto& it : httpResponse.cookies)
    {
        cookiesHelper_->SetCookies(it);
    }

    Json::Reader reader;
    Json::Value root(Json::objectValue);
    std::string content(httpResponse.content.begin(), httpResponse.content.end());
    std::string temp = RemoveChunk(content, "\\");
    if (!reader.parse(temp, root, false))
        return false;

    uint32 code = 0;
    std::string msg;
    auto memberNames = root.getMemberNames();
    for (const auto& member : memberNames)
    {
        if (member == "code")
            code = GetInt32FromJsonValue(root, "code");
        else if (member=="msg")
        {
            msg = root.get("msg", "").asString();
            *errmsg = base::UTF8ToWide(msg);
        }
    }

    if (code != 0)
    {
        return false;
    }
        

    return true;
}


