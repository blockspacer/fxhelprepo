
// SingerRank.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "third_party/chromium/base/memory/scoped_ptr.h"

namespace base
{
    class AtExitManager;
}


// CSingerRankApp: 
// �йش����ʵ�֣������ SingerRank.cpp
//

class CSingerRankApp : public CWinApp
{
public:
	CSingerRankApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��
    scoped_ptr<base::AtExitManager> atExitManager_;
	DECLARE_MESSAGE_MAP()
};

extern CSingerRankApp theApp;