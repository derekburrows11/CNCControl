// PathPositionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"
#include "PathPositionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPathPositionDlg dialog


CPathPositionDlg::CPathPositionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPathPositionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPathPositionDlg)
	m_strStartSeg = _T("0");
	m_iNumSegsShow = 50;		// default
	m_nrStartSeg = 0;
	//}}AFX_DATA_INIT
	
	m_iStartSeg = 0;			// default
	m_iNumSegsPath = 50;		// default
	m_pViewWnd = pParent;
	m_bUseNR = false;

}

CPathPositionDlg::~CPathPositionDlg()
{
	DestroyWindow();
	if (m_hWnd != NULL)
		TRACE("calling CPathPositionDlg::~CPathPositionDlg() with m_hWnd != NULL!\n");
	CDialog::~CDialog();
}

void CPathPositionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	if (!pDX->m_bSaveAndValidate)		// setting controls
	{
		// validate values
		if (m_iStartSeg < 0) m_iStartSeg = 0;
		if (m_iStartSeg > m_iNumSegsPath) m_iStartSeg = m_iNumSegsPath;
		if (m_iNumSegsShow < 0) m_iNumSegsShow = 0;
		if (m_iNumSegsShow > m_iNumSegsPath) m_iNumSegsShow = m_iNumSegsPath;

		char sz[20];
		itoa(m_iStartSeg, sz, 10);
		m_strStartSeg = sz;

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nMax = m_iNumSegsPath;
		si.nPage = m_iNumSegsShow;
		si.nPos = m_iStartSeg;
		m_scrollPos.SetScrollInfo(&si);
	}
	//{{AFX_DATA_MAP(CPathPositionDlg)
	DDX_Control(pDX, IDC_SCROLLBAR1, m_scrollPos);
	DDX_CBString(pDX, IDC_COMBO1, m_strStartSeg);
	DDV_MaxChars(pDX, m_strStartSeg, 12);
	DDX_Text(pDX, IDC_EDIT1, m_iNumSegsShow);
	DDX_Text(pDX, IDC_EDIT2, m_nrStartSeg);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPathPositionDlg, CDialog)
	//{{AFX_MSG_MAP(CPathPositionDlg)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnEditChange)
	ON_CBN_EDITCHANGE(IDC_COMBO1, OnEditChangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON1, OnViewAll)
	ON_WM_HSCROLL()
	ON_EN_KILLFOCUS(IDC_EDIT2, OnEditChange)
	ON_CBN_KILLFOCUS(IDC_COMBO1, OnEditChange)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnEditChange)
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPathPositionDlg::OnOK()		// return pressed read edits
{
	OnEditChange();
}
void CPathPositionDlg::OnCancel()
{
	CDialog::OnCancel();
//	DestroyWindow();
}
void CPathPositionDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	// delete this if new'ed and owner doesn't
	CDialog::PostNcDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CPathPositionDlg message handlers

BOOL CPathPositionDlg::Create(CWnd* pViewWnd /*=NULL*/) 
{
	BOOL res = CDialog::Create(IDD);
	ASSERT(res);
	pViewWnd = pViewWnd;
/*	if (pViewWnd)
		m_pViewWnd = pViewWnd;
	else
		m_pViewWnd = GetParent();
*/
	return res;
}


void CPathPositionDlg::OnEditChange()			// when edit values are changed
{
	// make copies of both start controls to check which change
	int segStart = m_iStartSeg;
	int nrStart = m_nrStartSeg;

	UpdateData();
	m_iStartSeg = atoi(m_strStartSeg);			// set from edit box
	UpdateData(false);
	m_bUseNR = nrStart != m_nrStartSeg;
	NotifyView();
	m_bUseNR = false;
}


void CPathPositionDlg::OnEditChangeCombo1() 
{
	// TODO: Add your control notification handler code here
	
}

void CPathPositionDlg::OnSelchangeCombo1() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL CPathPositionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPathPositionDlg::OnViewAll() 
{
	m_iStartSeg = 0;
	m_iNumSegsShow = m_iNumSegsPath;
	UpdateData(false);
	NotifyView();
}

void CPathPositionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{	// scroller message goes to OnHScroll(), OnVScroll not used
	if (pScrollBar != &m_scrollPos)
		return;
	switch (nSBCode)
	{
	case SB_LEFT:
		m_iStartSeg = 0;
		break;
	case SB_RIGHT:
		m_iStartSeg = m_iNumSegsPath - m_iNumSegsShow + 1;
		break;
	case SB_LINELEFT:
		m_iStartSeg -= 1;
		break;
	case SB_LINERIGHT:
		m_iStartSeg += 1;
		break;
	case SB_PAGELEFT:
		m_iStartSeg -= m_iNumSegsShow;
		break;
	case SB_PAGERIGHT:
		m_iStartSeg += m_iNumSegsShow;
		break;
	case SB_THUMBPOSITION:		// 4
	case SB_THUMBTRACK:			// 5
		m_iStartSeg = nPos;
		break;
	}
	UpdateData(false);
	NotifyView();
}


void CPathPositionDlg::NotifyView()
{
	// sends control notification message
	// View only invalidates if parameters have changed
	if (m_pViewWnd && IsWindow(m_pViewWnd->m_hWnd))
		m_pViewWnd->SendMessage(WM_COMMAND, MAKEWPARAM(IDD, DLGN_VALUECHANGED), (LPARAM)m_hWnd);
//	GetOwner()->SendMessage(WM_COMMAND, MAKEWPARAM(IDD, DLGN_VALUECHANGED), (LPARAM)m_hWnd);
//	doesn't work GetParentOwner()->SendMessage(WM_COMMAND, MAKEWPARAM(IDD, DLGN_VALUECHANGED), (LPARAM)m_hWnd);
//	m_pViewWnd->SendMessage(WM_COMMAND, MAKEWPARAM(IDD, DLGN_VALUECHANGED), (LPARAM)m_hWnd);

	// sends command message
//	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(DLGN_VALUECHANGED, 0), (LPARAM)m_hWnd);
//	GetOwner()->SendMessage(WM_COMMAND, MAKEWPARAM(DLGN_VALUECHANGED, 0), (LPARAM)m_hWnd);
//	doesn't work GetParentOwner()->SendMessage(WM_COMMAND, MAKEWPARAM(DLGN_VALUECHANGED, 0), (LPARAM)m_hWnd);
//	m_pViewWnd->SendMessage(WM_COMMAND, MAKEWPARAM(DLGN_VALUECHANGED, 0), (LPARAM)m_hWnd);

}



void CPathPositionDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	
	// TODO: Add your message handler code here
	
}
