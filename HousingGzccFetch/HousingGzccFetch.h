
// HousingGzccFetch.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/at_exit.h"

// CHousingGzccFetchApp: 
// 有关此类的实现，请参阅 HousingGzccFetch.cpp
//

class CHousingGzccFetchApp : public CWinApp
{
public:
	CHousingGzccFetchApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()

private:
    base::AtExitManager at_exit_manager_;

    void InitAppLog();
};

extern CHousingGzccFetchApp theApp;