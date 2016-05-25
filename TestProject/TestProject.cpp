// TestProject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <assert.h>
#include "third_party/chromium/base/at_exit.h"

#include "TcpProxyClient.h"

bool TcpProxyTest(const std::string& proxyip)
{
    TcpProxyClient client;
    client.SetProxy(proxyip.c_str(), 1080);
    return client.ConnectToProxy("42.62.68.50", 843);
}

int _tmain(int argc, _TCHAR* argv[])
{
    base::AtExitManager atExitManager;
    std::vector<std::string> ipvector =
    {
        //"61.157.198.67",
        //"24.196.69.180",
        //"37.32.44.1",
        "42.159.244.217"
    };
    for (auto it : ipvector)
    {
        if (TcpProxyTest(it))
        {
            std::cout << it.c_str() << " connect success";
        }
    }

	return 0;
}

