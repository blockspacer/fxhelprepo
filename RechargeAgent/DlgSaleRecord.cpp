// DlgSaleRecord.cpp : 实现文件
//
#include "stdafx.h"
#include "RechargeAgent.h"
#include "DlgSaleRecord.h"



// CDlgSaleRecord 对话框

IMPLEMENT_DYNAMIC(CDlgSaleRecord, CDialogEx)

CDlgSaleRecord::CDlgSaleRecord(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgSaleRecord::IDD, pParent)
{

}

CDlgSaleRecord::~CDlgSaleRecord()
{
}

void CDlgSaleRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSaleRecord, CDialogEx)
END_MESSAGE_MAP()


// CDlgSaleRecord 消息处理程序
