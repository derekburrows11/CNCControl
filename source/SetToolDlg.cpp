// SetTool.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"

#include "SetToolDlg.h"

#include "DlgUtils.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// SMachineDimensionsData

double SMachineDimensionsData::GetTip2ZAxisHeadDist()
{
	return m_dTip2Chuck + m_dChuck2Mount + m_dMount2ZAxisHead;
}

double SMachineDimensionsData::GetAxisPosTipAtBaseTop()
{
	return GetTip2ZAxisHeadDist() + m_dBaseBoard + m_dVacuumBoard - m_zAxisAdjust;
}



/////////////////////////////////////////////////////////////////////////////
// CSetToolDlg dialog


CSetToolDlg::CSetToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetToolDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetToolDlg)
	m_dStockTop2Tip = 0;
	m_dBaseTop2Tip = 0;
	m_zAxisPos = 0;
	m_dBase2ZAxisHead = 0;
	m_dProbeReading = 0;
	m_dMount2ZAxisHead = 0;
	m_dChuck2Mount = 0;
	m_dTip2Chuck = 0;
	m_dVacuumBoard = 0;
	m_dBaseBoard = 0;
	m_zAxisAdjust = 0;
	m_dProbeAdjust = 0;
	m_diaTool = 0;
	m_bLockFixedValues = TRUE;
	m_bProbeOnStock = FALSE;
	//}}AFX_DATA_INIT

	m_bGotzAxisPos = false;
}


void CSetToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	if (!pDX->m_bSaveAndValidate)		// calc some values when initializing controls
		CalcValues();

	//{{AFX_DATA_MAP(CSetToolDlg)
	DDX_Check(pDX, IDC_CHECK_LOCKFIXEDVALUES, m_bLockFixedValues);
	DDX_Check(pDX, IDC_CHECK_PROBEONSTOCK, m_bProbeOnStock);
	//}}AFX_DATA_MAP

	DDX_Text(pDX, IDC_EDIT_MOUNT2ZAXISHEAD, m_dMount2ZAxisHead);
	DDX_Text(pDX, IDC_EDIT_CHUCK2MOUNT, m_dChuck2Mount);
	DDX_Text(pDX, IDC_EDIT_TIP2CHUCK, m_dTip2Chuck);
	DDX_TextFix(pDX, IDC_EDIT_STOCKTOP2TIP, m_dStockTop2Tip, 3);
	DDX_TextFix(pDX, IDC_EDIT_BASETOP2TIP, m_dBaseTop2Tip, 3);
	DDX_Text(pDX, IDC_EDIT_VACUUMBOARD, m_dVacuumBoard);
	DDX_Text(pDX, IDC_EDIT_BASEBOARD, m_dBaseBoard);

	DDX_TextFix(pDX, IDC_EDIT_AXISPOS, m_zAxisPos, 3);
	DDX_Text(pDX, IDC_EDIT_AXISADJUST, m_zAxisAdjust);
	DDX_TextFix(pDX, IDC_EDIT_BASE2ZAXISHEAD, m_dBase2ZAxisHead, 3);

	DDX_TextFix(pDX, IDC_EDIT_PROBEREADING, m_dProbeReading, 3);
	DDX_Text(pDX, IDC_EDIT_PROBEADJUST, m_dProbeAdjust);
	DDX_Text(pDX, IDC_EDIT_STOCK, m_dStock);

	DDX_Text(pDX, IDC_EDIT_DIATOOL, m_diaTool);

}


BEGIN_MESSAGE_MAP(CSetToolDlg, CDialog)
	//{{AFX_MSG_MAP(CSetToolDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_STOCKTOP2TIP, OnChangeStockTop2Tip)
	ON_EN_KILLFOCUS(IDC_EDIT_BASETOP2TIP, OnChangeBaseTop2Tip)
	ON_EN_KILLFOCUS(IDC_EDIT_PROBEREADING, OnChangeProbe)
	ON_BN_CLICKED(IDC_CHECK_LOCKFIXEDVALUES, OnCheckLockFixedValues)
	ON_BN_CLICKED(IDC_CHECK_PROBEONSTOCK, OnCheckProbeOnStock)
	ON_EN_KILLFOCUS(IDC_EDIT_PROBEADJUST, OnChangeProbe)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_MOUNT2ZAXISHEAD, IDC_EDIT_STOCKTOP2TIP, OnChangeValue)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetToolDlg message handlers

BOOL CSetToolDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetLockedEdits();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSetToolDlg::OnChangeValue(UINT /*nID*/)
{
	UpdateData();
	UpdateData(false);
}

void CSetToolDlg::OnChangeStockTop2Tip()
{
	UpdateData();
	m_dBaseTop2Tip = m_dStockTop2Tip + m_dStock;
	CalcToolLength();
	UpdateData(false);
}

void CSetToolDlg::OnChangeBaseTop2Tip()
{
	UpdateData();
	CalcToolLength();
	UpdateData(false);
}

void CSetToolDlg::OnChangeProbe()
{
	UpdateData();
	CalcFromProbe();
	UpdateData(false);
}

void CSetToolDlg::OnCheckLockFixedValues() 
{
	UpdateData();
	SetLockedEdits();
}

void CSetToolDlg::OnCheckProbeOnStock() 
{
	UpdateData();
	UpdateData(false);
}

///////////////////////////////


void CSetToolDlg::CalcValues()	// after change in a value
{
//	if (!m_bGotzAxisPos)
//		m_zAxisPos = 0;
	m_dBase2ZAxisHead = m_zAxisPos + m_zAxisAdjust;
	m_dBaseTop2Tip = m_dBase2ZAxisHead - m_dBaseBoard - m_dVacuumBoard
							- m_dTip2Chuck - m_dChuck2Mount - m_dMount2ZAxisHead;
	m_dStockTop2Tip = m_dBaseTop2Tip - m_dStock;

	if (m_bProbeOnStock)
		m_dProbeReading = m_dStockTop2Tip - m_dProbeAdjust;
	else
		m_dProbeReading = m_dBaseTop2Tip - m_dProbeAdjust;
}

void CSetToolDlg::CalcToolLength()		// after change in m_dBaseTop2Tip
{
	// adjusts tool length 'm_dTip2Chuck' to compensate
	m_dTip2Chuck = m_dBase2ZAxisHead - m_dBaseBoard - m_dVacuumBoard
							- m_dBaseTop2Tip - m_dChuck2Mount - m_dMount2ZAxisHead;
}

void CSetToolDlg::CalcFromProbe()	// after change in m_dProbeReading, m_dProbeAdjust
{
	// adjusts tool length 'm_dTip2Chuck' to compensate
	m_dBaseTop2Tip = m_dProbeReading + m_dProbeAdjust;
	if (m_bProbeOnStock)
		m_dBaseTop2Tip += m_dStock;
	CalcToolLength();
}

void CSetToolDlg::SetLockedEdits()
{
	// set readonly of a group of controls
	BOOL fRO = m_bLockFixedValues;
	HWND hDlg = m_hWnd;
	::SendMessage(::GetDlgItem(hDlg, IDC_EDIT_MOUNT2ZAXISHEAD), EM_SETREADONLY, fRO, 0);
	::SendMessage(::GetDlgItem(hDlg, IDC_EDIT_CHUCK2MOUNT), EM_SETREADONLY, fRO, 0);
	::SendMessage(::GetDlgItem(hDlg, IDC_EDIT_VACUUMBOARD), EM_SETREADONLY, fRO, 0);
	::SendMessage(::GetDlgItem(hDlg, IDC_EDIT_BASEBOARD), EM_SETREADONLY, fRO, 0);
	::SendMessage(::GetDlgItem(hDlg, IDC_EDIT_AXISADJUST), EM_SETREADONLY, fRO, 0);
	::SendMessage(::GetDlgItem(hDlg, IDC_EDIT_PROBEADJUST), EM_SETREADONLY, fRO, 0);
}




