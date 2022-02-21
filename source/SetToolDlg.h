#if !defined(AFX_SETTOOL_H__66A1B084_DCDB_4A48_A702_6EA60B104A05__INCLUDED_)
#define AFX_SETTOOL_H__66A1B084_DCDB_4A48_A702_6EA60B104A05__INCLUDED_





#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SetToolDlg.h : header file
//



#include "SettingsData.h"


/////////////////////////////////////////////////////////////////////////////
// CSetToolDlg dialog

class CSetToolDlg : public SMachineDimensionsData, public CDialog
{
// Construction
public:
	CSetToolDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetToolDlg)
	enum { IDD = IDD_SETTOOL };
	double	m_dStockTop2Tip;		// calc'd
	double	m_dBaseTop2Tip;		// calc'd
	double	m_zAxisPos;				// given
	double	m_dBase2ZAxisHead;	// calc'd
	double	m_dProbeReading;		// calc'd
	BOOL	m_bLockFixedValues;
	//}}AFX_DATA

	double m_bGotzAxisPos;


public:


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetToolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void CalcValues();
	void CalcToolLength();
	void CalcFromProbe();
	void SetLockedEdits();

	// Generated message map functions
	//{{AFX_MSG(CSetToolDlg)
	afx_msg void OnChangeStockTop2Tip();
	afx_msg void OnChangeBaseTop2Tip();
	afx_msg void OnChangeProbe();
	afx_msg void OnCheckLockFixedValues();
	afx_msg void OnCheckProbeOnStock();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	afx_msg void OnChangeValue(UINT nID);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTOOL_H__66A1B084_DCDB_4A48_A702_6EA60B104A05__INCLUDED_)
