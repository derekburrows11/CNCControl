// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "CNCControl.h"

#include "Store.h"

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	m_szStorageLabel = NULL;
	
}

CChildFrame::~CChildFrame()
{
}


/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers



BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
//	cs.cx = 200;
//	cs.cy = 100;
	cs.style &= ~FWS_ADDTOTITLE;		// remove so can add title manually, not document name!
	return CMDIChildWnd::PreCreateWindow(cs);
}

int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// SetWindowPlacement can be before or after (CMDIChildWnd::OnCreate....
	WINDOWPLACEMENT wp;
	if (ReadWindowPlacement(&wp, m_szStorageLabel))
		SetWindowPlacement(&wp);
	
	return 0;
}

bool CChildFrame::SetStorageLabel(char* szLabel)
{
	m_szStorageLabel = szLabel;
	if (m_hWnd != NULL)
	{
		WINDOWPLACEMENT wp;
		if (ReadWindowPlacement(&wp, m_szStorageLabel))
		{
			SetWindowPlacement(&wp);
			return true;
		}
	}
	return false;
}

void CChildFrame::OnClose()	// not always called
{
	
	CMDIChildWnd::OnClose();
}

void CChildFrame::OnDestroy()
{
	WINDOWPLACEMENT wp;
	if (GetWindowPlacement(&wp))
		WriteWindowPlacement(&wp, m_szStorageLabel);

	CMDIChildWnd::OnDestroy();
	// TODO: Add your message handler code here
}
