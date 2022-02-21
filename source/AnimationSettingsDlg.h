#if !defined(AFX_ANIMATIONSETTINGSDLG_H__CF297342_95AD_11D7_86C3_83F5190B4E25__INCLUDED_)
#define AFX_ANIMATIONSETTINGSDLG_H__CF297342_95AD_11D7_86C3_83F5190B4E25__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AnimationSettingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimationSettingsDlg dialog

class CAnimationSettingsDlg : public CDialog
{
// Construction
public:
	CAnimationSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnimationSettingsDlg)
	enum { IDD = IDD_ANIMATIONSETTINGS };
	double	m_Rate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimationSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimationSettingsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATIONSETTINGSDLG_H__CF297342_95AD_11D7_86C3_83F5190B4E25__INCLUDED_)
