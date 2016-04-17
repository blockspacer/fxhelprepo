// Network.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "CurlWrapper.h"

bool NetworkInitialize()
{
    return CurlWrapper::CurlInit();
}
void NetworkFainalize()
{
    CurlWrapper::CurlCleanup();
}


