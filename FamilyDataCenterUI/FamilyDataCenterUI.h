
// FamilyDataCenterUI.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CFamilyDataCenterUIApp: 
// �йش����ʵ�֣������ FamilyDataCenterUI.cpp
//

class CFamilyDataCenterUIApp : public CWinApp
{
public:
	CFamilyDataCenterUIApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CFamilyDataCenterUIApp theApp;