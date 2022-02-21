#if !defined(AFX_PORTSELDLG_H)
#define AFX_PORTSELDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// PortSelDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPortSelDlg dialog

class CPortSelDlg : public CDialog
{
// Construction
public:
	CPortSelDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPortSelDlg)
	enum { IDD = IDD_PORTSELECT };
	int		m_iPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPortSelDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPortSelDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTSELDLG_H)

