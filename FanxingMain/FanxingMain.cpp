// FanxingMain.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include "TcpClient.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/basictypes.h"
#include "encodehelper.h"
#include "CookiesManager.h"

#pragma comment(lib,"ws2_32.lib")

bool GlobalInit()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    return true;
}

bool GlobalCleanup()
{
    WSACleanup();
    return true;
}

bool GetFirstPackage(std::vector<uint8> *packagedata)
{
    uint32 nowtime = static_cast<uint32>(base::Time::Now().ToDoubleT());

    Json::FastWriter writer;
    Json::Value root(Json::objectValue);
    root["cmd"];
    root["roomid"];
    root["userid"];
    root["nickname"];
    root["richlevel"];
    root["ismaster"];
    root["staruserid"];
    root["key"];
    root["keytime"];
    root["ext"];
    return false;
}

bool GetCookieTest()
{
    std::string cookie = "";
    bool ret = false;
    ret = CookiesManager::GetCookies(&cookie);

    return ret;
}
int _tmain(int argc, _TCHAR* argv[])
{
    GlobalInit();

    GetCookieTest();
    while (1);
    GlobalCleanup();
	return 0;
}

