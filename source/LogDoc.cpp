// LogDoc.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"
#include "LogDoc.h"

#include "ThreadMessages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogDoc

IMPLEMENT_DYNCREATE(CLogDoc, CDocument)

CLogDoc::CLogDoc()
{
}

BOOL CLogDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CLogDoc::~CLogDoc()
{
}


BEGIN_MESSAGE_MAP(CLogDoc, CDocument)
	//{{AFX_MSG_MAP(CLogDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogDoc diagnostics

#ifdef _DEBUG
void CLogDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLogDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLogDoc serialization

void CLogDoc::Serialize(CArchive& ar)
{
	((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);

	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLogDoc commands


void CLogDoc::LogEvent(const char* str)
{
	CEdit& edit = ((CEditView*)m_viewList.GetHead())->GetEditCtrl();
	edit.SetSel(-2, -2);			// anything large sets to end except -1
	edit.ReplaceSel((char*)str);	// return is "\r\n"
}

BOOL CLogDoc::SaveModified() 
{
	// TODO: Add your specialized code here and/or call the base class
	SetModifiedFlag(false);	
	return CDocument::SaveModified();
}

BOOL CLogDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDocument::CanCloseFrame(pFrame);
}
