// PathAnimateView.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"
#include "Colors.h"

#include "PathDoc.h"
#include "PathSpeedDoc.h"
#include "AnimationSettingsDlg.h"
#include "PlayerBar.h"
#include "MainFrm.h"

#include "PathAnimateView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




/////////////////////////////////////////////////////////////////////////////
// CPathAnimateView
/*
	CPathAnimateView shows path and animates in real time the tool movenent



*/



IMPLEMENT_DYNCREATE(CPathAnimateView, CPathView)

CPathAnimateView::CPathAnimateView()
{
}

CPathAnimateView::~CPathAnimateView()
{
}


BEGIN_MESSAGE_MAP(CPathAnimateView, CPathView)
	//{{AFX_MSG_MAP(CPathAnimateView)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_COMMAND(ID_PATH_ANIMATE_START, OnAnimateStart)
	ON_COMMAND(ID_PATH_ANIMATE_STOP, OnAnimateStop)
	ON_COMMAND(ID_PATH_ANIMATE_PAUSE, OnAnimatePause)
	ON_COMMAND(ID_PATH_ANIMATE, OnShowPathAnimate)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_UPDATE_COMMAND_UI(ID_PATH_ANIMATE_PAUSE, OnUpdatePathAnimatePause)
	ON_COMMAND(ID_ANIMATIONSETTINGS, OnAnimationSettings)
	ON_COMMAND(IDC_EDIT_RATE, OnPlayerRateChange)
	//}}AFX_MSG_MAP
	ON_CONTROL(DLGN_VALUECHANGED, IDW_PLAYER_BAR, OnPlayerBarChange)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPathAnimateView diagnostics

#ifdef _DEBUG
void CPathAnimateView::AssertValid() const
{
	CPathView::AssertValid();
}

void CPathAnimateView::Dump(CDumpContext& dc) const
{
	CPathView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPathAnimateView docs

/*
inline CPathDoc* CPathAnimateView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPathDoc)));
	return (CPathDoc*)m_pDocument;
}
*/
inline CPathSpeedDoc* CPathAnimateView::GetSpeedDoc()
{
	m_pSpeedDoc = GetDocument()->GetSpeedDoc();
	if (m_pSpeedDoc != NULL)
		ASSERT(m_pSpeedDoc->IsKindOf(RUNTIME_CLASS(CPathSpeedDoc)));
	return static_cast<CPathSpeedDoc*>(m_pSpeedDoc);
}

void CPathAnimateView::SetSpeedDoc(CPathSpeedDoc* pDoc)
{
	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CPathSpeedDoc)));
	m_pSpeedDoc = (CPathSpeedDoc*)pDoc;
}

/////////////////////////////////////////////////////////////////////////////
// CPathAnimateView Init

void CPathAnimateView::OnInitialUpdate() 
{
	CPathView::OnInitialUpdate();

	m_pSpeedDoc = NULL;

	m_bShowMarker = false;
	SetAnimationRate(1);
	m_IDTimer = 0;

}


void CPathAnimateView::ActivatePlayer()
{

	// turn PlayerBar on
	CFrameWnd* pFrmWnd = static_cast<CFrameWnd*>(AfxGetMainWnd());
	ASSERT(pFrmWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
	CControlBar* pBar = pFrmWnd->GetControlBar(IDW_PLAYER_BAR);
	pFrmWnd->ShowControlBar(pBar, true, true);

	// Drawing
//	m_penMarker.CreatePen(PS_SOLID, 0, RGB(255,180,50));
//	m_brMarker.CreateSolidBrush(RGB(50,180,50));
	m_bShowMarker = true;

//	m_pSegMotionArray = GetSpeedDoc()->m_CtrlTrack.m_SegMotionArray;
	OnAnimateReset();
//	OnAnimateStart();


}


void CPathAnimateView::SetAnimationRate(double rate)
{
	m_AnimationRate = rate;
}



/////////////////////////////////////////////////////////////////////////////
// CPathAnimateView drawing

void CPathAnimateView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

	DrawPath(pDC);
	DrawAxesTriple(pDC);
	DrawRegionBox(pDC);
	m_bMarkerVisible = false;
	if (m_bShowMarker)
		DrawMarker(pDC);
}


/////////////////////////////////////////////////////////////////////////////
// CPathAnimateView message handlers

void CPathAnimateView::OnShowPathAnimate()
{
	if (GetSpeedDoc() != NULL)
		ActivatePlayer();

}


void CPathAnimateView::UpdateMotion()
{
	__int64 iFreq, iCount;
	QueryPerformanceFrequency((LARGE_INTEGER*)&iFreq);
	QueryPerformanceCounter((LARGE_INTEGER*)&iCount);
	int iCountms = (int)((double)iCount / (iFreq * 1e-3));
	
	int iTickCount = GetTickCount() - m_iTickCountStartPath;
	m_iTPath = NEARINT(iTickCount * m_AnimationRate);		// path time in ms
	GetMotionAtPathTime(m_iTPath);
}

void CPathAnimateView::GetMotionAtPathTime(int iTPath)
{
	// calculate pos on path at current time
	// using calculated controller segment motions
	CPathSpeedDoc* pSpeedDoc = GetSpeedDoc();
	ASSERT_VALID(pSpeedDoc);
	CSegAccelArray& segAccelArray = pSpeedDoc->m_ContTracker.m_SegAccelArray;
	int sizeSegAccelArray = segAccelArray.GetSize();
	ASSERT(sizeSegAccelArray == m_sizeSegAccelArray);		// check array doesn't change
	SSegAccel* pSegAcc;
	SMotionI iMotAtTime;

	while (1)
	{
		if (m_bAtEnd)		// || (m_idxSegAcc >= sizeSegAccelArray)
		{
			iMotAtTime = m_iMot;
			break;
		}
		ASSERT(m_idxSegAcc < sizeSegAccelArray);
		pSegAcc = &segAccelArray[m_idxSegAcc];
		if (iTPath <= pSegAcc->GetTimeEnd())
		{
			while (iTPath < pSegAcc->GetTimeStart())
			{
				//go back a segment
				m_idxSegAcc--;
				if (m_idxSegAcc < 0)
					break;				// report iMot at start of first segment
				pSegAcc = &segAccelArray[m_idxSegAcc];
				m_iMot.BackTrackMotion(*pSegAcc);
			}
			iMotAtTime = m_iMot;
			if (m_idxSegAcc >= 0)
				iMotAtTime.UpdateMotionAtPathTime(*pSegAcc, iTPath);
			break;
		}
		m_iMot.UpdateMotion(*pSegAcc);
		m_idxSegAcc++;
		if (m_idxSegAcc >= sizeSegAccelArray)		// got to end of segments
		{
			iMotAtTime = m_iMot;
			m_bAtEnd = true;
			OnAnimatePause();
			break;				// report iMot at end of last segment
		}
	}
	pSpeedDoc->m_ContTracker.Int2Real(m_fMotAtTime, iMotAtTime);
}


void CPathAnimateView::UpdateMarker()
{
	CClientDC DC(this);
	CDC memDC;
	memDC.CreateCompatibleDC(&DC);

	CRect rectWnd;
	GetClientRect(rectWnd);		// in device coords (pixels)
	CSize sizeBitmap(rectWnd.Size());
//	sizeWnd.cx -= 30;
//	sizeWnd.cy -= 30;

//	sizeBitmap = CSize(40,40);		// size of bitmap
	CBitmap bm;
	bm.CreateCompatibleBitmap(&DC, sizeBitmap.cx, sizeBitmap.cy);
	CBitmap* pBitmapOrig = memDC.SelectObject(&bm);
	memDC.FillSolidRect(&rectWnd, BLACK);

	// Erase old marker
//	if (m_bMarkerVisible)
//		DC.BitBlt(m_ptMarker.x, m_ptMarker.y, sizeBitmap.cx, sizeBitmap.cy, &memDC, 0, 0, SRCINVERT);


	DrawMarker(&memDC);

// when memDC is drawn
//	SetViewportOrgToFocus(&DC);
	DC.SetWindowOrg(0,0);
	memDC.SetViewportOrg(0,0);

	DC.BitBlt(0, 0, sizeBitmap.cx, sizeBitmap.cy, &memDC, 0, 0, SRCINVERT);

	memDC.SelectObject(pBitmapOrig);
}

void CPathAnimateView::DrawMarker(CDC* pDC)
{
	SetViewportOrgToFocus(pDC);
	pDC->SetWindowOrg(0,0);

//	const double xReal2Log = m_real2log.x;		// a local copy - maybe faster?
//	const double yReal2Log = m_real2log.y;
//	const double xFocusReal = m_ptFocusReal.x;
//	const double yFocusReal = m_ptFocusReal.y;

//	m_fMotAtTime.pos;
	CVector vtLocation;
	vtLocation.SetFromArray(m_fMotAtTime.pos);

	CVector vtTrxLoc;
	vtTrxLoc.Prod(m_mxViewTransform2Log, (vtLocation - m_ptFocusReal));
	CPoint ptLoc = vtTrxLoc;


	CBrush brMarker;
	brMarker.CreateSolidBrush(GREEN);
	CPen penMarker;
	penMarker.CreatePen(PS_SOLID, 0, GREEN);

	CPen* pPenOrig = pDC->GetCurrentPen();
	CBrush* pBrushOrig = pDC->GetCurrentBrush();

	pDC->SelectObject(penMarker);
//	pDC->SelectStockObject(NULL_PEN);
//	pDC->SelectObject(brMarker);
	pDC->SelectStockObject(NULL_BRUSH);

	pDC->SetROP2(R2_XORPEN);
//	pDC->SetBkMode(TRANSPARENT);


	// calc marker size
	double zMax = m_ptMaxTxRegion.z;
	double zMin = m_ptMinTxRegion.z;
	const double change = 5;	// changes by factor of 5
	// for inverse distance
//	double distView = (zMax - zMin) / (change - 1);
//	double sizeView = distView / (distView + zMax - vtTrxLoc.z);	// varies from 1 down to 1/change
	// for linear change
	double slope = (change - 1) / ((zMax - zMin) * change);
	double sizeView = 1 - slope * (zMax - vtTrxLoc.z);

	// calc marker triangle
	double height = sizeView * 16;		// in mm
	int h = NEARINT(m_mm2log.y * height);
	int w = h / 2;

	// Erase old marker, draw new
	if (m_bMarkerVisible)
		pDC->Polygon(m_arPtsTriangle, 3);
	m_arPtsTriangle[0] = ptLoc;
	m_arPtsTriangle[1] = CPoint(-w,h) + ptLoc;
	m_arPtsTriangle[2] = CPoint(w,h) + ptLoc;
	pDC->Polygon(m_arPtsTriangle, 3);
	m_bMarkerVisible = true;

	pDC->SelectObject(pPenOrig);
	pDC->SelectObject(pBrushOrig);
}

void CPathAnimateView::OnDestroy() 
{
	CPathView::OnDestroy();
	
	// TODO: Add your message handler code here
	KillTimer(m_IDTimer);

}

void CPathAnimateView::OnAnimateStart() 
{
	CPathSpeedDoc* pSpeedDoc = GetSpeedDoc();
	if (pSpeedDoc == NULL)
		return;
	ASSERT_VALID(pSpeedDoc);

	if (m_IDTimer)
	{
		OnAnimatePause();
		return;
	}
	m_IDTimer = SetTimer(1, 10, NULL);		// ~55ms time out messages
	if (!m_IDTimer)
	{
		TRACE("Didn't get a timer!\n");
		return;
	}

// timing
	m_iTickCountStartPath = GetTickCount() - NEARINT(m_iTPath / m_AnimationRate);
	OnTimer(m_IDTimer);			// do first one at 0 time

	// to run smooth but no message queue!
//	while (!m_bAtEnd)
//		OnTimer(1);
}

void CPathAnimateView::OnAnimateStop() 
{
	KillTimer(m_IDTimer);
	m_IDTimer = 0;
	OnAnimateReset();
}

void CPathAnimateView::OnAnimatePause() 
{
	KillTimer(m_IDTimer);
	m_IDTimer = 0;
}

void CPathAnimateView::OnAnimateReset()
{
	CPathSpeedDoc* pSpeedDoc = GetSpeedDoc();
	if (pSpeedDoc == NULL)
		return;
	ASSERT_VALID(pSpeedDoc);

// segment motion data
	m_iMot.Zero();
	GetSpeedDoc()->m_ContTracker.GetInitialMotion(m_iMot);
	m_idxSegAcc = 0;
	m_iTPath = 0;
	m_sizeSegAccelArray = GetSpeedDoc()->m_ContTracker.m_SegAccelArray.GetSize();		// check array doesn't change
	m_bAtEnd = false;
	GetMotionAtPathTime(0);
	UpdateMarker();

	m_iTickCountStartPath = GetTickCount();
}

void CPathAnimateView::OnTimer(UINT nIDEvent) 
{
	ASSERT(nIDEvent == m_IDTimer);
//	SetWaitableTimer;

	UpdateMotion();
	UpdateMarker();

//while(1)
	{
//		int iTickCount = GetTickCount() - m_iTickCountInit;
//		double time = iTickCount * 1e-3;

//		m_vtLocation.x = 100 * sin(time);


		static DWORD timeInit;
		static int count;
		static int timePrev;

		int ttime = GetTickCount();
//		TRACE1("This time %i ms\n", ttime - timePrev);
		if (count == 0)
			timeInit = ttime;
		if (count == 0x40)
		{
//			TRACE1("Avg time %.2f ms\n", (double)(ttime - timeInit) / count);
			timeInit = ttime;
			count = 0;
		}
		count++;
		timePrev = ttime;
	}
}





void CPathAnimateView::OnKillFocus(CWnd* pNewWnd) 
{
	CPathView::OnKillFocus(pNewWnd);
//	if (m_wndPlayerBar.m_hWnd)
//		m_wndPlayerBar.GetDockingFrame()->ShowWindow(SW_HIDE);
}

void CPathAnimateView::OnSetFocus(CWnd* pOldWnd) 
{
	CPathView::OnSetFocus(pOldWnd);
//	if (m_wndPlayerBar.m_hWnd)
//		m_wndPlayerBar.GetDockingFrame()->ShowWindow(SW_SHOW);
}

void CPathAnimateView::OnUpdatePathAnimatePause(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_IDTimer != 0);	
}


void CPathAnimateView::OnAnimationSettings() 
{
	CAnimationSettingsDlg dlg;
	dlg.m_Rate = m_AnimationRate;
	if (dlg.DoModal() == IDOK)
	{
		m_AnimationRate = dlg.m_Rate;
		m_iTickCountStartPath = GetTickCount() - NEARINT(m_iTPath / m_AnimationRate);
	}
}

void CPathAnimateView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
//	set player bar to our rate
	CPathView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CPathAnimateView::OnPlayerRateChange() 
{
	CPlayerBar* pPlayerBar = &((CMainFrame*)AfxGetMainWnd())->m_wndPlayerBar;
	m_AnimationRate = pPlayerBar->m_Rate;
	m_iTickCountStartPath = GetTickCount() - NEARINT(m_iTPath / m_AnimationRate);

	
}

void CPathAnimateView::OnPlayerBarChange()
{
}