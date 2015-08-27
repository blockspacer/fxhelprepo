// FanxingMain.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include "TcpClient.h"
#include "CurlWrapper.h"
#include "GiftNotifyManager.h"
#include <iostream>
#include <string>
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_path.h"
#include <mutex> 
#include <condition_variable>
#pragma comment(lib,"ws2_32.lib")


uint32 g_userid;
std::string g_key;
std::string g_data;
std::mutex g_mtx;
std::mutex g_mtx_601;
std::condition_variable g_cv;
uint32 g_commandid;

void Lock()
{
    g_mtx_601.lock();
}

void Unlock()
{
    g_mtx_601.unlock();
}

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

// 接收flash tcp线程收到的数据回调消息
void globalNotifyFunction_601(uint32 userid, const std::string& key)
{
    Lock();
    g_commandid = 601;
    g_userid = userid;
    g_key = key;
    std::unique_lock<std::mutex> lck(g_mtx);
    g_cv.notify_one();
    Unlock();
}

void globalNotifyFunction(const std::string& data)
{
    Lock();
    g_commandid = 0;
    g_data = data;
    std::unique_lock<std::mutex> lck(g_mtx);
    g_cv.notify_one();
    Unlock();
}

void Wait()
{
    std::unique_lock<std::mutex> lck(g_mtx);
    g_cv.wait(lck);
}

bool RunTest()
{
    uint32 roomid = 1013785;
    bool ret = false;
    CurlWrapper curlWrapper;

    ret = curlWrapper.LoginRequestWithCookies();
    //std::string username = "fanxingtest001";
    //std::string password = "1233211234567";
    //ret = curlWrapper.LoginRequestWithUsernameAndPassword(username, password);
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
    giftNotifyManager.Set601Notify(globalNotifyFunction_601);
    giftNotifyManager.SetNormalNotify(globalNotifyFunction);
    ret = giftNotifyManager.Connect8080(roomid, userid, nickname, richlevel,
        ismaster, staruserid, key, ext);
    assert(ret);
    while (1)
    {
        Wait();
        if (g_commandid == 601)
        {
            for (auto i = 0; i < 10; i++)
            {
                Sleep(1000);
                curlWrapper.GiftService_GiftService(g_userid, g_key);
            }
        }
        else if (g_commandid == 0)
        {
            std::cout << g_data << std::endl;
        }
        else
        {
            std::cout << g_data << std::endl;
        }
        
    }
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



int _tmain(int argc, _TCHAR* argv[])
{
    GlobalInit();
    RunTest();
    GlobalCleanup();
	return 0;
}

