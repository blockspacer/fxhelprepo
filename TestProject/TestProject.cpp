// TestProject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <assert.h>
#include "third_party/chromium/base/at_exit.h"
#include "Network/CurlWrapper.h"
#include "TcpProxyClient.h"

bool TcpProxyTest(const std::string& proxyip)
{
    TcpProxyClient client;
    client.SetProxy(proxyip.c_str(), 1080);
    return client.ConnectToProxy("42.62.68.50", 843);
}
void SingleUserSingleRoomTest();

// 测试通过
void CurlTest()
{
    CurlWrapper curlWrapper;
    curlWrapper.CurlInit();
    HttpRequest request;
    request.url = "http://www.kugou.com/kf/client/app/i/stream.php?qid=5255440";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_HTTPPOST;
    request.postfile = "C:/Users/Administrator/AppData/Roaming/KuGou8/7643-1-83e43a6ceccefebffdf4da4403291e87-2014-10-18_01_51_09.dmp";
    HttpResponse response;
    bool result = curlWrapper.Execute(request, &response);

}
int _tmain(int argc, _TCHAR* argv[])
{   
    base::AtExitManager atExitManager;
    SingleUserSingleRoomTest();
    //std::vector<std::string> ipvector =
    //{
    //    //"61.157.198.67",
    //    //"24.196.69.180",
    //    //"37.32.44.1",
    //    "42.159.244.217"
    //};
    //for (auto it : ipvector)
    //{
    //    if (TcpProxyTest(it))
    //    {
    //        std::cout << it.c_str() << " connect success";
    //    }
    //}

	return 0;
}

