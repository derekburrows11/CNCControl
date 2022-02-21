// MachinePropDlg.cpp : implementation file
//

#include "stdafx.h"

#include "cnccontrol.h"
#include "Settings.h"
#include "ControllerTracker.h"
#include "PosConverter.h"

#include "MachinePropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



void DDX_TextFormat(CDataExchange* pDX, int nIDC, double& value, int nSizeGcvt);
void DDX_TextFormat(CDataExchange* pDX, int nIDC, float& value, int nSizeGcvt);

void DDX_TextFormat(CDataExchange* pDX, int nIDC, double& value, int nSizeGcvt)
{
	if (pDX->m_bSaveAndValidate)
		DDX_Text(pDX, nIDC, value);
	else
	{
		HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
		TCHAR szBuffer[32];
		_stprintf(szBuffer, _T("%.*g"), nSizeGcvt, value);
//		_stprintf(szBuffer, _T("%g"), value);  nSizeGcvt++;
		::SetWindowText(hWndCtrl, szBuffer);
	}
}

void DDX_TextFormat(CDataExchange* pDX, int nIDC, float& value, int nSizeGcvt)
{
	if (pDX->m_bSaveAndValidate)
		DDX_Text(pDX, nIDC, value);
	else
	{
		HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
		TCHAR szBuffer[32];
		_stprintf(szBuffer, _T("%.*g"), nSizeGcvt, (double)value);
//		_stprintf(szBuffer, _T("%g"), (double)value);  nSizeGcvt++;
		::SetWindowText(hWndCtrl, szBuffer);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMachinePropDlg dialog

CMachinePropDlg::CMachinePropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMachinePropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMachinePropDlg)
	m_dt = 0.0;
	m_dAcc = 0.0;
	m_dVel = 0.0;
	m_dPos = 0.0;
	m_JerkX = 0.0;
	m_JerkY = 0.0;
	m_JerkZ = 0.0;
	m_AccX = 0.0;
	m_AccY = 0.0;
	m_AccZ = 0.0;
	m_VelX = 0.0;
	m_VelY = 0.0;
	m_VelZ = 0.0;
	m_SpeedXY = 0.0;
	m_SpeedXYZ = 0.0;
	m_AngCusp = 0.0;
	m_FeedRate = 0.0;
	m_iCountsPerRev = 0;
	m_dPosPerRev = 0.0;
	m_iPosTrackFractionBits = 0;
	m_bUsePosCorr = FALSE;
	m_bUsePosCorrX = FALSE;
	m_bUsePosCorrY = FALSE;
	m_bUsePosCorrZ = FALSE;
	m_bUseBacklashCorr = FALSE;
	//}}AFX_DATA_INIT
}


void CMachinePropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMachinePropDlg)
	DDX_Text(pDX, IDC_EDIT_RESTIME, m_dt);
	DDX_Text(pDX, IDC_EDIT_JERKX, m_JerkX);
	DDX_Text(pDX, IDC_EDIT_JERKY, m_JerkY);
	DDX_Text(pDX, IDC_EDIT_JERKZ, m_JerkZ);
	DDX_Text(pDX, IDC_EDIT_ACCX, m_AccX);
	DDX_Text(pDX, IDC_EDIT_ACCY, m_AccY);
	DDX_Text(pDX, IDC_EDIT_ACCZ, m_AccZ);
	DDX_Text(pDX, IDC_EDIT_VELX, m_VelX);
	DDX_Text(pDX, IDC_EDIT_VELY, m_VelY);
	DDX_Text(pDX, IDC_EDIT_VELZ, m_VelZ);
	DDX_Text(pDX, IDC_EDIT_SPEEDXY, m_SpeedXY);
	DDX_Text(pDX, IDC_EDIT_SPEEDXYZ, m_SpeedXYZ);
	DDX_Text(pDX, IDC_EDIT_CUSPANGLE, m_AngCusp);
	DDX_Text(pDX, IDC_EDIT_FEEDRATE, m_FeedRate);
	DDX_Text(pDX, IDC_EDIT_COUNTSPERREV, m_iCountsPerRev);
	DDX_Text(pDX, IDC_EDIT_DPOSPERREV, m_dPosPerRev);
	DDX_Text(pDX, IDC_EDIT_POSTRACKFRACTBITS, m_iPosTrackFractionBits);
	DDX_Check(pDX, IDC_CHECK_USE_POS_CORR, m_bUsePosCorr);
	DDX_Check(pDX, IDC_CHECK_USE_POS_CORRX, m_bUsePosCorrX);
	DDX_Check(pDX, IDC_CHECK_USE_POS_CORRY, m_bUsePosCorrY);
	DDX_Check(pDX, IDC_CHECK_USE_POS_CORRZ, m_bUsePosCorrZ);
	DDX_Check(pDX, IDC_CHECK_USE_BL_CORR, m_bUseBacklashCorr);
	//}}AFX_DATA_MAP

	DDX_TextFormat(pDX, IDC_EDIT_RESACC, m_dAcc, 6);
	DDX_TextFormat(pDX, IDC_EDIT_RESVEL, m_dVel, 6);
	DDX_TextFormat(pDX, IDC_EDIT_RESPOS, m_dPos, 6);

	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		DDX_Text(pDX, IDC_EDIT_BLPOSX+ax, m_arBacklashPositive[ax]);
		DDX_Text(pDX, IDC_EDIT_BLNEGX+ax, m_arBacklashNegative[ax]);
	}

}

BEGIN_MESSAGE_MAP(CMachinePropDlg, CDialog)
	//{{AFX_MSG_MAP(CMachinePropDlg)
	ON_BN_CLICKED(ID_DEFAULT, OnDefault)
	ON_BN_CLICKED(IDC_CHECK_USE_POS_CORR, OnCheckUsePosCorr)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT1, IDC_EDIT4, OnChangeEdit)
END_MESSAGE_MAP()

// set dialog data from g_Settings
void CMachinePropDlg::SetData(const SMachineParameters& mp)
{
	m_dt = mp.dTime;
	m_dPosPerRev = mp.dPosPerRev;
	m_iCountsPerRev = mp.iCountsPerRev;
	m_iPosTrackFractionBits = mp.iPosTrackFractionBits;

	m_dPos = mp.dPosTrack;
	m_dVel = mp.dVelTrack;
	m_dAcc = mp.dAccTrack;

	mp.vtJerkMax.Get(m_JerkX, m_JerkY, m_JerkZ);
	mp.vtAccMax.Get(m_AccX, m_AccY, m_AccZ);
	mp.vtVelMax.Get(m_VelX, m_VelY, m_VelZ);
	m_SpeedXY  = mp.SpeedXYMax;
	m_SpeedXYZ = mp.SpeedXYZMax;
	m_FeedRate = mp.FeedRateMax;
	m_AngCusp = mp.AngCuspSegments;

	m_bUsePosCorr = g_utPosConvert.UsingPosCorrection();
	g_utPosConvert.UsingPosCorrection(m_bUsePosCorrX, m_bUsePosCorrY, m_bUsePosCorrZ);
}

void CMachinePropDlg::GetData(SMachineParameters& mp) const
{
	mp.dTime = m_dt;
	mp.dPosPerRev = m_dPosPerRev;
	mp.iCountsPerRev = m_iCountsPerRev;
	mp.iPosTrackFractionBits = m_iPosTrackFractionBits;

	mp.vtJerkMax.Set(m_JerkX, m_JerkY, m_JerkZ);
	mp.vtAccMax.Set(m_AccX, m_AccY, m_AccZ);
	mp.vtVelMax.Set(m_VelX, m_VelY, m_VelZ);
	mp.SpeedXYMax  = m_SpeedXY;
	mp.SpeedXYZMax = m_SpeedXYZ;
	mp.FeedRateMax = m_FeedRate;
	mp.AngCuspSegments = m_AngCusp;

	mp.CalculateAfterLoad();	// sets mp.dPosTrack etc

	g_utPosConvert.UsePosCorrection(m_bUsePosCorr);
	g_utPosConvert.UsePosCorrection(m_bUsePosCorrX, m_bUsePosCorrY, m_bUsePosCorrZ);
}


/////////////////////////////////////////////////////////////////////////////
// CMachinePropDlg message handlers

void CMachinePropDlg::OnChangeEdit(UINT /*nID*/)
{
	UpdateData(true);
	SMachineParameters mp;
	GetData(mp);		// will calculate some values
	SetData(mp);
	UpdateData(false);
}

void CMachinePropDlg::OnDefault()
{
	SMachineParameters mp;
	mp.Defaults();
	SetData(mp);		// set members to default values in mp
	m_bUsePosCorr = true;
	UpdateData(false);
}

void CMachinePropDlg::OnCheckUsePosCorr()
{
	UpdateData();
	GetDlgItem(IDC_CHECK_USE_POS_CORRX)->EnableWindow(m_bUsePosCorr);
	GetDlgItem(IDC_CHECK_USE_POS_CORRY)->EnableWindow(m_bUsePosCorr);
	GetDlgItem(IDC_CHECK_USE_POS_CORRZ)->EnableWindow(m_bUsePosCorr);
}
