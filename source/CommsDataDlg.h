#if !defined(AFX_COMMSDATADLG_H__A218D4A2_9857_11D4_8C1E_9706F1610B2F__INCLUDED_)
#define AFX_COMMSDATADLG_H__A218D4A2_9857_11D4_8C1E_9706F1610B2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CommsDataDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCommsDataDlg dialog

class CCommsDataDlg : public CDialog
{
// Construction
public:
	bool AddToTx(BYTE* data, int num, bool bNewLine = true) { return AddToEditBox(m_TxBoxEdit, data, num, bNewLine); }
	bool AddToRx(BYTE* data, int num, bool bNewLine = true) { return AddToEditBox(m_RxBoxEdit, data, num, bNewLine); }
	CCommsDataDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
protected:
	//{{AFX_DATA(CCommsDataDlg)
	enum { IDD = IDD_COMMSDATA };
	CEdit	m_RxBoxEdit;
	CEdit	m_TxBoxEdit;
	//}}AFX_DATA

	bool AddToEditBox(CEdit& edit, BYTE* data, int num, bool bNewLine = true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommsDataDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	COLORREF m_clrTxText;
	COLORREF m_clrRxText;
	COLORREF m_clrBkgnd;
	CBrush m_brTxRxBkgnd;

	int m_iTxBoxChars;
	int m_iRxBoxChars;

	// Generated message map functions
	//{{AFX_MSG(CCommsDataDlg)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMSDATADLG_H__A218D4A2_9857_11D4_8C1E_9706F1610B2F__INCLUDED_)
