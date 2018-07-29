
// SingerRank.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号
#include "third_party/chromium/base/memory/scoped_ptr.h"

namespace base
{
    class AtExitManager;
}


// CSingerRankApp: 
// 有关此类的实现，请参阅 SingerRank.cpp
//

class CSingerRankApp : public CWinApp
{
public:
	CSingerRankApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现
    scoped_ptr<base::AtExitManager> atExitManager_;
	DECLARE_MESSAGE_MAP()
};

extern CSingerRankApp theApp;