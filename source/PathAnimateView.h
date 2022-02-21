#if !defined(AFX_PATHANIMATEVIEW_H__A56ECD22_39CD_11D7_86C3_D222F73C5A26__INCLUDED_)
#define AFX_PATHANIMATEVIEW_H__A56ECD22_39CD_11D7_86C3_D222F73C5A26__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PathAnimateView.h : header file
//
#include "ControllerTracker.h"

#include "PathView.h"

class CPathSpeedDoc;


/////////////////////////////////////////////////////////////////////////////
// CPathAnimateView view

class CPathAnimateView : public CPathView
{
protected:
	CPathAnimateView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPathAnimateView)

// Attributes
public:

protected:

	CPathSpeedDoc* m_pSpeedDoc;


	// segment motion data
//	CSegMotionArray* m_pSegMotionArray;
	SMotionI m_iMot;
	int m_idxSegAcc;
	int m_sizeSegAccelArray;
	SMotionF m_fMotAtTime;
	bool m_bAtEnd;

	// timing
	UINT m_IDTimer;
	int m_iTPath;
	int m_iTickCountStartPath;
	double m_AnimationRate;

	// drawing
	bool m_bShowMarker;
	bool m_bMarkerVisible;
	CPen m_penMarker;
	CBrush m_brMarker;
	CPoint m_arPtsTriangle[3];


// Operations
public:

//	inline CPathDoc* GetDocument();
	inline CPathSpeedDoc* GetSpeedDoc();
	void SetSpeedDoc(CPathSpeedDoc* pDoc);




// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPathAnimateView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPathAnimateView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


protected:
	void ActivatePlayer();
	void UpdateMotion();
	void UpdateMarker();
	void GetMotionAtPathTime(int iTPath);
	void DrawMarker(CDC* pDC);

	void OnAnimateReset();
	void SetAnimationRate(double rate);


	// Generated message map functions
protected:
	//{{AFX_MSG(CPathAnimateView)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnAnimateStart();
	afx_msg void OnAnimateStop();
	afx_msg void OnAnimatePause();
	afx_msg void OnShowPathAnimate();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnUpdatePathAnimatePause(CCmdUI* pCmdUI);
	afx_msg void OnAnimationSettings();
	afx_msg void OnPlayerRateChange();
	//}}AFX_MSG
	afx_msg void OnPlayerBarChange();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATHANIMATEVIEW_H__A56ECD22_39CD_11D7_86C3_D222F73C5A26__INCLUDED_)
