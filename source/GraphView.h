#if !defined(AFX_GRAPHVIEW_H__34FD4846_3C8C_11D5_8C1E_852F691E2A2F__INCLUDED_)
#define AFX_GRAPHVIEW_H__34FD4846_3C8C_11D5_8C1E_852F691E2A2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GraphView.h : header file
//

#include <afxtempl.h>

#include "..\Common\Matrix.h"


enum {
	BLACK		= RGB(0, 0, 0),
	DARKGRAY	= RGB(64, 64, 64),
	GRAY		= RGB(128, 128, 128),
	LIGHTGRAY= RGB(192, 192, 192),
	WHITE		= RGB(255, 255, 255),
	RED		= RGB(255, 0, 0),
	GREEN		= RGB(0, 255, 0),
	BLUE		= RGB(0, 0, 255),
	CYAN		= RGB(0, 255, 255),
	MAGENTA	= RGB(255, 0, 255),
	YELLOW	= RGB(255, 255, 0),
	ORANGE	= RGB(255, 200, 0),
	PINK		= RGB(255, 175, 175),
};




struct SGraphData
{
	int lineType;
	COLORREF color;
	int numPoints;
	POINT* arPoints;

	SGraphData() { arPoints = NULL; }
	~SGraphData() { if (arPoints) delete[] arPoints; }
};


/////////////////////////////////////////////////////////////////////////////
// CGraphView view

class CGraphView : public CScrollView
{
protected:
	CGraphView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CGraphView)

// Attributes
public:

protected:
	CList<SGraphData, SGraphData&> m_GraphList;

	double m_XScale, m_YScale;
	double m_XOffset, m_YOffset;
	int m_LineType;
	COLORREF m_LineColor;

	COLORREF m_arLineColor[10];


// Operations
public:
	bool Plot(double* arX, double* arY, int numPoints);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CGraphView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CGraphView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPHVIEW_H__34FD4846_3C8C_11D5_8C1E_852F691E2A2F__INCLUDED_)
