// DimensionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"
#include "DimensionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDimensionsDlg dialog


CDimensionsDlg::CDimensionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDimensionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDimensionsDlg)
	m_dia1 = 0.0;
	m_dia2 = 0.0;
	m_dia3 = 0.0;
	m_dia4 = 0.0;
	m_strHeading = _T("");
	m_strVal1 = _T("");
	m_strVal2 = _T("");
	m_strVal3 = _T("");
	m_strVal4 = _T("");
	m_finishDepth = 0.1;
	m_finishRadius = 0.1;
	m_bFinishCut = FALSE;
	//}}AFX_DATA_INIT
}


void CDimensionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDimensionsDlg)
	DDX_Text(pDX, IDC_EDIT_DIA1, m_dia1);
	DDX_Text(pDX, IDC_EDIT_DIA2, m_dia2);
	DDX_Text(pDX, IDC_EDIT_DIA3, m_dia3);
	DDX_Text(pDX, IDC_EDIT_DIA4, m_dia4);
	DDX_Text(pDX, IDC_STATIC_HEADING, m_strHeading);
	DDX_Text(pDX, IDC_STATIC_VAL1, m_strVal1);
	DDX_Text(pDX, IDC_STATIC_VAL2, m_strVal2);
	DDX_Text(pDX, IDC_STATIC_VAL3, m_strVal3);
	DDX_Text(pDX, IDC_STATIC_VAL4, m_strVal4);
	DDX_Text(pDX, IDC_EDIT_FINISHDEPTH, m_finishDepth);
	DDX_Text(pDX, IDC_EDIT_FINISHRADIUS, m_finishRadius);
	DDX_Check(pDX, IDC_CHECK_FINISHCUT, m_bFinishCut);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDimensionsDlg, CDialog)
	//{{AFX_MSG_MAP(CDimensionsDlg)
	ON_BN_CLICKED(ID_DEFAULT, OnDefault)
	ON_BN_CLICKED(IDC_CHECK_FINISHCUT, OnCheckFinishcut)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDimensionsDlg message handlers

void CDimensionsDlg::OnDefault() 
{
	UpdateData(false);		// set controls from unchanged members
}

void CDimensionsDlg::OnCheckFinishcut() 
{
//	UpdateData();
	BOOL bFinish = ((CButton*)GetDlgItem(IDC_CHECK_FINISHCUT))->GetCheck();
	GetDlgItem(IDC_EDIT_FINISHDEPTH)->EnableWindow(bFinish);
	GetDlgItem(IDC_EDIT_FINISHRADIUS)->EnableWindow(bFinish);
	GetDlgItem(IDC_STATIC_FINISHDEPTH)->EnableWindow(bFinish);
	GetDlgItem(IDC_STATIC_FINISHRADIUS)->EnableWindow(bFinish);
	
}

BOOL CDimensionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	OnCheckFinishcut();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
