// GridView.cpp : implementation file
//

#include "stdafx.h"
#include "CNCControl.h"
#include "ItemEdit.h"
#include "GridView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridView

IMPLEMENT_DYNCREATE(CGridView, CListView)

BEGIN_MESSAGE_MAP(CGridView, CListView)
	//{{AFX_MSG_MAP(CGridView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_CONTROL(EN_UPDATE, IDC_GRIDEDIT, OnEditUpdate)
	ON_EN_CHANGE(IDC_GRIDEDIT, OnEditChange)
	ON_EN_SETFOCUS(IDC_GRIDEDIT, OnEditSetFocus)
	ON_EN_KILLFOCUS(IDC_GRIDEDIT, OnEditKillFocus)
	ON_CONTROL(EN_END, IDC_GRIDEDIT, OnEditEnd)
	ON_CONTROL(EN_CANCEL, IDC_GRIDEDIT, OnEditCancel)
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnLVKeydown)
	ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
	ON_WM_NCCALCSIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CGridView::CGridView()
{
	m_clrText = ::GetSysColor(COLOR_WINDOWTEXT);
	m_clrTextBk = ::GetSysColor(COLOR_WINDOW);
	m_clrBkgnd = ::GetSysColor(COLOR_WINDOW);
	m_iFocusItem = -1;
	m_iFocusSubItem = -1;

	m_pItemEdit = NULL;

	m_CellColor = NULL;

}

CGridView::~CGridView()
{
	delete m_pItemEdit;		// normally doesn't exist now
	delete []m_CellColor;
}

/////////////////////////////////////////////////////////////////////////////
// CParamView initialization

BOOL CGridView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style &= ~LVS_TYPEMASK;
//	cs.style |= LVS_SHOWSELALWAYS;
	cs.style |= LVS_REPORT;
	cs.style |= LVS_OWNERDRAWFIXED;
//	cs.style |= LVS_EDITLABELS;

	return CListView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CGridView drawing

void CGridView::OnDraw(CDC* /* pDC */)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// offsets for first and other columns
#define OFFSET_TEXT		6

void CGridView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CListCtrl& ListCtrl = GetListCtrl();
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rcItem(lpDrawItemStruct->rcItem);
	int nItem = lpDrawItemStruct->itemID;
	BOOL bFocus = (GetFocus() == this);
	static char szBuff[MAX_PATH];
	LPCTSTR pszText;


	COLORREF clrTextSave, clrBkSave;
	clrTextSave = pDC->GetTextColor();
	clrBkSave = pDC->GetBkColor();


// draw labels for columns

	BOOL bGotFocusItem = false;
	CRect rcLabel, rcFocusItem;
	ListCtrl.GetItemRect(nItem, rcItem, LVIR_BOUNDS);	// Same rect as lpDrawItemStruct

	rcItem.right = rcItem.left;		// setup for initial assignment
	
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.pszText = szBuff;
	lvi.cchTextMax = sizeof(szBuff);
	lvi.stateMask = 0xFFFF;		// get all state flags
				   
	for (int nColumn = 0; ListCtrl.GetColumn(nColumn, &lvc); nColumn++)
	{
		rcItem.left = rcItem.right;
		rcItem.right += lvc.cx;

		lvi.iItem = nItem;
		lvi.iSubItem = nColumn;
		ListCtrl.GetItem(&lvi);

		pszText = szBuff;
//		pszText = MakeShortString(pDC, szBuff,
//			rcItem.right - rcItem.left, 2*OFFSET_OTHER);

		UINT nJustify = GetDT_JUSTIFY(lvc.fmt);
		rcLabel = rcItem;
		rcLabel.left += OFFSET_TEXT;
		rcLabel.right -= OFFSET_TEXT;

	// Check for item with focus
		if (lvi.state & LVIS_FOCUSED)
		{
			bGotFocusItem = true;
			rcFocusItem = rcItem;
		}

	// Set colors based on selection status and cell color if activated
		COLORREF clrText, clrBack;
		bool bSelected = (lvi.state & LVIS_SELECTED) && (bFocus || (GetStyle() & LVS_SHOWSELALWAYS));
		bSelected = bSelected || (lvi.state & LVIS_DROPHILITED);	
		if (bSelected)
		{
			clrText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
			clrBack = ::GetSysColor(COLOR_HIGHLIGHT);
		}
		else	// not selected
		{
			clrText = clrTextSave;
			clrBack = clrBkSave;
			if (m_CellColor)		// set cell color if cell color active
			{
				CELLCOLORIDX cci = m_CellColor[nItem * m_NumColumns + nColumn];
				if (cci.text)
					clrText = m_CellPalette[cci.text];
				if (cci.back)
					clrBack = m_CellPalette[cci.back];
			}
		}

		pDC->SetTextColor(clrText);
		pDC->SetBkColor(clrBack);
//		pDC->SetBkMode(OPAQUE);
		CBrush brBkgnd(clrBack);
		pDC->FillRect(rcItem, &brBkgnd);
		pDC->DrawText(pszText, -1, rcLabel,
			nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);
	}

// draw focus rectangle if an item has focus
	if (bGotFocusItem && bFocus)
		pDC->DrawFocusRect(rcFocusItem);

// set original colors if item was selected
//	if (bSelected)
	{
        pDC->SetTextColor(clrTextSave);
		pDC->SetBkColor(clrBkSave);
	}
}

// Converts a 'List View Column' format to 'DrawText'
UINT CGridView::GetDT_JUSTIFY(UINT lvcfmt)
{
	switch(lvcfmt & LVCFMT_JUSTIFYMASK)
	{
	case LVCFMT_RIGHT:
		return DT_RIGHT;
	case LVCFMT_CENTER:
		return DT_CENTER;
	case LVCFMT_LEFT:
		return DT_LEFT;
	default:
		return 0;
	}
}




/////////////////////////////////////////////////////////////////////////////
// CGridView diagnostics

#ifdef _DEBUG
void CGridView::AssertValid() const
{
	CListView::AssertValid();
}

void CGridView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGridView message handlers


void CGridView::OnLButtonDown(UINT /*nFlags*/, CPoint point) 
{
	CListCtrl& ListCtrl = GetListCtrl();
	UINT uFlags = 0;
	int nHitItem = ListCtrl.HitTest(point, &uFlags);

	// we only get LVHT_ONITEM if on item row (in width of client)
	if (uFlags == LVHT_ONITEM)
	{
		int nCol = ColumnTest(point.x);
		if (nCol >= 0)
		{
			if ((m_iFocusItem != nHitItem) || (m_iFocusSubItem != nCol))
			{
				if (m_pItemEdit)
					OnEditEnd();
				FocusStateReset(LVIS_SELECTED | LVIS_FOCUSED);
				m_iFocusItem = nHitItem;
				m_iFocusSubItem = nCol;
				FocusStateSet(LVIS_SELECTED | LVIS_FOCUSED);
			}
		}
	}
// 	CListView::OnLButtonDown(nFlags, point);
}

void CGridView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	EditFocusItem();
	CListView::OnLButtonDblClk(nFlags, point);
}

int CGridView::ColumnTest(int x)
{
// Use column boundaries to determine cell
	LV_COLUMN lvc;
	int nCol = -1;			// remains -1 if point to left of columns
	int nColREdge = 0;
	lvc.mask = LVCF_WIDTH | LVCF_SUBITEM;
	while (nColREdge <= x)
	{
		if (!GetListCtrl().GetColumn(++nCol, &lvc))
		{
			nCol = -2;		// -2 if point to right of columns
			break;
		}
		nColREdge += lvc.cx;
	}
	return nCol;
}

void CGridView::EditFocusItem()
{
	// Send notification to parent first LVN_BEGINLABELEDIT
	LV_DISPINFO di;
	di.hdr.hwndFrom = m_hWnd;
	di.hdr.code = LVN_BEGINLABELEDIT;
	di.item.iItem = m_iFocusItem;
	di.item.iSubItem = m_iFocusSubItem;
	if (GetParent()->SendMessage(WM_NOTIFY,
		(WPARAM)0, (LPARAM)&di))
		return;			// don't edit if returns true

	ASSERT(m_pItemEdit == NULL);
	m_pItemEdit = new CItemEdit;
	CRect rcItem;
	GetListCtrl().GetItemRect(m_iFocusItem, rcItem, LVIR_BOUNDS);
	// Adjust item rect for edit box
	rcItem.top -= 2;
	rcItem.bottom += 1;
	GetColumnEdges(m_iFocusSubItem, rcItem);
	rcItem.left += OFFSET_TEXT;
	rcItem.right -= OFFSET_TEXT;

	m_pItemEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		rcItem, this, IDC_GRIDEDIT);
	m_pItemEdit->SetFont(GetFont());
	
	m_pItemEdit->SetWindowText(GetListCtrl().GetItemText(m_iFocusItem, m_iFocusSubItem));
	m_pItemEdit->SetFocus();
	m_pItemEdit->SetSel(0, -1, true);
}

void CGridView::GetColumnEdges(int nCol, CRect& rect)
{
	LV_COLUMN lvc;
	lvc.mask = LVCF_WIDTH;
	int i = 0;
	rect.left = 0;
	lvc.cx = 0;
	do {
		rect.left += lvc.cx;
		if (!GetListCtrl().GetColumn(i, &lvc))
		{
			rect.right = rect.left;
			rect.left = 0;
			return;
		}
	} while (i++ < nCol);
	rect.right = rect.left + lvc.cx;
}

void CGridView::FocusStateReset(UINT statemask)
{
	LV_ITEM lvi;
	lvi.iItem = m_iFocusItem;
	lvi.iSubItem = m_iFocusSubItem;
	lvi.mask = LVIF_STATE;
	lvi.stateMask = statemask;
	lvi.state = 0;
	GetListCtrl().SetItem(&lvi);
}

void CGridView::FocusStateSet(UINT statemask)
{
	LV_ITEM lvi;
	lvi.iItem = m_iFocusItem;
	lvi.iSubItem = m_iFocusSubItem;
	lvi.mask = LVIF_STATE;
	lvi.stateMask = statemask;
	lvi.state = statemask;
	GetListCtrl().SetItem(&lvi);
}

// key codes defined in <winuser.h> & <winresrc.h> starting line 278
void CGridView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
	case VK_LEFT:
		MoveFocus(-1, 0); break;
	case VK_RIGHT:
		MoveFocus(+1, 0); break;
	case VK_UP:
		MoveFocus(0, -1); break;
	case VK_DOWN:
		MoveFocus(0, +1); break;
	case VK_PRIOR:
		break;
	case VK_NEXT:
		break;
	case VK_RETURN:
		break;
	case VK_ESCAPE:
		break;
	default:
		CListView::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CGridView::MoveFocus(int dx, int dy)
{
	FocusStateReset(LVIS_SELECTED | LVIS_FOCUSED);
	if (dy)
	{
		m_iFocusItem += dy;
		if (m_iFocusItem < 0) m_iFocusItem = 0;
		else
		{
			int maxItem = GetListCtrl().GetItemCount() - 1;
			if (m_iFocusItem > maxItem) m_iFocusItem = maxItem;
		}
	}
	if (dx)
	{
		m_iFocusSubItem += dx;
		if (m_iFocusSubItem < 0) m_iFocusSubItem = 0;
		else
		{
			int maxSubItem = 4 - 1;
			if (m_iFocusSubItem > maxSubItem) m_iFocusSubItem = maxSubItem;
		}
	}
	FocusStateSet(LVIS_SELECTED | LVIS_FOCUSED);
}

void CGridView::OnChar(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) 
{
	if (!m_pItemEdit && (isprint(nChar) || nChar == VK_RETURN))
	{
		EditFocusItem();
		if (m_pItemEdit && isprint(nChar))
		{
			const MSG& m = *GetCurrentMessage();
			m_pItemEdit->PostMessage(m.message, m.wParam, m.lParam);
		}
	}
//	CListView::OnChar(nChar, nRepCnt, nFlags);
}



/////////////////////////////////////////////////////////////////////////////
// CGridView Edit notification handlers

void CGridView::OnEditUpdate()
{
	CRect rcEdit, rcFormat;
	m_pItemEdit->GetWindowRect(rcEdit);
	m_pItemEdit->GetRect(rcFormat);

	rcEdit.top += 0;
	rcEdit.bottom += 0;
//	m_pItemEdit->MoveWindow(rcEdit);


	char str[40];
	static int cnt;
	sprintf(str, "Updating count: %i", ++cnt);
//	m_pItemEdit->SetWindowText("Changed");
}

void CGridView::OnEditChange()
{
}

void CGridView::OnEditSetFocus()
{
}

void CGridView::OnEditKillFocus()
{
}


void CGridView::OnLVKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	*pResult = 0;
}

void CGridView::OnReturn(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	TRACE0("Got Return\n");	
	*pResult = 0;
}

void CGridView::OnEditEnd()
{
	// Send notification to parent before storing new edit text-> LVN_ENDLABELEDIT
	ASSERT(m_pItemEdit);
	CString newStr;
	m_pItemEdit->GetWindowText(newStr);

	LV_DISPINFO lvdi;
	lvdi.hdr.hwndFrom = m_hWnd;
	lvdi.hdr.code = LVN_ENDLABELEDIT;
	lvdi.item.iItem = m_iFocusItem;
	lvdi.item.iSubItem = m_iFocusSubItem;
	lvdi.item.pszText = const_cast<LPTSTR>((LPCTSTR)newStr);
	if (GetParent()->SendMessage(WM_NOTIFY, (WPARAM)0, (LPARAM)&lvdi))
		GetListCtrl().SetItemText(m_iFocusItem, m_iFocusSubItem, newStr);
	delete m_pItemEdit;
	m_pItemEdit = NULL;
}

void CGridView::OnEditCancel()
{
	// Send notification to parent before deleting edit-> LVN_ENDLABELEDIT
	LV_DISPINFO lvdi;
	lvdi.hdr.hwndFrom = m_hWnd;
	lvdi.hdr.code = LVN_ENDLABELEDIT;
	lvdi.item.iItem = m_iFocusItem;
	lvdi.item.iSubItem = m_iFocusSubItem;
	lvdi.item.pszText = NULL;
	GetParent()->SendMessage(WM_NOTIFY, (WPARAM)0, (LPARAM)&lvdi);
	delete m_pItemEdit;
	m_pItemEdit = NULL;
}

void CGridView::StoreEditValue()
{	

}

int CGridView::GetColumnCount()
{
	LV_COLUMN lvc;
	lvc.mask = LVCF_SUBITEM;
	int iCol = 0;
	while (GetListCtrl().GetColumn(iCol, &lvc))
		iCol++;
	m_NumColumns = iCol;
	return iCol;
}

//////////////////////////////////////////////////
// Cell color routines
bool CGridView::ActivateColoredCells()
{
	if (m_CellColor)
		return true;

	int iRows = GetListCtrl().GetItemCount();
	int iCols = GetColumnCount();

	m_CellColor = new CELLCOLORIDX[iRows * iCols];
	if (!m_CellColor)
		return false;

	return true;
}

void CGridView::FitFrame()
{
//	ResizeParentToFit();
//	return;

	CSize sGridFrm;
	sGridFrm.cx = 4;
	CListCtrl& ListCtrl = GetListCtrl();
	LV_COLUMN lvc;
	lvc.mask = LVCF_WIDTH;

	for (int nCol = 0; ListCtrl.GetColumn(nCol, &lvc); nCol++)
		sGridFrm.cx += lvc.cx;
	sGridFrm.cy = ListCtrl.GetItemCount() * 14 + (17 + 4);

	int cyMax = 800;
	if (sGridFrm.cy > cyMax)		// shorten if too long
	{
		sGridFrm.cy = cyMax;
		sGridFrm.cx += 18;		// room for vertical scroll bar
	}

	CRect rcPWindow, rcPClient;
	GetParent()->GetWindowRect(&rcPWindow);
	GetParent()->GetClientRect(&rcPClient);



	// ReqWindow = CurrWindow + (ReqClient - CurrClient)
	sGridFrm += rcPWindow.Size() - rcPClient.Size();
	GetParent()->SetWindowPos(NULL, 0, 0, sGridFrm.cx, sGridFrm.cy,
			SWP_NOZORDER | SWP_NOMOVE);
}



void CGridView::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	// TODO: Add your message handler code here and/or call default
	
	CListView::OnNcCalcSize(bCalcValidRects, lpncsp);
}
