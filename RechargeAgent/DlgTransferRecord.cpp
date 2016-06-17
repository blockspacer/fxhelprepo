// DlgTransferRecord.cpp : 实现文件
//
#include "stdafx.h"
#include "RechargeAgent.h"
#include "afxdialogex.h"
#include "DlgTransferRecord.h"

// CDlgTransferRecord 对话框

IMPLEMENT_DYNAMIC(CDlgTransferRecord, CDialogEx)

CDlgTransferRecord::CDlgTransferRecord(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgTransferRecord::IDD, pParent)
{

}

CDlgTransferRecord::~CDlgTransferRecord()
{
}

void CDlgTransferRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgTransferRecord, CDialogEx)
END_MESSAGE_MAP()


// CDlgTransferRecord 消息处理程序
