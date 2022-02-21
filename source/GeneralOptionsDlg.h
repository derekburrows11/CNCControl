#if !defined(AFX_GENERALOPTIONSDLG_H__66FE904F_2952_4364_BEF2_A0D162186637__INCLUDED_)
#define AFX_GENERALOPTIONSDLG_H__66FE904F_2952_4364_BEF2_A0D162186637__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GeneralOptionsDlg.h : header file
//

#include "SettingsData.h"





/////////////////////////////////////////////////////////////////////////////
// CGeneralOptionsDlg dialog

class CGeneralOptionsDlg : public CDialog
{
// Construction
public:
	CGeneralOptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGeneralOptionsDlg)
	enum { IDD = IDD_GENERALOPTIONS };
	SSmoothSegsData m_SmoothSegs;
	SJoiningTabData m_JoinTab;
	//}}AFX_DATA

	void LoadData();
	void SaveData();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeneralOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGeneralOptionsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERALOPTIONSDLG_H__66FE904F_2952_4364_BEF2_A0D162186637__INCLUDED_)
