#if !defined(AFX_GRIDCTRL_H__EF823E62_6EA0_11D4_8C1E_901544B20C2F__INCLUDED_)
#define AFX_GRIDCTRL_H__EF823E62_6EA0_11D4_8C1E_901544B20C2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GridCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl window

class CGridCtrl : public CListCtrl
{
// Construction
public:
	CGridCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGridCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGridCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDCTRL_H__EF823E62_6EA0_11D4_8C1E_901544B20C2F__INCLUDED_)
