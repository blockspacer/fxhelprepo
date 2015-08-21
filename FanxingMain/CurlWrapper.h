#pragma once
#include <string>
#include "third_party/chromium/base/basictypes.h"
#include"third_party/chromium/base/files/file.h"

// 提供方便的使用curl接口的执行请求函数。
class CurlWrapper
{
public:
    CurlWrapper();
    ~CurlWrapper();
    static void CurlInit();
    static void CurlCleanup();

    bool LoginRequestWithCookies();
    bool LoginRequestWithUsernameAndPassword(const std::string& username, 
                                             const std::string& password);
    bool Services_UserService_UserService_getMyUserDataInfo();
    bool Services_IndexService_IndexService_getUserCenter();

    bool EnterRoom(uint32 roomid);

	// 在进入房间以后，获取用户信息
	bool Servies_Uservice_UserService_getCurrentUserInfo(uint32 roomid);

    // 关键数据获取，返回数据里面包含tcp请求中需要带的key参数值
    bool RoomService_RoomService_enterRoom(uint32 roomid);

    bool WriteCallback(const std::string& data);

private:
    //base::File file_;
    std::string currentWriteData_;
    std::string response_of_RoomService_RoomService_enterRoom_;
    std::string response_of_Services_UserService_UserService_getMyUserDataInfo_;
};

