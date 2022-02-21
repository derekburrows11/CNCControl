// SendBytesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"

#include "CNCComms.h"

#include "SendBytesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSendBytesDlg dialog


CSendBytesDlg::CSendBytesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSendBytesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSendBytesDlg)
	m_iByte = 0;
	m_szList = _T("");
	//}}AFX_DATA_INIT
}


void CSendBytesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendBytesDlg)
	DDX_Text(pDX, IDC_EDIT1, m_iByte);
	DDX_LBString(pDX, IDC_LIST1, m_szList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendBytesDlg, CDialog)
	//{{AFX_MSG_MAP(CSendBytesDlg)
	ON_BN_CLICKED(IDSEND, OnSend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendBytesDlg message handlers

void CSendBytesDlg::OnSend() 
{
	UpdateData();
	BYTE arBytes[50];
	int iNumBytes = 1;

	arBytes[0] = (BYTE)m_iByte;
	m_pCNCComms->SendMonitorMessage(WMU_SENDBYTES, (int)arBytes, iNumBytes);		// value to send

	m_szList = "Next";
	UpdateData(false);			// sets dialog box

}
