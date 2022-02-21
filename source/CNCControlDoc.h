// CNCControlDoc.h : interface of the CCNCControlDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CNCCONTROLDOC_H__4E7108AC_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
#define AFX_CNCCONTROLDOC_H__4E7108AC_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CCNCControlDoc : public CDocument
{
protected: // create from serialization only
	CCNCControlDoc();
	DECLARE_DYNCREATE(CCNCControlDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCNCControlDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCNCControlDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCNCControlDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CNCCONTROLDOC_H__4E7108AC_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
