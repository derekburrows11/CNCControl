// GeneralOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"
#include "GeneralOptionsDlg.h"

#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeneralOptionsDlg dialog


CGeneralOptionsDlg::CGeneralOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGeneralOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGeneralOptionsDlg)
	//}}AFX_DATA_INIT
}


void CGeneralOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneralOptionsDlg)
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_SMOOTH_SEGS, m_SmoothSegs.bSmooth);
	DDX_Text(pDX, IDC_EDIT_SEGTOL, m_SmoothSegs.segTol);
	DDX_Text(pDX, IDC_EDIT_SEGTOLDIRSET, m_SmoothSegs.segTolDirSet);
	DDX_Text(pDX, IDC_EDIT_SEGSTRAIGHTMIN, m_SmoothSegs.segStraightMin);
	DDX_Text(pDX, IDC_EDIT_SEGANGCUSP, m_SmoothSegs.angCusp);

	DDX_Check(pDX, IDC_CHECK_ADD_JOINING_TABS, m_JoinTab.bAdd);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_JoinTab.height);
	DDX_Text(pDX, IDC_EDIT_LOW_PASS_MAX, m_JoinTab.zLowPassMax);
	DDX_Text(pDX, IDC_EDIT_RAMP_LENGTH, m_JoinTab.rampLength);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_JoinTab.width);
}


BEGIN_MESSAGE_MAP(CGeneralOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CGeneralOptionsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeneralOptionsDlg message handlers

void CGeneralOptionsDlg::LoadData()
{
	m_SmoothSegs = g_Settings.PathOptions.m_SmoothSegs;
	m_JoinTab = g_Settings.PathOptions.m_JoinTab;


}

void CGeneralOptionsDlg::SaveData()
{
	g_Settings.PathOptions.m_SmoothSegs = m_SmoothSegs;
	g_Settings.PathOptions.m_JoinTab = m_JoinTab;
}



