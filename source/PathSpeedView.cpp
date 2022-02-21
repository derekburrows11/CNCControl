// PathSpeedView.cpp: implementation of the CPathSpeedView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"
#include "PathSpeedDoc.h"

#include "Colors.h"

#include "PathSpeedView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPathSpeedView

IMPLEMENT_DYNCREATE(CPathSpeedView, CMoveView)

CPathSpeedView::CPathSpeedView()
{
}

CPathSpeedView::~CPathSpeedView()
{
}


BEGIN_MESSAGE_MAP(CPathSpeedView, CMoveView)
	//{{AFX_MSG_MAP(CPathSpeedView)
	ON_COMMAND(ID_VIEW_ZOOM11, OnViewZoom11)
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomIn)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomOut)
	ON_COMMAND(ID_VIEW_ZOOMALL, OnViewZoomAll)
	ON_COMMAND(ID_VIEW_STORE_LOC, OnViewStoreLocation)
	ON_COMMAND(ID_VIEW_SET_LOC, OnViewSetLocation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SET_LOC, OnUpdateViewSetLocation)
	//}}AFX_MSG_MAP

	ON_COMMAND_RANGE(ID_SHOWX, ID_SHOWZ, OnShowAxis)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SHOWX, ID_SHOWZ, OnUpdateShowAxis)
	ON_COMMAND_RANGE(ID_SHOWPOS, ID_SHOWJERK, OnShowPVA)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SHOWPOS, ID_SHOWJERK, OnUpdateShowPVA)
	ON_COMMAND_RANGE(ID_SHOWREQUESTEDSEGS, ID_SHOWACTUALreqSEGS, OnShowSegs)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SHOWREQUESTEDSEGS, ID_SHOWACTUALreqSEGS, OnUpdateShowSegs)
	ON_COMMAND_RANGE(ID_SHOWENDNODES, ID_SHOWCTRLNODES, OnShowNodes)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SHOWENDNODES, ID_SHOWCTRLNODES, OnUpdateShowNodes)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPathSpeedView diagnostics

#ifdef _DEBUG
void CPathSpeedView::AssertValid() const
{
	CMoveView::AssertValid();
}

void CPathSpeedView::Dump(CDumpContext& dc) const
{
	CMoveView::Dump(dc);
}

CPathSpeedDoc* CPathSpeedView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPathSpeedDoc)));
	return (CPathSpeedDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CPathSpeedView updateing

void CPathSpeedView::OnInitialUpdate()
{
	CMoveView::OnInitialUpdate();

	m_nShowAxis = 3;		// show x, y, z axis flags (b0-b2)
	m_nShowPVA = 6;		// show pos, vel, acc, jerk flags (b0-b3)
	m_nShowSeg = 1;		// show desired (b0), actual (b1) segment values
	m_nShowNodes = 0;		// show end (b0), control (b1) nodes

	FitViewRegionToWindow();

}

void CPathSpeedView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/) 
{
	// set region
	CPathSpeedDoc* pDoc = GetDocument();
	if (pDoc->m_bArrayExtentsSet)
	{
		SMotionStateBasic& msMin = pDoc->m_MinArrayVals;
		SMotionStateBasic& msMax = pDoc->m_MaxArrayVals;
		CVector vtMin = msMin.vtPVAJ[0];
		CVector vtMax = msMax.vtPVAJ[0];
		for (int i = 1; i < 3; i++)		// don't include jerk!
		{
			vtMin.Min(msMin.vtPVAJ[i]);
			vtMax.Max(msMax.vtPVAJ[i]);
		}
		CVect2 ptMin(msMin.t, vtMin.MinElem());
		CVect2 ptMax(msMax.t, vtMax.MaxElem());
		SetViewRegion(ptMin, ptMax);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPathSpeedView drawing

void CPathSpeedView::OnDraw(CDC* pDC)
{
	CPathSpeedDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
/*
	Plot selection of pos, vel, acc, jerk of the x, y, z axis for required or actual cubic segment values
  
	 
*/
	SetViewportOrgToFocus(pDC);

	const double xReal2Log = m_real2log.x;		// a local copy - maybe faster?
	const double yReal2Log = m_real2log.y;
	const double xFocusReal = m_ptFocusReal.x;
	const double yFocusReal = m_ptFocusReal.y;
//	CVect2 ptLog16Orig;
//	GetWindowULReal(ptLog16Orig);
//	const double xLog16Orig = ptLog16Orig.x;
//	const double yLog16Orig = ptLog16Orig.y;
	const double maxRealCoordX = m_log2real.x * m_maxLogicalCoord;		// 8192 pixels (~5 display widths) either side
	const double maxRealCoordY = fabs(m_log2real.y) * m_maxLogicalCoord;
	
	CMotionStateBasicArray* arpMotArray[3][2];
	arpMotArray[0][0] = &pDoc->m_MotionArray;
	arpMotArray[0][1] = &pDoc->m_BezMotionArray;
	arpMotArray[1][0] = &pDoc->m_ActualMotionArray;
	arpMotArray[1][1] = &pDoc->m_BezActualMotionArray;
	arpMotArray[2][0] = &pDoc->m_RequestMotionArray;
	arpMotArray[2][1] = &pDoc->m_BezRequestMotionArray;

	int numPtsA = pDoc->m_MotionArray.GetSize();
	int numPtsB = pDoc->m_ActualMotionArray.GetSize();
	int numPtsC = pDoc->m_RequestMotionArray.GetSize();
	int numPts = max(numPtsA, max(numPtsB, numPtsC));		// max num points to store
	int numBezPts = 3 * numPts - 2;
	if (numPts == 0)
		numBezPts = 0;
	CPoint* arPts = new CPoint[numPts];
	CPoint* arBezPts = new CPoint[numBezPts];
	CPoint* arStepPts = new CPoint[2 * numPts];

	CPen* pPenOrig = pDC->GetCurrentPen();
	CBrush* pBrushOrig = pDC->GetCurrentBrush();

	CPen penAxis[3];
	penAxis[0].CreatePen(PS_SOLID, 0, RED);		// these match toolbar axis buttons
	penAxis[1].CreatePen(PS_SOLID, 0, GREEN);
	penAxis[2].CreatePen(PS_SOLID, 0, YELLOW);
	CBrush brush;
	brush.CreateStockObject(NULL_BRUSH);

	SDrawNodes dn;
	dn.iRadius = 4;
	dn.nEndType = 0;
	dn.nControlType = 0;		// 6 for x
	dn.iStart = 0;
	if (m_nShowNodes & 1) dn.nEndType = 6;
	if (m_nShowNodes & 2) dn.nControlType = 7;

	CVect2 vtShowMin, vtShowMax;
	CVect2 vtWndSizeReal, vtFocusReal;
	GetWindowSizeReal(vtWndSizeReal);
	GetFocusReal(vtFocusReal);
	vtShowMin = vtFocusReal - vtWndSizeReal;
	vtShowMax = vtFocusReal + vtWndSizeReal;

	for (int seg = 0; seg < 3; seg++)	// segment type: 0 required, 1 actual segments, 2 requested(same as required!)
	{
		CMotionStateBasicArray& motArray = *arpMotArray[seg][0];
		CMotionStateBasicArray& bezMotArray = *arpMotArray[seg][1];
		numPts = motArray.GetSize();
		numBezPts = bezMotArray.GetSize();
		if (numPts == 0 || numBezPts == 0)
			continue;

		// calc times for this group of arrays
		int numPtsDraw;
		int idxarPtsInit, idxarPtsFinal;
		SetLogicalXValuesInRange(arPts, &motArray[0].t, sizeof(motArray[0]), numPts, vtShowMin.x, vtShowMax.x, idxarPtsInit, numPtsDraw);
		idxarPtsFinal = idxarPtsInit + numPtsDraw - 1;


		int idxarBezPtsInit, idxarBezPtsFinal;
		SetLogicalXBezValuesInRange(arBezPts, &bezMotArray[0].t, sizeof(bezMotArray[0]), numBezPts, vtShowMin.x, vtShowMax.x, idxarBezPtsInit, numPtsDraw);
		idxarBezPtsFinal = idxarBezPtsInit + numPtsDraw - 1;

		for (int ax = 0; ax < 3; ax++)		// axis x, y, z
		for (int pva = 0; pva < 4; pva++)	// 1-pos, 2-vel, 3-acc, 4-jerk
		{
			if ((m_nShowSeg & (1<<seg)) && (m_nShowAxis & (1<<ax)) && (m_nShowPVA & (1<<pva)))
			{
				int lineType = 0;		// straight line segments for acc and jerk
				if (pva <= 1)
					lineType = 1;		// bezier segments for pos and vel

				if (pva == 2 && seg == 1)	// actual acceleration segments (straight) - special handling
				{
					CSegAccelArray& segAccelArray = pDoc->m_ContTracker.m_SegAccelArray;
					double dAcc = pDoc->m_ContTracker.Get_dAcc();
					double dt = pDoc->m_ContTracker.Get_dt();

					// draw accel segments
					int iTimeStart;
					int iTimeEnd = segAccelArray[0].GetTimeStart();
					int iAccStart;
					int iAccEnd = 0;
					int iPt = 0;
					const int arrSize = segAccelArray.GetSize();
					for (int i = 0; i < arrSize; i++)
					{
						SSegAccel& segAcc = segAccelArray[i];
						iTimeStart = iTimeEnd;
						iTimeEnd += segAcc.idTime;
						if (iPt == 0)
						{
							double xVal = dt * iTimeStart;
							if (xVal < vtShowMin.x || xVal > vtShowMax.x)				// point out of range
								continue;
						}
						double xVal = dt * iTimeEnd;
						if (xVal > vtShowMax.x)				// point out of range
							break;				// draw points up to here
						double xReal = xVal - xFocusReal;

						if (segAcc.nSegType == PATHSEG_ACCEL)		// if accel stepped, or first one
						{	// step to segment start from prev final
							iAccStart = segAcc.iAcc[ax];
							iAccEnd = iAccStart;
							double yReal = dAcc * iAccStart - yFocusReal;
							arStepPts[iPt].y = NEARINT(yReal2Log * yReal);
							if (iPt != 0)
								arStepPts[iPt].x = arStepPts[iPt-1].x;	// time is same as previous step
							else		// if first time
							{
								double xReal = dt * iTimeStart - xFocusReal;
								arStepPts[iPt].x = NEARINT(xReal2Log * xReal);
							}
							iPt++;
						}
						else if (segAcc.nSegType == PATHSEG_dACCEL)
						{
							iAccStart = iAccEnd;
							iAccEnd += segAcc.idTime * segAcc.iAcc[ax];
						}

//						double xReal = dt * iTimeEnd - xFocusReal;
						double yReal = dAcc * iAccEnd - yFocusReal;
						if ((fabs(xReal) > maxRealCoordX) || (fabs(yReal) > maxRealCoordY))	// don't use point - out of range!
						{
							TRACE2("CPathSpeedView::OnDraw()  Coord too big x: %d  y: %d\n", xReal, yReal);
							// out of range - don't use point
						}
						arStepPts[iPt].x = NEARINT(xReal2Log * xReal);
						arStepPts[iPt].y = NEARINT(yReal2Log * yReal);
						iPt++;
					}
					pDC->SelectObject(&penAxis[ax]);		// set color for type - axis
					pDC->Polyline(arStepPts, iPt);
				}
				else if (lineType == 0)	// straight segment
				{
					pDC->SelectObject(&penAxis[ax]);		// set color for type - axis
					dn.iPeriod = 1;
					int idxInit, numDraw;
					bool bInRange = false;
					bool bDraw = false;
					for (int i = idxarPtsInit; i <= idxarPtsFinal; i++)
					{
						double yReal = motArray[i].vtPVAJ[pva][ax] - yFocusReal;
						if (fabs(yReal) <= maxRealCoordY)				// point in range	
						{
							if (!bInRange)
								{ bInRange = true; idxInit = i; }
							arPts[i].y = NEARINT(yReal2Log * yReal);
							if (i == idxarPtsFinal)		// got to last so draw!
								{ bDraw = true; numDraw = i - idxInit + 1; }
						}
						else			// point out of range
						{
							if (bInRange)
							{
								bInRange = false;
								numDraw = i - idxInit;
								bDraw = true;
							}
						}
						if (bDraw)
						{
							pDC->Polyline(arPts + idxInit, numDraw);
							dn.arPts = arPts + idxInit;
							dn.numPts = numDraw;
							DrawNodes(pDC, dn);
						}
					}
				}
				else if (lineType == 1)		// bezier segment
				{
					for (int i = idxarBezPtsInit; i <= idxarBezPtsFinal; i++)
						arBezPts[i].y = NEARINT(yReal2Log * (bezMotArray[i].vtPVAJ[pva][ax] - yFocusReal));

					int idxInit = idxarBezPtsInit;
					int numDraw = 1 + idxarBezPtsFinal - idxarBezPtsInit;
					pDC->SelectObject(&penAxis[ax]);		// set color for type - axis
					pDC->PolyBezier(arBezPts + idxInit, numDraw);
					dn.arPts = arBezPts + idxInit;
					dn.numPts = numDraw;
					dn.iPeriod = 3;
					DrawNodes(pDC, dn);
				}

			}
		}
	}
	delete[] arPts;
	delete[] arBezPts;
	delete[] arStepPts;

	DrawAxes(pDC);
	pDC->SelectObject(pPenOrig);
	pDC->SelectObject(pBrushOrig);


}

/////////////////////////////////////////////////////////////////////////////
// CPathSpeedView message handlers

void CPathSpeedView::OnShowAxis(UINT nID)
{
	m_nShowAxis ^= 1 << (nID - ID_SHOWX);		// toggle relevant bit
	Invalidate();
}

void CPathSpeedView::OnUpdateShowAxis(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(1 & (m_nShowAxis >> (pCmdUI->m_nID - ID_SHOWX)));
}

void CPathSpeedView::OnShowPVA(UINT nID)
{
	m_nShowPVA ^= 1 << (nID - ID_SHOWPOS);		// toggle relevant bit
	Invalidate();
}

void CPathSpeedView::OnUpdateShowPVA(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(1 & (m_nShowPVA >> (pCmdUI->m_nID - ID_SHOWPOS)));
}

void CPathSpeedView::OnShowSegs(UINT nID)
{
	m_nShowSeg ^= 1 << (nID - ID_SHOWREQUESTEDSEGS);		// toggle relevant bit
	Invalidate();
}

void CPathSpeedView::OnUpdateShowSegs(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(1 & (m_nShowSeg >> (pCmdUI->m_nID - ID_SHOWREQUESTEDSEGS)));
}

void CPathSpeedView::OnShowNodes(UINT nID)
{
	m_nShowNodes ^= 1 << (nID - ID_SHOWENDNODES);		// toggle relevant bit
	Invalidate();
}

void CPathSpeedView::OnUpdateShowNodes(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(1 & (m_nShowNodes >> (pCmdUI->m_nID - ID_SHOWENDNODES)));
}





