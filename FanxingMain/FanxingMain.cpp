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
    uint32 roomId = 1053121;
    bool ret = false;
    CurlWrapper curlWrapper;
    ret = curlWrapper.LoginRequestWithCookies();
    assert(ret);
    ret = curlWrapper.Services_UserService_UserService_getMyUserDataInfo();
    assert(ret);
    ret = curlWrapper.Services_IndexService_IndexService_getUserCenter();
    assert(ret);
    ret = curlWrapper.EnterRoom(roomId);
    assert(ret);
    ret = curlWrapper.Servies_Uservice_UserService_getCurrentUserInfo(roomId);
    assert(ret);
    ret = curlWrapper.RoomService_RoomService_enterRoom(roomId);
    assert(ret);
    return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
    GlobalInit();

    GetCookieTest();

    GlobalCleanup();
	return 0;
}

