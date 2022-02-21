// PlayerBar.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"
#include "PlayerBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//#define RATEBOX_INDEX 4
//#define RATEBOX_BITMAP 3
#define RATEBOX_WIDTH 60
#define RATEBOX_HEIGHT 20



/////////////////////////////////////////////////////////////////////////////
// CPlayerBar

CPlayerBar::CPlayerBar()
{
	m_Rate = 1;
}

CPlayerBar::~CPlayerBar()
{
}


BEGIN_MESSAGE_MAP(CPlayerBar, CToolBar)
	//{{AFX_MSG_MAP(CPlayerBar)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_EN_KILLFOCUS(IDC_EDIT_RATE, OnEditChange)
	ON_COMMAND(IDC_EDIT_RATE, OnEditRate)
	ON_UPDATE_COMMAND_UI(IDC_EDIT_RATE, OnUpdateEditRate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CPlayerBar::LoadToolBar(UINT nIDResource)
{
	if (!CToolBar::LoadToolBar(nIDResource))
		return false;
//	CRect rect(-RATEBOX_WIDTH, -RATEBOX_HEIGHT, 0, 0);
	CRect rect(0, 0, RATEBOX_WIDTH, RATEBOX_HEIGHT);
//	CRect rect(-1, -1, 0, 0);
//	if (!m_RateBox.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_AUTOHSCROLL |
//			WS_VSCROLL | CBS_HASSTRINGS, rect, this, IDC_EDIT_RATE))
	if (!m_RateBox.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT, rect, this, IDC_EDIT_RATE))
		return false;
//	m_RateBox.ModifyStyleEx(0, WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY);	// gives 3d border
	// The ID of the ComboBox is important for two reasons.  One, so you
	// can receive notifications from the control.  And also for ToolTips.
	// During HitTesting if the ToolBar sees that the mouse is one a child
	// control, the toolbar will lookup the controls ID and search for a
	// string in the string table with the same ID to use for ToolTips
	// and StatusBar info.
	SetBarStyle(GetBarStyle() | CBRS_ALIGN_TOP);
	int iButton = CommandToIndex(IDC_EDIT_RATE);	// find index to replace botton with control
	if (iButton == -1)
		return true;
	SetButtonInfo(iButton, IDC_EDIT_RATE, TBBS_SEPARATOR, RATEBOX_WIDTH);

	GetItemRect(iButton, rect);
	m_RateBox.SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOCOPYBITS);
//	m_RateBox.ShowWindow(SW_SHOW);

//	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
//	if (hFont == NULL)
//		hFont = (HFONT)GetStockObject(ANSI_VAR_FONT);
//	m_RateBox.SendMessage(WM_SETFONT, (WPARAM)hFont);

	UpdateData(false);
	return true;
}


void CPlayerBar::DoDataExchange(CDataExchange* pDX)
{
//	CToolBar::DoDataExchange(pDX);		// does nothing
	DDX_Text(pDX, IDC_EDIT_RATE, m_Rate);
}

/////////////////////////////////////////////////////////////////////////////
// CPlayerBar message handlers

void CPlayerBar::OnEditChange()			// when edit values are changed
{
	UpdateData();
	// check values
	UpdateData(false);
	NotifyView();
}

void CPlayerBar::NotifyView()
{
	// sends control notification message
	AfxGetMainWnd()->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), DLGN_VALUECHANGED), (LPARAM)m_hWnd);

	// sends command message
	AfxGetMainWnd()->SendMessage(WM_COMMAND, MAKEWPARAM(IDC_EDIT_RATE, 0), NULL);
}

void CPlayerBar::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_RETURN)
		NotifyView();
	// TODO: Add your message handler code here and/or call default
	
	CToolBar::OnChar(nChar, nRepCnt, nFlags);
}

void CPlayerBar::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_RETURN)
		NotifyView();
	// TODO: Add your message handler code here and/or call default
	
	CToolBar::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CPlayerBar::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message != WM_KEYDOWN) || (pMsg->wParam != VK_RETURN))
		return CToolBar::PreTranslateMessage(pMsg);
	OnEditChange();
	return true;
}

void CPlayerBar::OnEditRate() 
{
	// TODO: Add your command handler code here
	
}

void CPlayerBar::OnUpdateEditRate(CCmdUI* /*pCmdUI*/) 
{
//	pCmdUI->Enable(0);
	
}
