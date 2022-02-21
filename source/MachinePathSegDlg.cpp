// MachinePathSegDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"
#include "Settings.h"
#include "MachinePathSegDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMachinePathSegDlg dialog


CMachinePathSegDlg::CMachinePathSegDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMachinePathSegDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMachinePathSegDlg)
	m_tStep = 0.0;
	m_posTol = 0.0;
	m_iMethod = -1;
	m_wtPos = 0.0;
	m_wtVel = 0.0;
	m_wtAcc = 0.0;
	m_bAllowAccStep = FALSE;
	m_wtPV = 0.0;
	m_wtVA = 0.0;
	m_wtPA = 0.0;
	//}}AFX_DATA_INIT
}


void CMachinePathSegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMachinePathSegDlg)
	DDX_Text(pDX, IDC_EDIT1, m_tStep);
	DDV_MinMaxDouble(pDX, m_tStep, 0., 1000.);
	DDX_Text(pDX, IDC_EDIT2, m_posTol);
	DDV_MinMaxDouble(pDX, m_posTol, 0., 100.);
	DDX_Radio(pDX, IDC_RADIO1, m_iMethod);
	DDX_Text(pDX, IDC_EDIT3, m_wtPos);
	DDX_Text(pDX, IDC_EDIT4, m_wtVel);
	DDX_Text(pDX, IDC_EDIT5, m_wtAcc);
	DDX_Check(pDX, IDC_ALLOWACCSTEP, m_bAllowAccStep);
	DDX_Text(pDX, IDC_EDIT6, m_wtPV);
	DDX_Text(pDX, IDC_EDIT7, m_wtVA);
	DDX_Text(pDX, IDC_EDIT8, m_wtPA);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMachinePathSegDlg, CDialog)
	//{{AFX_MSG_MAP(CMachinePathSegDlg)
	ON_BN_CLICKED(ID_DEFAULT, OnDefault)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT3, IDC_EDIT11, OnChangeEdit)
END_MESSAGE_MAP()



// set dialog data from g_Settings

void CMachinePathSegDlg::LoadDialogData(SMachinePathSegments& mps, SControllerTracker& ct)
{
	m_iMethod = mps.iMethod;
	m_tStep   = mps.SegmentTime;
	m_posTol  = mps.PosTol;

	m_wtPos = ct.wtPos;
	m_wtVel = ct.wtVel;
	m_wtAcc = ct.wtAcc;

	m_wtPV = ct.wtPV;
	m_wtPA = ct.wtPA;
	m_wtVA = ct.wtVA;

	m_bAllowAccStep = ct.bAllowAccStep;
}

void CMachinePathSegDlg::SaveDialogData() const
{
	SMachinePathSegments& mps = g_Settings.MachPathSeg;
	SControllerTracker& ct = g_Settings.ContTrack;
	mps.iMethod     = m_iMethod;
	mps.SegmentTime = m_tStep;
	mps.PosTol      = m_posTol;

	ct.wtPos = m_wtPos;
	ct.wtVel = m_wtVel;
	ct.wtAcc = m_wtAcc;

	ct.wtPV = m_wtPV;
	ct.wtPA = m_wtPA;
	ct.wtVA = m_wtVA;

	ct.bAllowAccStep = m_bAllowAccStep != 0;
}


/////////////////////////////////////////////////////////////////////////////
// CMachinePathSegDlg message handlers

void CMachinePathSegDlg::OnChangeEdit(UINT /*nID*/) 
{
	UpdateData();

	m_wtAcc = 1 - m_wtPos - m_wtVel;

	UpdateData(false);
}

void CMachinePathSegDlg::OnDefault() 
{
	SMachinePathSegments mps;
	SControllerTracker ct;
	mps.Defaults();
	ct.Defaults();
	LoadDialogData(mps, ct);		// set members to default values in mps & ct
	UpdateData(false);
}
