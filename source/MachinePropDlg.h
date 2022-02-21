#if !defined(AFX_MACHINEPROPDLG_H__024B81C0_2C8E_11D7_86C3_D55347623626__INCLUDED_)
#define AFX_MACHINEPROPDLG_H__024B81C0_2C8E_11D7_86C3_D55347623626__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MachinePropDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMachinePropDlg dialog

class CMachinePropDlg : public CDialog
{
// Construction
public:
	CMachinePropDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMachinePropDlg)
	enum { IDD = IDD_MACHINEPROPERTIES };
	double	m_dt;
	double	m_dAcc;
	double	m_dVel;
	double	m_dPos;
	double	m_JerkX;
	double	m_JerkY;
	double	m_JerkZ;
	double	m_AccX;
	double	m_AccY;
	double	m_AccZ;
	double	m_VelX;
	double	m_VelY;
	double	m_VelZ;
	double	m_SpeedXY;
	double	m_SpeedXYZ;
	double	m_AngCusp;
	double	m_FeedRate;
	int		m_iCountsPerRev;
	double	m_dPosPerRev;
	int		m_iPosTrackFractionBits;
	BOOL	m_bUsePosCorr;
	BOOL	m_bUsePosCorrX;
	BOOL	m_bUsePosCorrY;
	BOOL	m_bUsePosCorrZ;
	BOOL	m_bUseBacklashCorr;
	//}}AFX_DATA
	double	m_arBacklashPositive[NUM_AXIS];
	double	m_arBacklashNegative[NUM_AXIS];


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMachinePropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetData(const SMachineParameters& mp);
	void GetData(SMachineParameters& mp) const;

protected:

	// Generated message map functions
	//{{AFX_MSG(CMachinePropDlg)
	afx_msg void OnDefault();
	afx_msg void OnCheckUsePosCorr();
	//}}AFX_MSG
	void OnChangeEdit(UINT nID);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACHINEPROPDLG_H__024B81C0_2C8E_11D7_86C3_D55347623626__INCLUDED_)
