// CNCControlView.cpp : implementation of the CCNCControlView class
//

#include "stdafx.h"
#include "CNCControl.h"

#include "CNCControlDoc.h"
#include "CNCControlView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCNCControlView

IMPLEMENT_DYNCREATE(CCNCControlView, CView)

BEGIN_MESSAGE_MAP(CCNCControlView, CView)
	//{{AFX_MSG_MAP(CCNCControlView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCNCControlView construction/destruction

CCNCControlView::CCNCControlView()
{
	// TODO: add construction code here

}

CCNCControlView::~CCNCControlView()
{
}

BOOL CCNCControlView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCNCControlView drawing

void CCNCControlView::OnDraw(CDC* /* pDC */)
{
	CCNCControlDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CCNCControlView printing

BOOL CCNCControlView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCNCControlView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCNCControlView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CCNCControlView diagnostics

#ifdef _DEBUG
void CCNCControlView::AssertValid() const
{
	CView::AssertValid();
}

void CCNCControlView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCNCControlDoc* CCNCControlView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCNCControlDoc)));
	return (CCNCControlDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCNCControlView message handlers
