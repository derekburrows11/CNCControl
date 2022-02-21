// PathView.cpp : implementation file
//

#include "stdafx.h"

#include "ThreadMessages.h"

#include "cnccontrol.h"
#include "PathDoc.h"

#include "Colors.h"


#include "PathView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




/////////////////////////////////////////////////////////////////////////////
// CPathView

IMPLEMENT_DYNCREATE(CPathView, CMoveView)

CPathView::CPathView()
{
	m_pDoc = NULL;
	m_pCurveInfoArray = NULL;
	m_pPathPosDlg = NULL;
}

CPathView::~CPathView()
{
}


BEGIN_MESSAGE_MAP(CPathView, CMoveView)
	//{{AFX_MSG_MAP(CPathView)
	ON_COMMAND(ID_PATHCOLORCURVE, OnPathColorCurve)
	ON_UPDATE_COMMAND_UI(ID_PATHCOLORCURVE, OnUpdatePathColorCurve)
	ON_COMMAND(ID_PATHCOLORFEEDRATE, OnPathColorFeedrate)
	ON_UPDATE_COMMAND_UI(ID_PATHCOLORFEEDRATE, OnUpdatePathColorFeedrate)
	ON_COMMAND(ID_PATHSHOWFITTED, OnPathShowFitted)
	ON_UPDATE_COMMAND_UI(ID_PATHSHOWFITTED, OnUpdatePathShowFitted)
	ON_COMMAND(ID_PATHSHOWORIG, OnPathShowOrig)
	ON_UPDATE_COMMAND_UI(ID_PATHSHOWORIG, OnUpdatePathShowOrig)
	ON_COMMAND(ID_SHOWENDNODES, OnShowEndNodes)
	ON_UPDATE_COMMAND_UI(ID_SHOWENDNODES, OnUpdateShowEndNodes)
	ON_COMMAND(ID_SHOWCTRLNODES, OnShowCtrlNodes)
	ON_UPDATE_COMMAND_UI(ID_SHOWCTRLNODES, OnUpdateShowCtrlNodes)
	ON_COMMAND(ID_SHOWCTRLNODELINES, OnShowCtrlNodeLines)
	ON_UPDATE_COMMAND_UI(ID_SHOWCTRLNODELINES, OnUpdateShowCtrlNodeLines)
	ON_COMMAND(ID_SHOWBEZIERS, OnShowBeziers)
	ON_UPDATE_COMMAND_UI(ID_SHOWBEZIERS, OnUpdateShowBeziers)
	ON_COMMAND(ID_VIEW_ZOOM11, OnViewZoom11)
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomIn)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomOut)
	ON_COMMAND(ID_VIEW_ZOOMALL, OnViewZoomAll)
	ON_COMMAND(ID_VIEW_ISOSCALE, OnViewIsoScale)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ISOSCALE, OnUpdateViewIsoScale)
	ON_COMMAND(ID_VIEW_SHOWREGION, OnViewRegionBox)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWREGION, OnUpdateViewRegionBox)
	ON_COMMAND(ID_VIEWDIR_X, OnViewDirSetToX)
	ON_COMMAND(ID_VIEWDIR_Y, OnViewDirSetToY)
	ON_COMMAND(ID_VIEWDIR_Z, OnViewDirSetToZ)
	ON_COMMAND(ID_VIEWDIR_ISO, OnViewDirSetToIso)
	ON_COMMAND(ID_VIEWDIR_FLIP, OnViewDirFlip)
	ON_COMMAND(ID_OPTIONS_SCROLLBARS, OnOptionsScrollBars)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SCROLLBARS, OnUpdateOptionsScrollBars)
	ON_WM_CHAR()

	//}}AFX_MSG_MAP

	ON_COMMAND(DLGN_VALUECHANGED, OnPathPosDlgChangeCom)		// doesn't get called
	ON_CONTROL(DLGN_VALUECHANGED, CPathPositionDlg::IDD, OnPathPosDlgChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPathView diagnostics

#ifdef _DEBUG
void CPathView::AssertValid() const
{
	CMoveView::AssertValid();
}

void CPathView::Dump(CDumpContext& dc) const
{
	CMoveView::Dump(dc);
}
#endif //_DEBUG

inline CPathDoc* CPathView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPathDoc)));
	return (CPathDoc*)m_pDocument;
}


/////////////////////////////////////////////////////////////////////////////
// CPathView drawing

void CPathView::OnInitialUpdate()
{
	m_bIsoScaling = true;
	m_b3DView = true;

	m_nShowNodes = SNF_BEZIER;
	m_nShowSegTypes = STF_ORIGIONAL;

	m_nPathColorFrom = PC_FEEDRATE;
//	m_nPathColorFrom = PC_SEGTYPE;
	
	m_iFirstSegToDraw = 0;
	m_nrFirstSegToDraw = 0;		// guess, set during draw
	m_bUseNR = false;
	m_numSegsToDraw = 0;			// set in OnUpdate()


	CMoveView::OnInitialUpdate();

	OnViewDirSetToIso();
	FitViewRegionToWindow();

	CPathDoc* pDoc = GetDocument();
//	pDoc->SetTitlePlusTool();			// open doc doesn't give chance to set title
	if (!pDoc->GetToolName().IsEmpty())
	{
		CString strTitle = pDoc->GetTitle();		// title set to file name when this function returns!
		strTitle += " - Tool: ";
		strTitle += pDoc->GetToolName();
		GetParentFrame()->ModifyStyle(FWS_ADDTOTITLE , 0);
		GetParentFrame()->SetWindowText(strTitle);
	}



	AfxGetMainWnd()->SendMessage(WMU_SETACTIVEPATHVIEW, (WPARAM)this, (LPARAM)&m_pPathPosDlg);	// will set m_pPathPosDlg
	// activate CPathDisplayDlg (or toolbar) if required
	
//	if (m_pPathPosDlg == NULL || m_pPathPosDlg->m_hWnd == NULL || !(m_pPathPosDlg->GetStyle() & WS_VISIBLE))
//		AfxGetMainWnd()->SendMessage(WM_COMMAND, IDW_PATHPOSITION_BAR);			// Activate PathPosDlg if not already
	UpdatePathPosDlg();
}

void CPathView::OnPathPosDlgChangeCom()	// doesn't get called
{
	OnPathPosDlgChange();
	ASSERT(0);
}

void CPathView::OnPathPosDlgChange()
{
	if (GetPathPosDlgData())
		Invalidate();
}

void CPathView::UpdatePathPosDlg()
{
	if (m_pPathPosDlg == NULL)
		return;
	SetPathPosDlgData();
	if (m_pPathPosDlg->m_hWnd)
		m_pPathPosDlg->UpdateData(false);
}

void CPathView::SetPathPosDlgData()
{
	if (m_pPathPosDlg == NULL)
		return;
	m_pPathPosDlg->m_iNumSegsPath = GetDocument()->GetNumSegments();
	m_pPathPosDlg->m_iStartSeg = m_iFirstSegToDraw;
	m_pPathPosDlg->m_iNumSegsShow = m_numSegsToDraw;
	m_pPathPosDlg->m_nrStartSeg = m_nrFirstSegToDraw;
}

bool CPathView::GetPathPosDlgData()		// returns true if values are changed
{
	if (m_pPathPosDlg == NULL)
		return false;
	bool bChanged = false;
	if (m_iFirstSegToDraw != m_pPathPosDlg->m_iStartSeg
		|| m_numSegsToDraw != m_pPathPosDlg->m_iNumSegsShow
		|| m_nrFirstSegToDraw != m_pPathPosDlg->m_nrStartSeg)
		bChanged = true;

	m_bUseNR = m_pPathPosDlg->m_bUseNR;
	m_nrFirstSegToDraw = m_pPathPosDlg->m_nrStartSeg;
	m_iFirstSegToDraw = m_pPathPosDlg->m_iStartSeg;
	m_numSegsToDraw = m_pPathPosDlg->m_iNumSegsShow;
	return bChanged;
}

void CPathView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/) 
{
	CPathDoc* pDoc = GetDocument();
	CVector& minVal = pDoc->GetMinNodeValues();
	CVector& maxVal = pDoc->GetMaxNodeValues();
	SetViewRegion(minVal, maxVal);

	if (m_numSegsToDraw == 0)		// show all if nothing currently
	{
		m_iFirstSegToDraw = 0;
		m_nrFirstSegToDraw = 0;		// guess, set during draw
		m_bUseNR = false;
		m_numSegsToDraw = pDoc->GetNumSegments();		// show all
		if (m_numSegsToDraw > 2000)		// limit
			m_numSegsToDraw = 2000;
	}

	m_pCurveInfoArray = &pDoc->m_CurveInfoArray;
	FitViewRegionToWindow();
	Invalidate();
}


void CPathView::OnDraw(CDC* pDC)
{
	DrawAxesTriple(pDC);
	DrawPath(pDC);
	DrawRegionBox(pDC);
}


void CPathView::DrawPath(CDC* pDC)
{
#define MAXPOINTS 2048
	CPathDoc* pDoc = GetDocument();
	m_pDoc = pDoc;
	ASSERT_VALID(pDoc);

	SetViewportOrgToFocus(pDC);
	pDC->SetWindowOrg(0,0);

//	const double xReal2Log = m_real2log.x;		// a local copy - maybe faster?
//	const double yReal2Log = m_real2log.y;
//	const double xFocusReal = m_ptFocusReal.x;
//	const double yFocusReal = m_ptFocusReal.y;
	const CVector vtFocusReal = m_ptFocusReal;

//	const double maxRealCoordX = m_log2real.x * m_maxLogicalCoord;		// 8192 pixels (~5 display widths) either side
//	const double maxRealCoordY = fabs(m_log2real.y) * m_maxLogicalCoord;

	CPen penPathNorm(PS_SOLID, 0, YELLOW);
	CPen penPathRapid(PS_SOLID, 0, RED);
	CPen penPathFitted(PS_SOLID, 0, CYAN);
	CPen penFittedCurve(PS_SOLID, 0, LIGHTBLUE);
	CPen penControls(PS_SOLID, 0, GREEN);
	CPen* pPenPath;

	CPen* arpPenFeedRate[2] = {&penPathNorm, &penPathRapid};
	CPen* arpPenSegType[2] = {&penPathNorm, &penPathFitted};
	CPen* pPenOrig = pDC->GetCurrentPen();
/*
	static HPEN hPenPrev = 0;
	if (hPenPrev != penPathNorm)
	{
		hPenPrev = penPathNorm;
		TRACE1("PathView::DrawPath() New Handle to PathNorm Pen(p): %#.8x\n", (HPEN)penPathNorm);
	}
*/
	NODEREF& nr = m_SegTypeNext.nrEndNode;
	CPathNode* pNode = pDoc->GetFirstPathNode(nr);
	if (pNode == NULL)
		return;		// no nodes
	nr--;			// set it before first node
	// initialisation required for ReadNextNode(nr)
	m_SegTypeNext.nLineStyle = LS_START;
	m_SegTypeNext.nFeedRate = 0;				// default
	m_SegTypeNext.bForCurveFit = false;
	m_SegTypeNext.nEndNodeType = 0;				// PNT_ENDNODE will inc seg count
	m_idxCIA = 0;		// index to m_pCurveInfoArray
	if (m_pCurveInfoArray == NULL || m_pCurveInfoArray->GetSize() == 0)
		m_nrNextFitCurveExtent = NODEREF_MAX;
	else
		m_nrNextFitCurveExtent = m_pCurveInfoArray->ElementAt(m_idxCIA).nrStart;
	ReadNextNode();		// reads to first PNTF_NODE

	CVect2 TrxNode;
	CPoint arPathPts[MAXPOINTS];
	int nPtCount = 0;

	int numSegAfterNext = 0;
	while (1)	// seek to first segment to draw
	{
		if ( (m_bUseNR && (nr >= m_nrFirstSegToDraw))
			|| (!m_bUseNR && (numSegAfterNext >= m_iFirstSegToDraw)) )
			break;
		do 
		{
			ReadNextNode();
		} while ((m_SegTypeNext.nEndNodeType & PNTF_END) == 0);	// refers to seg after m_SegTypeNext
		numSegAfterNext++;
	}
	if (m_nrFirstSegToDraw != nr || m_iFirstSegToDraw != numSegAfterNext)
	{
		m_nrFirstSegToDraw = nr;
		m_iFirstSegToDraw = numSegAfterNext;
		UpdatePathPosDlg();
	}
	m_numSegNext = -1;		// to get correct segment count

	bool bDraw = false;
	while (1)
	{
/*
	to convert World point to screen coord:
	ptA = mxViewTransform * (ptWorld - ptFocusReal)
	ptB = ptA - Log16Orig
	check ptB relative to screen
	ptScreen = NEARINT(Real2Log * ptB)
*/
		pNode = pDoc->GetNodeAbs(nr);		// may set a single CPathNode member if relative
		TrxNode.ProdPart(m_mxViewTransform2Log, (*pNode - vtFocusReal));
		arPathPts[nPtCount].x = NEARINT(TrxNode.x);
		arPathPts[nPtCount].y = NEARINT(TrxNode.y);
		nPtCount++;

		ReadNextNode();		// reads to next PNTF_NODE

		if (m_numSegNext >= m_numSegsToDraw)
		{
			bDraw = true;
			m_SegTypeNext.nLineStyle = LS_FINISH;		// make loop finish
		}
		if (nPtCount >= MAXPOINTS - 3 && m_SegTypeCurr.nEndNodeType & PNTF_END)
			bDraw = true;
		if (m_nSegChange != 0)
			bDraw = true;

		if (bDraw)
		{
			bDraw = false;
			if ((!m_SegTypeCurr.bForCurveFit || m_nShowSegTypes & STF_ORIGIONAL) && nPtCount > 1)		// is sometimes 1 at start?
			{
				// set color depending on feedrate, bInCurve, or...
				if (m_nPathColorFrom == PC_FEEDRATE)
					pPenPath = arpPenFeedRate[m_SegTypeCurr.nFeedRate];
				else if (m_nPathColorFrom == PC_SEGTYPE)
					pPenPath = arpPenSegType[m_SegTypeCurr.bForCurveFit ? 1:0];

				if (m_SegTypeCurr.nLineStyle == LS_STRAIGHT)
				{
					pDC->SelectObject(pPenPath);
					VERIFY(pDC->Polyline(arPathPts, nPtCount));
				}
				else if (m_SegTypeCurr.nLineStyle == LS_BEZIER)
				{
					if (m_nShowNodes & SNF_CONTROLLINE)		// if showing control nodes
					{
						pDC->SelectObject(&penControls);
						VERIFY(pDC->Polyline(arPathPts, nPtCount));
					}
					if (m_nShowNodes & SNF_BEZIER)
					{
						pDC->SelectObject(pPenPath);
						VERIFY(pDC->PolyBezier(arPathPts, nPtCount));
					}
				}
			}
			if (m_SegTypeNext.nLineStyle == LS_FINISH)
				break;
			arPathPts[0] = arPathPts[nPtCount - 1];
			nPtCount = 1;
		}
	}	// while (1)
	m_nrLastSegToDraw = m_SegTypeCurr.nrEndNode;		// last node of last segment


/////////////////////
// draw any fitted curve beziers in path drawing range


	NODEREF nrBez = 0;
	int sizeBezArray = pDoc->m_FittedBezierArray.GetSize();
	if (!(m_nShowSegTypes & STF_FITTEDCURVE))
		sizeBezArray = 0;		// won't show curves
	do
	{
		if (nrBez >= sizeBezArray)
			break;
		pNode = &pDoc->m_FittedBezierArray.ElementAt(nrBez++);
	}	while (pNode->type != PNT_BREAKPATH || (int)pNode->x < m_nrFirstSegToDraw);

	nPtCount = 0;
	while (nrBez < sizeBezArray)
	{
		if (pNode->type == PNT_BREAKPATH && (int)pNode->x >= m_nrLastSegToDraw)
			break;
		while (nrBez < sizeBezArray)
		{
			pNode = &pDoc->m_FittedBezierArray.ElementAt(nrBez++);
			if (!(pNode->type & PNTF_NODE))
				break;
			TrxNode.ProdPart(m_mxViewTransform2Log, (*pNode - vtFocusReal));
			arPathPts[nPtCount].x = NEARINT(TrxNode.x);
			arPathPts[nPtCount].y = NEARINT(TrxNode.y);
			nPtCount++;
			if (nPtCount >= MAXPOINTS - 3 && pNode->type & PNTF_END)
				break;
		}
		if (nPtCount != 0)
		{
			if (m_nShowNodes & SNF_CONTROLLINE)		// if showing control nodes
			{
				pDC->SelectObject(&penControls);
				VERIFY(pDC->Polyline(arPathPts, nPtCount));
			}
			if (m_nShowNodes & SNF_BEZIER)
			{
				pDC->SelectObject(&penFittedCurve);
				VERIFY(pDC->PolyBezier(arPathPts, nPtCount));
			}
		}
		if (pNode->type & PNTF_NODE)		// stopped due to max points
		{
			arPathPts[0] = arPathPts[nPtCount - 1];
			nPtCount = 1;
		}
		else
			nPtCount = 0;
	}


	pDC->SelectObject(pPenOrig);
}


void CPathView::ReadNextNode()
{
	// reads up to the next PNTF_NODE type and sets any path type or feed rate change flags
	CPathNode* pNode;
	m_SegTypeCurr = m_SegTypeNext;
	m_nSegChange = 0;		// segment change flags
	int nType;
	while (1)
	{
		pNode = m_pDoc->GetNextPathNode(m_SegTypeNext.nrEndNode);
		if (!pNode)
		{
			m_SegTypeNext.nLineStyle = LS_FINISH;
			m_nSegChange |= SCF_LINESTYLE;
			return;
		}
		nType = pNode->type;
		if (nType & PNTF_NODE)
			break;
		if (nType == PNT_SETFEEDRATE)
			if (pNode->x == -1)		// for rapid feed
				m_SegTypeNext.nFeedRate = 1;
			else
				m_SegTypeNext.nFeedRate = 0;
		m_nSegChange |= SCF_FEEDRATE;		// segment change flags
	}	// while (1) - breaks out with a PNTF_NODE type

	m_SegTypeNext.nEndNodeType = nType;
	if ((nType & PNTF_END) && (m_SegTypeCurr.nEndNodeType & PNTF_END))
		m_SegTypeNext.nLineStyle = LS_STRAIGHT;
	else
		m_SegTypeNext.nLineStyle = LS_BEZIER;
	if (m_SegTypeNext.nLineStyle != m_SegTypeCurr.nLineStyle)
		m_nSegChange |= SCF_LINESTYLE;					// segment change flags
	if (m_SegTypeCurr.nEndNodeType & PNTF_END)		// refers to seg in m_SegTypeNext
		m_numSegNext++;

	// check for start/end of fitted curve
	ASSERT(m_SegTypeCurr.nrEndNode <= m_nrNextFitCurveExtent);		// should not be past!
	while (m_SegTypeCurr.nrEndNode >= m_nrNextFitCurveExtent)		// repeat if end/start's are same
		if (m_SegTypeCurr.bForCurveFit)
		{
			if (++m_idxCIA < m_pCurveInfoArray->GetSize())
				m_nrNextFitCurveExtent = m_pCurveInfoArray->ElementAt(m_idxCIA).nrStart;
			else
				m_nrNextFitCurveExtent = NODEREF_MAX;
			m_SegTypeNext.bForCurveFit = false;		// unless next nrStart is this nr
			m_nSegChange |= SCF_SEGTYPE;		// segment change flags
		}
		else
		{
			m_nrNextFitCurveExtent = m_pCurveInfoArray->ElementAt(m_idxCIA).nrEnd;
			m_SegTypeNext.bForCurveFit = true;
			m_nSegChange |= SCF_SEGTYPE;		// segment change flags
		}
}



/////////////////////////////////////////////////////////////////////////////
// CPathView message handlers

void CPathView::OnShowEndNodes()			// ID_SHOWENDNODES
{
	m_nShowNodes ^= SNF_END;
	Invalidate();
}
void CPathView::OnShowCtrlNodes()		// ID_SHOWCTRLNODES
{
	m_nShowNodes ^= SNF_CONTROL;
	Invalidate();
}
void CPathView::OnShowCtrlNodeLines()	// ID_SHOWCTRLNODELINES
{
	m_nShowNodes ^= SNF_CONTROLLINE;
	Invalidate();
}
void CPathView::OnShowBeziers()			// ID_SHOWBEZIERS
{
	m_nShowNodes ^= SNF_BEZIER;
	Invalidate();
}
void CPathView::OnUpdateShowEndNodes(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_nShowNodes & SNF_END) != 0);
}
void CPathView::OnUpdateShowCtrlNodes(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_nShowNodes & SNF_CONTROL) != 0);
}
void CPathView::OnUpdateShowCtrlNodeLines(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_nShowNodes & SNF_CONTROLLINE) != 0);
}
void CPathView::OnUpdateShowBeziers(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_nShowNodes & SNF_BEZIER) != 0);
}


void CPathView::OnPathColorCurve() 
{
	m_nPathColorFrom = PC_SEGTYPE;
	Invalidate();
}
void CPathView::OnUpdatePathColorCurve(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nPathColorFrom == PC_SEGTYPE);
}

void CPathView::OnPathColorFeedrate() 
{
	m_nPathColorFrom = PC_FEEDRATE;
	Invalidate();
}
void CPathView::OnUpdatePathColorFeedrate(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nPathColorFrom == PC_FEEDRATE);
}


void CPathView::OnPathShowFitted() 
{
	m_nShowSegTypes ^= STF_FITTEDCURVE;	
	Invalidate();
}
void CPathView::OnUpdatePathShowFitted(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_nShowSegTypes & STF_FITTEDCURVE) != 0);
}

void CPathView::OnPathShowOrig() 
{
	m_nShowSegTypes ^= STF_ORIGIONAL;	
	Invalidate();
}
void CPathView::OnUpdatePathShowOrig(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_nShowSegTypes & STF_ORIGIONAL) != 0);
}


void CPathView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
//	m_PathPosDlg
	CMoveView::OnChar(nChar, nRepCnt, nFlags);
}



void CPathView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CMoveView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	// Let MainFrame know about change so it can change info in any tool windows showing current view
	if (bActivate)
	{
		AfxGetMainWnd()->SendMessage(WMU_SETACTIVEPATHVIEW, (WPARAM)this, (LPARAM)&m_pPathPosDlg);	// will set m_pPathPosDlg
		UpdatePathPosDlg();
	}
}
