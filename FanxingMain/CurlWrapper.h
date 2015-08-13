#pragma once
#include <string>
#include "third_party/chromium/base/basictypes.h"
// 提供方便的使用curl接口的执行请求函数。
class CurlWrapper
{
public:
    CurlWrapper();
    ~CurlWrapper();
    static void CurlInit();
    static void CurlCleanup();

    bool LoginRequest();
    bool Services_UserService_UserService_getMyUserDataInfo();
    bool Services_IndexService_IndexService_getUserCenter();

    bool EnterRoom(uint32 roomid);

    bool WriteCallback(const std::string& data);

};

