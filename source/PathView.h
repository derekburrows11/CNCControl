#if !defined(AFX_PATHVIEW_H__90231B6A_00B8_11D5_8C1E_88F34AF5402C__INCLUDED_)
#define AFX_PATHVIEW_H__90231B6A_00B8_11D5_8C1E_88F34AF5402C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PathView.h : header file
//

#include "MoveView.h"
#include "PathPositionDlg.h"


class CPathDoc;





/////////////////////////////////////////////////////////////////////////////
// CPathView view

class CPathView : public CMoveView
{
protected:
	CPathView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPathView)

// Attributes
public:

protected:
	CPathDoc* m_pDoc;

	struct SPathSegType
	{
		int nLineStyle;		// LS_START, LS_STRAIGHT, LS_BEZIER, LS_FINISH
		int nFeedRate;			// 0 norm, 1 rapid
//		int idxSegType;		// ST_ORIGIONAL, ST_FITTED
		bool bForCurveFit;	// true if curve will be fit to segments
		int nEndNodeType;		// PNT_???
		NODEREF nrEndNode;	// NODEREF of end node of segment
	};

	// path drawing controls
	enum		// line styles
	{
		LS_START = 1,
		LS_STRAIGHT,
		LS_BEZIER,
		LS_FINISH,
	};
	enum		// path color from
	{
		PC_FEEDRATE = 1,
		PC_SEGTYPE,
	};
	int m_nPathColorFrom;	// sets path color method
	enum		// segment type flags
	{
		STF_ORIGIONAL		= 0x01,
		STF_FITTEDCURVE	= 0x02,
	};
	int m_nShowSegTypes;		// show segment type flags

	enum		// show node flags
	{
		SNF_END				= 0x01,
		SNF_CONTROL			= 0x02,
		SNF_CONTROLLINE	= 0x04,
		SNF_BEZIER			= 0x08,		// lines actually!
	};
	int m_nShowNodes;		// show nodes flags
/*	enum		// Line Type Flags
	{
		LTF_STRAIGHT			= 0x01,
		LTF_BEZIER				= 0x02,
		LTF_ARC					= 0x04,
		LTF_ARCFITBEZIER		= 0x08,
		LTF_CURVINGSEGS		= 0x10,
		LTF_CURVEFITBEZIER	= 0x20,
	};
	int m_nShowLineTypes;		// show lines type flags
*/
	enum		// segment change flags
	{
		SCF_LINESTYLE = 0x01,	// straight, bezier
		SCF_FEEDRATE = 0x02,
		SCF_SEGTYPE = 0x04,
	};
	int m_nSegChange;		// segment change flags

	int m_iFirstSegToDraw;		// determines first seg drawn of path
	int m_iFirstNodeRefToDraw;	// found from m_iFirstSegToDraw, if unknown = -1
	int m_numSegsToDraw;
	NODEREF m_nrFirstSegToDraw;		// NODEREF of first node of first seg to draw, updated during drawing
	NODEREF m_nrLastSegToDraw;			// NODEREF of last node of last seg to draw, updated during drawing
	bool m_bUseNR;

	int m_numSegNext;

	SPathSegType m_SegTypeCurr, m_SegTypeNext;

	// data for fitted curves
	CCurveInfoArray* m_pCurveInfoArray;
	int m_idxCIA;
	NODEREF m_nrNextFitCurveExtent;

	CPathPositionDlg* m_pPathPosDlg;


// Operations
public:
	inline CPathDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPathView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPathView();
	void DrawPath(CDC* pDC);

	void ReadNextNode();

	void SetPathPosDlgData();
	bool GetPathPosDlgData();
	void UpdatePathPosDlg();


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CPathView)
	afx_msg void OnPathColorCurve();
	afx_msg void OnUpdatePathColorCurve(CCmdUI* pCmdUI);
	afx_msg void OnPathColorFeedrate();
	afx_msg void OnUpdatePathColorFeedrate(CCmdUI* pCmdUI);
	afx_msg void OnPathShowFitted();
	afx_msg void OnUpdatePathShowFitted(CCmdUI* pCmdUI);
	afx_msg void OnPathShowOrig();
	afx_msg void OnUpdatePathShowOrig(CCmdUI* pCmdUI);
	afx_msg void OnShowEndNodes();
	afx_msg void OnUpdateShowEndNodes(CCmdUI* pCmdUI);
	afx_msg void OnShowCtrlNodes();
	afx_msg void OnUpdateShowCtrlNodes(CCmdUI* pCmdUI);
	afx_msg void OnShowCtrlNodeLines();
	afx_msg void OnUpdateShowCtrlNodeLines(CCmdUI* pCmdUI);
	afx_msg void OnShowBeziers();
	afx_msg void OnUpdateShowBeziers(CCmdUI* pCmdUI);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	void OnPathPosDlgChangeCom();
	void OnPathPosDlgChange();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATHVIEW_H__90231B6A_00B8_11D5_8C1E_88F34AF5402C__INCLUDED_)
