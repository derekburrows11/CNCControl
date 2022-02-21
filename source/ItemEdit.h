#if !defined(AFX_ITEMEDIT_H__94ACE88D_728A_11D4_8C1E_B72D6A6B0E2F__INCLUDED_)
#define AFX_ITEMEDIT_H__94ACE88D_728A_11D4_8C1E_B72D6A6B0E2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ItemEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CItemEdit window

class CItemEdit : public CEdit
{
// Construction
public:
	CItemEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CItemEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CItemEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CItemEdit)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

// Extra control notifications

// above the range for normal EN_ messages
#define EN_END			0x8000	// sent to parent when edit done (return usually)
#define EN_CANCEL		0x8001	// sent to parent when edit cancelled (escape)

/////////////////////////////////////////////////////////////////////////////



//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ITEMEDIT_H__94ACE88D_728A_11D4_8C1E_B72D6A6B0E2F__INCLUDED_)
