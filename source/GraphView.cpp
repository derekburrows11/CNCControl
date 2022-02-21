// GraphView.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"		// for resource ID's

#include "GraphView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/*
int Plot(double* arX, double* arY, int numPoints)
{
	// initiate a GraphView window
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_GRAPHVIEW,
		RUNTIME_CLASS(CDocument),
		RUNTIME_CLASS(CMDIChildWnd),
		RUNTIME_CLASS(CGraphView));
	CFrameWnd* pFrame = pDocTemplate->CreateNewFrame(NULL, NULL);

	// find new CGraphView object to set plot data
	CWnd* pWnd = pFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST);
	ASSERT( pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CGraphView)) );
	((CGraphView*)pWnd)->Plot(arX, arY, numPoints);			// 1 for axis parameters or 0 for general

	pDocTemplate->InitialUpdateFrame(pFrame, NULL);
	AfxGetApp()->AddDocTemplate(pDocTemplate);		// why??


	return 1;
}
*/

/////////////////////////////////////////////////////////////////////////////
// CGraphView

IMPLEMENT_DYNCREATE(CGraphView, CScrollView)

CGraphView::CGraphView()
{
}

CGraphView::~CGraphView()
{
}


BEGIN_MESSAGE_MAP(CGraphView, CScrollView)
	//{{AFX_MSG_MAP(CGraphView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphView drawing

void CGraphView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);


	// set some default colors
	int l = 0;
	m_arLineColor[l++] = YELLOW;
	m_arLineColor[l++] = RED;
	m_arLineColor[l++] = BLUE;
	m_arLineColor[l++] = ORANGE;
	m_arLineColor[l++] = GREEN;
	m_arLineColor[l++] = MAGENTA;
	m_arLineColor[l++] = CYAN;
	m_arLineColor[l++] = PINK;

}



void CGraphView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();

	// CGraphView plots a number of lines from the set of value vectors

	int xRadius = 5;
	int yRadius = 5;
	int xDiamP1 = 2*xRadius + 1;		// to suit Ellipse function
	int yDiamP1 = 2*yRadius + 1;

	CPen* pPenOrig = pDC->GetCurrentPen();
	POINT* arPoints = NULL;

	POSITION pos = m_GraphList.GetTailPosition();
	while (pos)
	{
		SGraphData& graphData = m_GraphList.GetPrev(pos);
		int numPoints = graphData.numPoints;
		if (graphData.arPoints)
			arPoints = graphData.arPoints;

		CPen penGraph(PS_SOLID, 0, graphData.color);	// will be destrored before deselected
		pDC->SelectObject(penGraph);
//		pDC->SelectObject(CPen(PS_SOLID, 0, graphData.color));


		VERIFY(pDC->Polyline(arPoints, numPoints));
		VERIFY(pDC->PolyBezier(arPoints, numPoints));
		for (int i = 0; i < numPoints; i++)
		{
			int x1 = arPoints[i].x - xRadius;
			int y1 = arPoints[i].y - yRadius;
			pDC->Ellipse(x1, y1, x1+xDiamP1, y1+yDiamP1);
		}
	}
	pDC->SelectObject(pPenOrig);

}

/////////////////////////////////////////////////////////////////////////////
// CGraphView diagnostics

#ifdef _DEBUG
void CGraphView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CGraphView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGraphView message handlers

BOOL CGraphView::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect, RGB(0,0,0));
	return true;
//	return CScrollView::OnEraseBkgnd(pDC);
}



bool CGraphView::Plot(double* arX, double* arY, int numPoints)
{
	// can plot lines, points or beziers
	// convert 'floating' arrays to an array of 'POINT' for dc functions using appropriate scaling
	// converted points are stored in object to draw at draw time!

	POINT* arPoints = new POINT[numPoints];


	double xScale = m_XScale;		// a local copy - maybe faster?
	double yScale = m_YScale;
	double xOff = m_XOffset + 0.5;
	double yOff = m_YOffset + 0.5;

	int xRadius = 5;
	int yRadius = 5;
	int xDiamP1 = 2*xRadius + 1;		// to suit Ellipse function
	int yDiamP1 = 2*yRadius + 1;

	for (int i = 0; i < numPoints; i++)
	{
		arPoints[i].x = (long)floor((arX[i] * xScale) + xOff);
		arPoints[i].y = (long)floor((arY[i] * yScale) + yOff);
		// mark point
		int x1 = arPoints[i].x - xRadius;
		int y1 = arPoints[i].y - yRadius;
//		pDC->Ellipse(x1, y1, x1+xDiamP1, y1+yDiamP1);
	}

//	pDC->Polyline(arPoints, numPoints);
//	pDC->PolyBezier(arPoints, numPoints);

	SGraphData newGraph;
	newGraph.numPoints = numPoints;
	newGraph.arPoints = arPoints;
	newGraph.color = m_LineColor;
	newGraph.lineType = m_LineType;
	m_GraphList.AddHead(newGraph);

	return true;
}
