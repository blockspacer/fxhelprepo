// FanxingMain.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include "TcpClient.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/basictypes.h"
#include "encodehelper.h"

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

int _tmain(int argc, _TCHAR* argv[])
{
    GlobalInit();
    //TcpClient tcpClient;

    //tcpClient.Connect("42.62.68.50", 843);

    //std::string str = "<policy-file-request/>";
    //std::vector<char> data;
    //data.assign(str.begin(), str.end());
    //data.push_back(0);
    //tcpClient.Send(data);


    std::string decodestr = UrlDecode(std::string("%7B%22ui%22%3A0%2C%22isSpRoom%22%3A0%2C%22stli%22%3A%7B%22st%22%3A%222%22%2C%22sl%22%3A0%7D%2C%22userGuard%22%3A%5B%5D%2C%22starCard%22%3A0%2C%22external%22%3A0%2C%22exMemo%22%3A%22%22%2C%22worship%22%3A3%2C%22z%22%3A0%2C%22isGoldFans%22%3A0%2C%22token%22%3A%22c920ae50679115ada1ef5f89b3508294a47f797da3294bf57cc8b74582416f65%22%2C%22kugouId%22%3A454395944%7D"));
    TcpClient tcpClient_8080;

    tcpClient_8080.Connect("42.62.68.50", 8080);
    std::vector<uint8> data_for_send;
    GetFirstPackage(&data_for_send);
    std::string str = R"({ "cmd":201, "roomid" : 1021987, "userid" : "40919199", "nickname" : "............", "richlevel" : 2, "ismaster" : 0, "staruserid" : "22616837", "key" : "c98672e20fdd8bf2b32d0e421a3b8a12", "keytime" : 1438686392, "ext" : "%7B%22ui%22%3A0%2C%22isSpRoom%22%3A0%2C%22stli%22%3A%7B%22st%22%3A%222%22%2C%22sl%22%3A0%7D%2C%22userGuard%22%3A%5B%5D%2C%22starCard%22%3A0%2C%22external%22%3A0%2C%22exMemo%22%3A%22%22%2C%22worship%22%3A3%2C%22z%22%3A0%2C%22isGoldFans%22%3A0%2C%22token%22%3A%22c920ae50679115ada1ef5f89b3508294a47f797da3294bf57cc8b74582416f65%22%2C%22kugouId%22%3A454395944%7D" })";
    std::vector<char> data_8080;
    data_8080.assign(str.begin(), str.end());
    data_8080.push_back(0);
    tcpClient_8080.Send(data_8080);


    while (1);
    GlobalCleanup();
	return 0;
}

