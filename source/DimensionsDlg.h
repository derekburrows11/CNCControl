#if !defined(AFX_DIMENSIONSDLG_H__976CFCBC_3AFF_474C_B2D9_1ADF36458339__INCLUDED_)
#define AFX_DIMENSIONSDLG_H__976CFCBC_3AFF_474C_B2D9_1ADF36458339__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DimensionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDimensionsDlg dialog

class CDimensionsDlg : public CDialog
{
// Construction
public:
	CDimensionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDimensionsDlg)
	enum { IDD = IDD_DIMENSIONS };
	double	m_dia1;
	double	m_dia2;
	double	m_dia3;
	double	m_dia4;
	CString	m_strHeading;
	CString	m_strVal1;
	CString	m_strVal2;
	CString	m_strVal3;
	CString	m_strVal4;
	double	m_finishDepth;
	double	m_finishRadius;
	BOOL	m_bFinishCut;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDimensionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDimensionsDlg)
	afx_msg void OnDefault();
	afx_msg void OnCheckFinishcut();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIMENSIONSDLG_H__976CFCBC_3AFF_474C_B2D9_1ADF36458339__INCLUDED_)
