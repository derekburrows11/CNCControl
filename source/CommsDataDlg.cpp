// CommsDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CNCControl.h"

#include "CommsDataDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommsDataDlg dialog

BEGIN_MESSAGE_MAP(CCommsDataDlg, CDialog)
	//{{AFX_MSG_MAP(CCommsDataDlg)
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



CCommsDataDlg::CCommsDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	m_clrTxText = RGB(255, 100, 100);
	m_clrRxText = RGB(150, 150, 255);
	m_clrBkgnd = RGB(50, 50, 50);
	m_brTxRxBkgnd.CreateSolidBrush(m_clrBkgnd);

	m_iTxBoxChars = 0;
	m_iRxBoxChars = 0;

	Create(IDD);
}


void CCommsDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommsDataDlg)
	DDX_Control(pDX, IDC_EDIT1, m_TxBoxEdit);
	DDX_Control(pDX, IDC_EDIT2, m_RxBoxEdit);
	//}}AFX_DATA_MAP
}



/////////////////////////////////////////////////////////////////////////////
// CCommsDataDlg message handlers


HBRUSH CCommsDataDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
//	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	// TODO: Change any attributes of the DC here
	if (pWnd == (CWnd*)&m_TxBoxEdit)
	{
		pDC->SetTextColor(m_clrTxText);
		pDC->SetBkColor(m_clrBkgnd);
	}
	else if (pWnd == (CWnd*)&m_RxBoxEdit)
	{
		pDC->SetTextColor(m_clrRxText);
		pDC->SetBkColor(m_clrBkgnd);
	}
	else
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	// TODO: Return a different brush if the default is not desired
	return m_brTxRxBkgnd;
}


bool CCommsDataDlg::AddToEditBox(CEdit& edit, BYTE* data, int num, bool bNewLine)
{
	if (!IsWindowVisible())
		return false;
	char txt[200];
	num = min(60, num);
	char* pTxt = txt;
	if (num > 0)
	for (int i = 0;;)
	{
		pTxt += sprintf(pTxt, "%02x ", data[i]);	// \r\n is return
		if (++i >= num)
			break;
		if (i % 16 == 0)
			strcpy(pTxt++-1, "\r\n");		// write over last space
	}
	if (bNewLine)
		if (num == 0)
		{
			strcpy(pTxt, "\r\n");
			pTxt += 2;
		}
		else if (*(pTxt-1) != '\n')
			strcpy((pTxt++)-1, "\r\n");

	int* piSize;
	if (&edit == &m_TxBoxEdit)
		piSize = &m_iTxBoxChars;
	else if (&edit == &m_RxBoxEdit)
		piSize = &m_iRxBoxChars;
	*piSize += pTxt - txt;		// size after text is added

	if (*piSize > 10000)
	{
		int iLoc = edit.LineIndex(10);		// remove first 10 lines
		edit.SetSel(0, iLoc, true);
		edit.ReplaceSel("");
		*piSize -= iLoc;
	}

//	int start, end;
//	edit.GetSel(start, end);
	edit.SetSel(-2, -2);			// anything large sets to end except -1
	edit.ReplaceSel(txt);

	int len = edit.GetWindowTextLength();
	ASSERT(len == *piSize);
	return true;
}

void CCommsDataDlg::OnClose() 
{
//	PostMessage(WM_COMMAND, ID_MACHINE_SHOWCOMMSDATA);
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_MACHINE_SHOWCOMMSDATA);
//	AfxGetApp()->PostThreadMessage(WM_COMMAND, ID_MACHINE_SHOWCOMMSDATA, 0);	// doesn't work
	
//	CDialog::OnClose();
}
