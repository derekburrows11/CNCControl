// PortSelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CNCControl.h"

#include "PortSelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPortSelDlg dialog


CPortSelDlg::CPortSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPortSelDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPortSelDlg)
	m_iPort = -1;
	//}}AFX_DATA_INIT
}


void CPortSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortSelDlg)
	DDX_Radio(pDX, IDC_RADIO1, m_iPort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortSelDlg, CDialog)
	//{{AFX_MSG_MAP(CPortSelDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPortSelDlg message handlers
