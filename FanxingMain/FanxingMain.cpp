// FanxingMain.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include "TcpClient.h"
#include "CurlWrapper.h"

#pragma comment(lib,"ws2_32.lib")

bool GlobalInit()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    CurlWrapper::CurlInit();
    return true;
}

bool GlobalCleanup()
{
    CurlWrapper::CurlCleanup();
    WSACleanup();
    return true;
}

bool GetCookieTest()
{
    uint32 roomId = 1021987;
    bool ret = false;
    CurlWrapper curlWrapper;
    ret = curlWrapper.LoginRequestWithCookies();
    ret = curlWrapper.Services_UserService_UserService_getMyUserDataInfo();
    ret = curlWrapper.Services_IndexService_IndexService_getUserCenter();
    ret = curlWrapper.EnterRoom(roomId);//亲妞房间
    ret = curlWrapper.Servies_Uservice_UserService_getCurrentUserInfo(roomId);
    ret = curlWrapper.RoomService_RoomService_enterRoom(roomId);
    return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
    GlobalInit();

    GetCookieTest();

    GlobalCleanup();
	return 0;
}

