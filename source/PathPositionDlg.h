#if !defined(AFX_PATHDISPLAYDLG_H__CE17F1E2_7700_11D7_86C3_8A13E3817F26__INCLUDED_)
#define AFX_PATHDISPLAYDLG_H__CE17F1E2_7700_11D7_86C3_8A13E3817F26__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PathPositionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPathPositionDlg dialog

class CPathPositionDlg : public CDialog
{
// Construction
public:
	CPathPositionDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPathPositionDlg)
	enum { IDD = IDD_PATHPOSITION };
	CScrollBar	m_scrollPos;
	CString	m_strStartSeg;
	int		m_iNumSegsShow;
	int		m_nrStartSeg;
	//}}AFX_DATA

	void SetView(CWnd* pViewWnd) { m_pViewWnd = pViewWnd; }	// then values should be set by new view


	int m_iStartSeg;
	int m_iNumSegsPath;
	bool m_bUseNR;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPathPositionDlg)
	public:
	virtual BOOL Create(CWnd* pViewWnd = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPathPositionDlg();
protected:

	void NotifyView();
	CWnd* m_pViewWnd;		// view for which dialog is setting values

	// Generated message map functions
	//{{AFX_MSG(CPathPositionDlg)
	afx_msg void OnEditChange();
	afx_msg void OnEditChangeCombo1();
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnViewAll();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATHDISPLAYDLG_H__CE17F1E2_7700_11D7_86C3_8A13E3817F26__INCLUDED_)
