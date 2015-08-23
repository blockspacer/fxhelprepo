// FanxingMain.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include "TcpClient.h"
#include "CurlWrapper.h"
#include "GiftNotifyManager.h"

#include <string>
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_path.h"

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

bool RunTest()
{
    uint32 roomid = 1051837;
    bool ret = false;
    CurlWrapper curlWrapper;

    ret = curlWrapper.LoginRequestWithCookies();
    assert(ret);
    ret = curlWrapper.Services_UserService_UserService_getMyUserDataInfo();
    assert(ret);
    ret = curlWrapper.Services_IndexService_IndexService_getUserCenter();
    assert(ret);
    ret = curlWrapper.EnterRoom(roomid);
    assert(ret);
    ret = curlWrapper.Servies_Uservice_UserService_getCurrentUserInfo(roomid);
    assert(ret);
    ret = curlWrapper.RoomService_RoomService_enterRoom(roomid);
    assert(ret);

    uint32 userid = 0;
    std::string nickname = "";
    uint32 richlevel = 0;
    uint32 ismaster = 0;
    uint32 staruserid = 0;
    std::string key = "";   
    std::string ext = "";

    curlWrapper.ExtractUsefulInfo_RoomService_enterRoom(&userid,
        &nickname, &richlevel, &staruserid, &key, &ext);

    GiftNotifyManager giftNotifyManager;
    ret = giftNotifyManager.Connect843();
    assert(ret);
    ret = giftNotifyManager.Connect8080(roomid, userid, nickname, richlevel,
        ismaster, staruserid, key, ext);
    assert(ret);
    return ret;
}

void RunUnitTest()
{
    CurlWrapper curlWrapper;
    curlWrapper.GiftService_GiftService(123, "123456");
}
void test_get_key_data()
{
    base::FilePath path(std::wstring(L"d:/response.txt"));
    base::File fileobject;
    fileobject.Initialize(path, base::File::FLAG_READ | base::File::FLAG_OPEN);
    bool valid = fileobject.IsValid();
    fileobject.Seek(base::File::FROM_BEGIN, 0L);
    auto len = fileobject.GetLength();
    std::string responsedata;
    responsedata.resize(len);
    auto readsize = fileobject.ReadAtCurrentPos(&responsedata[0], len);

    CurlWrapper curlWrapper;
    //curlWrapper.ExtractUsefulInfo_RoomService_enterRoom(responsedata);
}

// 接收flash tcp线程收到的数据回调消息
void NotifyFunction()
{

}

int _tmain(int argc, _TCHAR* argv[])
{
    GlobalInit();
    RunTest();
    while (1);
    GlobalCleanup();
	return 0;
}

