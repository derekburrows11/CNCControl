#if !defined(AFX_DIAGNOSEDLG_H__50866B65_EDE4_11D4_8C1E_8569E8ED302C__INCLUDED_)
#define AFX_DIAGNOSEDLG_H__50866B65_EDE4_11D4_8C1E_8569E8ED302C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DiagnoseDlg.h : header file
//

#include "CNCComms.h"

/////////////////////////////////////////////////////////////////////////////
// CDiagnoseDlg dialog

class CDiagnoseDlg : public CDialog
{
// Construction
public:
	CDiagnoseDlg(CWnd* pParent = NULL);   // standard constructor


// Dialog Data
protected:
	//{{AFX_DATA(CDiagnoseDlg)
	enum { IDD = IDD_DIAGNOSE };
	int		m_iAddrStyle;
	int		m_iValueStyle;
	BOOL	m_bIncReg;
	BOOL	m_bIncRegBank;
	BOOL	m_bIncMemAddr;
	CString	m_strRegBank;
	CString	m_strRegValue;
	CString	m_strMemAddr;
	CString	m_strMemValue;
	CString	m_strRegAddr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiagnoseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
public:
	CCNCComms* m_pCNCComms;

protected:
	bool m_bMemValueValid;
	int m_iMemWordSize;
	int m_iMemAddr;
	int m_iMemValue;

	bool m_bRegValueValid;
	int m_iRegWordSize;
	int m_iRegBank;
	int m_iRegAddr;
	int m_iRegValue;

public:
	int RxMemValue(int iAddr, int iBytes, int iValue);
	int RxRegValue(int iReg, int iBytes, int iValue);

protected:
	virtual void OnOK();
	void InterperateValues();
	void ShowValues();
	int ScanValue(const char* sz, int* pVal, int iStyle = 0);
	void PrintValue(char* sz, int iVal, int iStyle = 0, int iMinWidth = 1);

	// Generated message map functions
	//{{AFX_MSG(CDiagnoseDlg)
	afx_msg void OnSetBytesMem();
	afx_msg void OnRequestMem();
	afx_msg void OnSendMem();
	afx_msg void OnSetBytesReg();
	afx_msg void OnRequestReg();
	afx_msg void OnSendReg();
	afx_msg void OnChangeStyle();
	//}}AFX_MSG
	afx_msg void OnSetfocusEdit(UINT nID);
	afx_msg void OnKillfocusEdit(UINT nID);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIAGNOSEDLG_H__50866B65_EDE4_11D4_8C1E_8569E8ED302C__INCLUDED_)
