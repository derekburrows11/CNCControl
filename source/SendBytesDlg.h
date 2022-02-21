#if !defined(AFX_SENDBYTESDLG_H__3924F4A0_3080_11D8_86C3_9D288CFF5125__INCLUDED_)
#define AFX_SENDBYTESDLG_H__3924F4A0_3080_11D8_86C3_9D288CFF5125__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SendBytesDlg.h : header file
//

class CCNCComms;

/////////////////////////////////////////////////////////////////////////////
// CSendBytesDlg dialog

class CSendBytesDlg : public CDialog
{
// Construction
public:
	CSendBytesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSendBytesDlg)
	enum { IDD = IDD_SENDBYTES };
	int		m_iByte;
	CString	m_szList;
	//}}AFX_DATA

	CCNCComms* m_pCNCComms;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendBytesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSendBytesDlg)
	afx_msg void OnSend();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDBYTESDLG_H__3924F4A0_3080_11D8_86C3_9D288CFF5125__INCLUDED_)
