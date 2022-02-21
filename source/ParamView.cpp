// ParamView.cpp : implementation file
//

#include "stdafx.h"
#include "CNCControl.h"

//#include <afxcview.h>

#include "CNCComms.h"
#include "GridView.h"
#include "ParamDoc.h"
#include "ParamView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParamView

IMPLEMENT_DYNCREATE(CParamView, CGridView)

CParamView::CParamView()
{
	m_nViewType = VT_NONE;
}

CParamView::~CParamView()
{
}


BEGIN_MESSAGE_MAP(CParamView, CGridView)
	//{{AFX_MSG_MAP(CParamView)
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_COMMAND(ID_MACHINE_SENDPARAMETER, OnSendParam)
	ON_COMMAND(ID_MACHINE_REQUESTPARAMETER, OnRequestParam)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParamView drawing

void CParamView::OnDraw(CDC*)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CParamView diagnostics

#ifdef _DEBUG
void CParamView::AssertValid() const
{
	CGridView::AssertValid();
}

void CParamView::Dump(CDumpContext& dc) const
{
	CGridView::Dump(dc);
}

CParamDoc* CParamView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CParamDoc)));
	return (CParamDoc*)m_pDocument;
}
#endif //_DEBUG

#ifndef _DEBUG  // debug version defined above
inline CParamDoc* CParamView::GetDocument()
   { return (CParamDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
// CParamView message handlers

void CParamView::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	TRACE0("Starting Param Label Edit\n");
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM& lvi = pDispInfo->item;
	*pResult = 0;			// allow editing

//	DWORD lItemCode = GetListCtrl().GetItemData(lvi.iItem);
//	int code = lItemCode & PSY_CODEMASK;
//	int axis = 0;
//	if (lItemCode & PSY_AXIS)
//		axis = lvi.iSubItem - m_nAxis0SubItem + 1;
	if (lvi.iSubItem == 0)
		*pResult = 1;		// do not allow editing of parameter name
}

void CParamView::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	TRACE0("Ending Param Label Edit\n");	

	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM& lvi = pDispInfo->item;
	*pResult = 0;			// reject new value default
	if (!lvi.pszText)		// if edit was canceled
		return;

	DWORD nItemCode = GetListCtrl().GetItemData(lvi.iItem);
	int code = nItemCode & PSY_CODEMASK;
	int axis = lvi.iSubItem - m_nAxis0SubItem + 1;
	if (nItemCode & PSY_AXIS)
	{
		if (axis < 1 || axis > GetDocument()->m_nNumAxis)
			axis = -1;
	}
	else if (nItemCode & PSY_GENERAL)
	{
		if (axis-- != 1)		// changes 1 to 0 and all others to -1
			axis = -1;
	}
	else axis = -1;

	int val;
	if ((axis != -1) && (sscanf(lvi.pszText, "%i", &val) == 1))
	{
		GetDocument()->SetParam((axis << 8) | code, val);
		*pResult = 1;		// set to new value
	}
}




/////////////////////////////////////////////////////////////////////////////
// data for the list view control

char* _szAxisColLabel[] =
{ "Parameter", "Axis 1 (x)", "Axis 2 (y)", "Axis 3 (z)", NULL };

int _nAxisColFormat[] = 
{ LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_RIGHT };

int _nAxisColWidth[] = 
{ 100, 80, 80, 80 };

char* _szGenColLabel[] =
{ "Parameter", "Value", NULL };

int _nGenColFormat[] = 
{ LVCFMT_RIGHT, LVCFMT_RIGHT };

int _nGenColWidth[] = 
{ 120, 80 };

enum {
	CIDC_BLACK = 0,
	CIDX_UNKNOWN,
	CIDX_READY,
	CIDX_NVSET,
	CIDX_SENDQUEUED,
	CIDX_REQUESTQUEUED,
	CIDX_SENDERROR,
	CIDX_REQUESTERROR,
};

/////////////////////////////////////////////////////////////////////////////
// CParamView initialization

void CParamView::OnInitialUpdate() 
{
	CGridView::OnInitialUpdate();
	CListCtrl& ListCtrl = GetListCtrl();
	CParamDoc& doc = *GetDocument();

	// Set window frame title   FWS_ADDTOTITLE style is removed from frame so doesn't try to show doc name!
	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame != NULL)
		if (m_nViewType == VT_GENPARAM)			// general parameter view
			pFrame->SetWindowText("General Parameters");
		else if (m_nViewType == VT_AXISPARAM)		// axis parameter view
			pFrame->SetWindowText("Axis Parameters");

// insert columns
// axis headers if any axis parameters in view or else general headers

	m_nAxis0SubItem = 1;

//	CPoint wndPos;
	char** szColLabel;
	int* nColFmt;
	int* nColWidth;
	int nMaxPar;
	if (m_nViewType == VT_GENPARAM)			// general parameter view
	{
		szColLabel = _szGenColLabel;
		nColFmt = _nGenColFormat;
		nColWidth = _nGenColWidth;
		nMaxPar = doc.m_nMaxGeneralPar;
//		wndPos.x = 0; wndPos.y = 10;
	}
	else if (m_nViewType == VT_AXISPARAM)		// axis parameter view
	{
		szColLabel = _szAxisColLabel;
		nColFmt = _nAxisColFormat;
		nColWidth = _nAxisColWidth;
		nMaxPar = doc.m_nMaxAxisPar;
//		wndPos.x = 240; wndPos.y = 10;
	}
	else		// view not established
	{
		nMaxPar = 0;
		return;
	}
//	GetParent()->SetWindowPos(NULL, wndPos.x, wndPos.y, 0, 0,
//			SWP_NOZORDER | SWP_NOSIZE);

	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	int i = 0;
	while (szColLabel[i])
	{
		lvc.iSubItem = i;
		lvc.pszText = szColLabel[i];
		lvc.cx = nColWidth[i];
		lvc.fmt = nColFmt[i];
		ListCtrl.InsertColumn(i, &lvc);
		i++;
	}

// insert items
	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iSubItem = 0;

	int row = 0;
	for (int code = 0; code <= nMaxPar; code++)
	{
		if (m_nViewType == VT_GENPARAM)
		{
			lvi.pszText = doc.m_GeneralPar[code].name;
			lvi.lParam = code | PSY_GENERAL;
		}
		else if (m_nViewType == VT_AXISPARAM)
		{
			lvi.pszText = doc.m_AxisPar[code][0].name;
			lvi.lParam = code | PSY_AXIS;
		}
		if (lvi.pszText)
		{
			lvi.iItem = row++;
			ListCtrl.InsertItem(&lvi);
		}
	}

	ActivateColoredCells();
// Set state colors
	m_CellPalette[CIDC_BLACK]				= RGB(0, 0, 0);
	m_CellPalette[CIDX_UNKNOWN]			= RGB(255, 200, 200);
	m_CellPalette[CIDX_READY]				= RGB(200, 255, 200);

	m_CellPalette[CIDX_NVSET]				= RGB(200, 200, 255);
	m_CellPalette[CIDX_SENDQUEUED]		= RGB(255, 255, 200);
	m_CellPalette[CIDX_REQUESTQUEUED]	= RGB(200, 255, 255);
	m_CellPalette[CIDX_SENDERROR]			= RGB(255, 0, 150);
	m_CellPalette[CIDX_REQUESTERROR]		= RGB(255, 150, 0);

// Update item values
	int nNumItems = ListCtrl.GetItemCount();
	for (i = 0; i < nNumItems; i++)
		UpdateItemValue(i);		// update all axes

//	FitFrame();		//  fit frame to grid after its setup

}

void CParamView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/) 
{
//	TRACE2("CParamView::OnUpdate(): %p, hint: %lx\n", this, lHint);
	switch (lHint & HINT_MASK)
	{
	case 0:
		Invalidate();		// Generic update was sent
		break;
	case HINT_AXPAR:
	{
		CAxPar axpar = LOWORD(lHint);
		// find item index of par (if par is in list)
		CListCtrl& ListCtrl = GetListCtrl();
		LV_FINDINFO lvfi;
		lvfi.flags = LVFI_PARAM;
		lvfi.lParam = axpar.Param();
		if (axpar.Axis())			// if an axis param set PSY_AXIS else PSY_GENERAL
			lvfi.lParam |= PSY_AXIS;
		else
			lvfi.lParam |= PSY_GENERAL;
		int idxItem = ListCtrl.FindItem(&lvfi);
		if (idxItem != -1)
		{
			UpdateItemValue(idxItem);
	//		ListCtrl.Update(idxItem);		// gets updated anyway!
		}
		break;
	}
	case HINT_MACHSTATE:		// ignore
		break;
	case HINT_MACHBUFF:		// ignore
		break;
	default:
		Invalidate();		// Generic update was sent
	}
}

// Updates item text and cell color in given row index.
// If an axis variable, axis == -1 updates all axes
void CParamView::UpdateItemValue(int idx, int axis /* = -1 */)		// given row index
{
	CListCtrl& ListCtrl = GetListCtrl();
	CParamDoc& pdata = *GetDocument();

	int iCode = ListCtrl.GetItemData(idx);
	bool bAxisPar = (iCode & PSY_AXIS) != 0;
	iCode &= PSY_CODEMASK;			// take only code number

	if (!bAxisPar) axis = 0;
	int ax0, ax1;
	if (axis == -1)
		{	ax0 = 0; ax1 = pdata.m_nNumAxis - 1; }
	else
		{ ax0 = axis; ax1 = axis; }

	CParam* pPar;
	for (int ax = ax0; ax <= ax1; ax++)
	{
		if (bAxisPar)
			pPar = &pdata.m_AxisPar[iCode][ax];
		else
			pPar = &pdata.m_GeneralPar[iCode];

		int st = pPar->state;
	// set cell color index from state
		if (m_CellColor)
		{
			char idxClr = CIDC_BLACK;
			if (st & PSAB_READY)
			{	if (st & PSAB_UNKNOWN) idxClr = CIDX_UNKNOWN;
				else if (st & PSAB_MODIFIED) idxClr = CIDX_NVSET;
				else idxClr = CIDX_READY;
			}
			else if (st & PSAB_UNKNOWN) idxClr = CIDX_UNKNOWN;
			else if (st & PSAB_ERROR)
			{	if (st & PSAB_SEND) idxClr = CIDX_SENDERROR;
				else if (st & PSAB_REQUEST) idxClr = CIDX_REQUESTERROR;
			}
			else if (st & PSAB_SEND) idxClr = CIDX_SENDQUEUED;
			else if (st & PSAB_REQUEST) idxClr = CIDX_REQUESTQUEUED;
			m_CellColor[idx * m_NumColumns + ax + m_nAxis0SubItem].back = idxClr;
		}

		int iVal = pPar->value;
		if (st & PSAB_MODIFIED)
			iVal = pPar->sendValue;
		char szVal[50];
		if ((st & PSAB_UNKNOWN) && !(st & PSAB_MODIFIED))
			strcpy(szVal, "??");
		else
		{
			switch (pPar->style & (PSY_FORMATMASK | PSY_UNSIGNED))
			{
			case PSY_DEC:
			case 0:
				sprintf(szVal, "%i", iVal);		// signed dec
				break;
			case PSY_DEC | PSY_UNSIGNED:
			case PSY_UNSIGNED:
				sprintf(szVal, "%u", iVal);		// unsigned dec
				break;
			case PSY_HEX:
				if (iVal < 0)
					sprintf(szVal, "-0x%02x", -iVal);	// signed negative hex
				else
					sprintf(szVal, "0x%02x", iVal);		// signed positive hex
				break;
			case PSY_HEX | PSY_UNSIGNED:
				sprintf(szVal, "0x%02x", iVal);			// unsigned hex
				break;
	//		case PSY_BOOL_EN:
	//			break;
			default:
				sprintf(szVal, "%i", iVal);		// default is signed dec
			}
		}

		ListCtrl.SetItemText(idx, ax + m_nAxis0SubItem, szVal);
	}

}





void CParamView::OnSendParam() 
{
	if (m_iFocusItem < 0)
		return;
	DWORD lItemCode = GetListCtrl().GetItemData(m_iFocusItem);
	int code = lItemCode & PSY_CODEMASK;
	int axis = 0;
	if (lItemCode & PSY_AXIS)
		axis = m_iFocusSubItem - m_nAxis0SubItem + 1;
	GetDocument()->SendParam((axis << 8) | code);
}

void CParamView::OnRequestParam() 
{
	if (m_iFocusItem < 0)
		return;
	DWORD lItemCode = GetListCtrl().GetItemData(m_iFocusItem);
	int code = lItemCode & PSY_CODEMASK;
	int axis = 0;
	if (lItemCode & PSY_AXIS)
		axis = m_iFocusSubItem - m_nAxis0SubItem + 1;
	GetDocument()->RequestParam((axis << 8) | code);
}

void CParamView::SetViewType(int nViewType)
{
	m_nViewType = (enum VIEWTYPE) nViewType;
}



BOOL CParamView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class

	return CGridView::PreCreateWindow(cs);
}
