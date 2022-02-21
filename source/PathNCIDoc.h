#if !defined(AFX_PATHNCIDOC_H__E14BDCA2_546E_11D7_86C3_BB33B0897726__INCLUDED_)
#define AFX_PATHNCIDOC_H__E14BDCA2_546E_11D7_86C3_BB33B0897726__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PathNCIDoc.h : header file
//

//#include <afxtempl.h>


#include "PathDoc.h"

class CPolyCurveFit;




/////////////////////////////////////////////////////////////////////////////
// CPathNCIDoc document

class CPathNCIDoc : public CPathDoc
{
protected:
	CPathNCIDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPathNCIDoc)

// Attributes
protected:

	CPolyCurveFit* m_pCurveFitter;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPathNCIDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPathNCIDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void SmoothCurves();
	void FindCurve(SCurveInfo& ci);


	// Generated message map functions
protected:
	//{{AFX_MSG(CPathNCIDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATHNCIDOC_H__E14BDCA2_546E_11D7_86C3_BB33B0897726__INCLUDED_)
