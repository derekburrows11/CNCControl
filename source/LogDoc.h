#if !defined(AFX_LOGDOC_H__870138E2_BDEE_11D8_86C3_0008A15E291C__INCLUDED_)
#define AFX_LOGDOC_H__870138E2_BDEE_11D8_86C3_0008A15E291C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LogDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogDoc document

class CLogDoc : public CDocument
{
protected:
	CLogDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLogDoc)

// Attributes
public:

// Operations
public:
	void LogEvent(const char* str);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual BOOL OnNewDocument();
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLogDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CLogDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGDOC_H__870138E2_BDEE_11D8_86C3_0008A15E291C__INCLUDED_)
