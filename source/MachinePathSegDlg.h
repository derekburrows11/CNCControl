#if !defined(AFX_MACHINEPATHSEGDLG_H__C7165B04_94B9_11D6_86C3_93275B2D8426__INCLUDED_)
#define AFX_MACHINEPATHSEGDLG_H__C7165B04_94B9_11D6_86C3_93275B2D8426__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MachinePathSegDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMachinePathSegDlg dialog

class CMachinePathSegDlg : public CDialog
{
// Construction
public:
	CMachinePathSegDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMachinePathSegDlg)
	enum { IDD = IDD_MACHINEPATHSEGMENTS };
	double	m_tStep;
	double	m_posTol;
	int		m_iMethod;
	double	m_wtPos;
	double	m_wtVel;
	double	m_wtAcc;
	BOOL	m_bAllowAccStep;
	double	m_wtPV;
	double	m_wtVA;
	double	m_wtPA;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMachinePathSegDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void LoadDialogData() { LoadDialogData(g_Settings.MachPathSeg, g_Settings.ContTrack); }
	void LoadDialogData(SMachinePathSegments& mps, SControllerTracker& ct);
	void SaveDialogData() const;

protected:

	// Generated message map functions
	void OnChangeEdit(UINT nID);
	//{{AFX_MSG(CMachinePathSegDlg)
	afx_msg void OnDefault();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACHINEPATHSEGDLG_H__C7165B04_94B9_11D6_86C3_93275B2D8426__INCLUDED_)
