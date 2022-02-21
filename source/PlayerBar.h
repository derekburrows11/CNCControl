#if !defined(AFX_PLAYERBAR_H__6E306F80_967F_11D7_86C3_B80770505225__INCLUDED_)
#define AFX_PLAYERBAR_H__6E306F80_967F_11D7_86C3_B80770505225__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PlayerBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlayerBar window

class CPlayerBar : public CToolBar
{
// Construction
public:
	double m_Rate;
//	CEdit m_RateBox;
	CComboBox m_RateBox;

	CPlayerBar();
	BOOL LoadToolBar(UINT nIDResource);



// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerBar)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPlayerBar();

protected:
	void NotifyView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlayerBar)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditChange();
	afx_msg void OnEditRate();
	afx_msg void OnUpdateEditRate(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYERBAR_H__6E306F80_967F_11D7_86C3_B80770505225__INCLUDED_)
