// CNCControlView.h : interface of the CCNCControlView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CNCCONTROLVIEW_H__4E7108AE_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
#define AFX_CNCCONTROLVIEW_H__4E7108AE_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CCNCControlView : public CView
{
protected: // create from serialization only
	CCNCControlView();
	DECLARE_DYNCREATE(CCNCControlView)

// Attributes
public:
	CCNCControlDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCNCControlView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCNCControlView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCNCControlView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CNCControlView.cpp
inline CCNCControlDoc* CCNCControlView::GetDocument()
   { return (CCNCControlDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CNCCONTROLVIEW_H__4E7108AE_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
