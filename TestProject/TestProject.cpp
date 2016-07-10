// TestProject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "Network/Network.h"
#include "third_party/chromium/base/at_exit.h"
#include "third_party/chromium/base/run_loop.h"

bool MakePostdata(const std::map<std::string, std::string>& postmap,
    std::vector<uint8>* postdata)
{
    if (postmap.empty())
        return false;

    std::string temp;
    bool first = true;
    for (const auto& param : postmap)
    {
        if (first)
            first = false;
        else
            temp += "&";

        temp += param.first + "=" + UrlEncode(param.second);
    }
    postdata->assign(temp.begin(), temp.end());
    return true;
}

void DoRobotTest()
{
    CurlWrapper curlWrapper;
    HttpRequest httpRequest;
    httpRequest.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_POST;
    httpRequest.url = "http://www.tuling123.com//openapi/api";
    std::map<std::string, std::string> postmap;
    postmap["key"] = "21ef7a39254642f721736081e8e2226d";
    postmap["info"] = "chat info";
    postmap["userid"] = "123";
    bool result = MakePostdata(postmap, &httpRequest.postdata);

    HttpResponse httpResponse;
    result = curlWrapper.Execute(httpRequest, &httpResponse);

}

int _tmain(int argc, _TCHAR* argv[])
{   
    base::AtExitManager atExitManager;

    NetworkInitialize();

    DoRobotTest();

    NetworkFainalize();
	return 0;
}

