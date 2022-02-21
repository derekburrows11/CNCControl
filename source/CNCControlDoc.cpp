// CNCControlDoc.cpp : implementation of the CCNCControlDoc class
//

#include "stdafx.h"
#include "CNCControl.h"

#include "CNCControlDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCNCControlDoc

IMPLEMENT_DYNCREATE(CCNCControlDoc, CDocument)

BEGIN_MESSAGE_MAP(CCNCControlDoc, CDocument)
	//{{AFX_MSG_MAP(CCNCControlDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCNCControlDoc construction/destruction

CCNCControlDoc::CCNCControlDoc()
{
	// TODO: add one-time construction code here

}

CCNCControlDoc::~CCNCControlDoc()
{
}

BOOL CCNCControlDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CCNCControlDoc serialization

void CCNCControlDoc::Serialize(CArchive& ar)
{
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
// CCNCControlDoc diagnostics

#ifdef _DEBUG
void CCNCControlDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCNCControlDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCNCControlDoc commands
