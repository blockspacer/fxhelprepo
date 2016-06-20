// TestProject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Network/TcpClient.h"
#include <iostream>
#include <assert.h>
#include "third_party/chromium/base/at_exit.h"
#include "third_party/chromium/base/run_loop.h"


TcpHandle g_handle;
void tcpclientcallback(TcpManager* tcpmanager, bool result, TcpHandle handle)
{
    if (!result)
        return;
    g_handle = handle;
    std::string str = "<policy-file-request/>";
    std::vector<char> data;
    data.assign(str.begin(), str.end());
    data.push_back(0);
    tcpmanager->Send(handle, data);
}

void clientcallback(bool result, const std::vector<char>& data)
{
    std::string str(data.begin(), data.end());
    std::cout << str;
}

bool TcpManagerTest()
{
    TcpManager tcpmanager;
    tcpmanager.Initialize();

    tcpmanager.AddClient(
        std::bind(tcpclientcallback, &tcpmanager, std::placeholders::_1, std::placeholders::_2),
        "114.54.2.204", 843, clientcallback);

    Sleep(10000);
    tcpmanager.RemoveClient(g_handle);
    while (1);
}

int _tmain(int argc, _TCHAR* argv[])
{   
    base::AtExitManager atExitManager;
    TcpManagerTest();
	return 0;
}

