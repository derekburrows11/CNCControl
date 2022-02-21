#if !defined(AFX_PARAMVIEW_H__B8B36A00_67BE_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
#define AFX_PARAMVIEW_H__B8B36A00_67BE_11D4_8C1E_C12B6D9C0F2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "GridView.h"
// ParamView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CParamView view

class CParamView : public CGridView
{
protected:
	CParamView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CParamView)

// Attributes
public:
	enum VIEWTYPE {
		VT_NONE = 0,
		VT_GENPARAM,
		VT_AXISPARAM,
	};
	CParamDoc* GetDocument();
protected:
	int m_nAxis0SubItem;
	enum VIEWTYPE m_nViewType;		// general or axis parameter view

// Operations
public:
	void SetViewType(int nViewType);

// Overrides
protected:

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParamView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CParamView();
	void UpdateItemValue(int par, int axis = -1);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


// Generated message map functions
protected:
	//{{AFX_MSG(CParamView)
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSendParam();
	afx_msg void OnRequestParam();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARAMVIEW_H__B8B36A00_67BE_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
