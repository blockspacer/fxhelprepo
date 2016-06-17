
// RechargeAgent.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号
#include "third_party/chromium/base/memory/scoped_ptr.h"


// CRechargeAgentApp: 
// 有关此类的实现，请参阅 RechargeAgent.cpp
//

namespace base
{
    class AtExitManager;
}

class CRechargeAgentApp : public CWinApp
{
public:
	CRechargeAgentApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

    DECLARE_MESSAGE_MAP()
private:
    void InitAppLog();
    scoped_ptr<base::AtExitManager> atExitManager_;
};

extern CRechargeAgentApp theApp;