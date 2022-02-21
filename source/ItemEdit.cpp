// ItemEdit.cpp : implementation file
//

#include "stdafx.h"
#include "CNCControl.h"
#include "ItemEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CItemEdit

CItemEdit::CItemEdit()
{
}

CItemEdit::~CItemEdit()
{
}


BEGIN_MESSAGE_MAP(CItemEdit, CEdit)
	//{{AFX_MSG_MAP(CItemEdit)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CItemEdit message handlers

void CItemEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CItemEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
	case VK_RETURN:
		GetParent()->SendMessage(WM_COMMAND,
			MAKELONG(GetDlgCtrlID(), EN_END), (LPARAM)m_hWnd);
		break;
	case VK_ESCAPE:
		GetParent()->SendMessage(WM_COMMAND,
			MAKELONG(GetDlgCtrlID(), EN_CANCEL), (LPARAM)m_hWnd);
		break;
	default:
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
}
