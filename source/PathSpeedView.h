// PathSpeedView.h: interface for the CPathSpeedView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHSPEEDVIEW_H__8200FCE3_B8E6_11D6_86C3_CE055F4F9526__INCLUDED_)
#define AFX_PATHSPEEDVIEW_H__8200FCE3_B8E6_11D6_86C3_CE055F4F9526__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MoveView.h"


class CPathSpeedDoc;

/////////////////////////////////////////////////////////////////////////////
// CPathSpeedView view

class CPathSpeedView : public CMoveView
{
protected:
	CPathSpeedView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPathSpeedView)

// Attributes
public:
	CPathSpeedDoc* GetDocument();

protected:
	int m_nShowAxis;		// show x, y, z axis flags (b0-b2)
	int m_nShowPVA;		// show pos, vel, acc, jerk flags (b0-b3)
	int m_nShowSeg;		// show desired (b0), actual (b1) segment values
	int m_nShowNodes;		// show end (b0), control (b1) nodes


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPathSpeedView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPathSpeedView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif



	// Generated message map functions
	//{{AFX_MSG(CPathSpeedView)
	//}}AFX_MSG
	void OnShowAxis(UINT nID);
	void OnUpdateShowAxis(CCmdUI* pCmdUI);
	void OnShowPVA(UINT nID);
	void OnUpdateShowPVA(CCmdUI* pCmdUI);
	void OnShowSegs(UINT nID);
	void OnUpdateShowSegs(CCmdUI* pCmdUI);
	void OnShowNodes(UINT nID);
	void OnUpdateShowNodes(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in PathSpeedView.cpp
inline CPathSpeedDoc* CPathSpeedView::GetDocument()
   { return (CPathSpeedDoc*)m_pDocument; }
#endif


#endif // !defined(AFX_PATHSPEEDVIEW_H__8200FCE3_B8E6_11D6_86C3_CE055F4F9526__INCLUDED_)
