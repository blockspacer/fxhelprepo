
// UserTracker.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
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

// CUserTrackerApp: 
// �йش����ʵ�֣������ UserTracker.cpp
//
class UserTrackerHelper;
class CLoginDlg;
class CUserTrackerDlg;

class CUserTrackerApp : public CWinApp
{
public:
	CUserTrackerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()

private:
    bool LoginProcedure(UserTrackerHelper* tracker_helper);
    bool TrackerProcedure(UserTrackerHelper* tracker_helper);
    bool VMPVerifyProcedure(UserTrackerHelper* tracker_helper);

    void InitAppLog();
    scoped_ptr<base::AtExitManager> atExitManager_;
};

extern CUserTrackerApp theApp;