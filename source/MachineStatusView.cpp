// MachineStatusView.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"

#include "MachineStatusView.h"

#include "ParamDoc.h"
#include "MachineState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMachineStatusView

IMPLEMENT_DYNCREATE(CMachineStatusView, CFormView)

CMachineStatusView::CMachineStatusView()
	: CFormView(CMachineStatusView::IDD)
{
	//{{AFX_DATA_INIT(CMachineStatusView)
	m_AccSeg_TI = 0;
	m_AccSeg_TF = 0;
	m_iAccSeg_PSI = 0;
	m_iAccSeg_PSF = 0;
	m_iAccSeg_ASI = 0;
	m_iAccSeg_ASF = 0;
	m_Limit_TI = 0;
	m_Limit_TF = 0;
	m_iLimit_PSI = 0;
	m_iLimit_PSF = 0;
	m_Machine_TI = 0;
	m_Machine_TF = 0;
	m_iMachine_ASI = 0;
	m_iMachine_ASF = 0;
	m_strPos = _T("");
	m_strVel = _T("");
	m_strPosError = _T("");
	m_strPosTrack = _T("");
	m_strVelError = _T("");
	m_strVelTrack = _T("");
	m_PathTime = 0.0;
	m_Path_TI = 0;
	m_Path_TF = 0;
	m_iPath_PSI = 0;
	m_iPath_PSF = 0;
	m_iPathStep = 0;
	//}}AFX_DATA_INIT

}

CMachineStatusView::~CMachineStatusView()
{
}

void CMachineStatusView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMachineStatusView)
	DDX_Text(pDX, IDC_EDIT_ACCSEG_TI,  m_AccSeg_TI);
	DDX_Text(pDX, IDC_EDIT_ACCSEG_TF,  m_AccSeg_TF);
	DDX_Text(pDX, IDC_EDIT_ACCSEG_PSI, m_iAccSeg_PSI);
	DDX_Text(pDX, IDC_EDIT_ACCSEG_PSF, m_iAccSeg_PSF);
	DDX_Text(pDX, IDC_EDIT_ACCSEG_ASI, m_iAccSeg_ASI);
	DDX_Text(pDX, IDC_EDIT_ACCSEG_ASF, m_iAccSeg_ASF);
	DDX_Text(pDX, IDC_EDIT_LIMIT_TI,  m_Limit_TI);
	DDX_Text(pDX, IDC_EDIT_LIMIT_TF,  m_Limit_TF);
	DDX_Text(pDX, IDC_EDIT_LIMIT_PSI, m_iLimit_PSI);
	DDX_Text(pDX, IDC_EDIT_LIMIT_PSF, m_iLimit_PSF);
	DDX_Text(pDX, IDC_EDIT_MACHINE_TI,  m_Machine_TI);
	DDX_Text(pDX, IDC_EDIT_MACHINE_TF,  m_Machine_TF);
	DDX_Text(pDX, IDC_EDIT_MACHINE_ASI, m_iMachine_ASI);
	DDX_Text(pDX, IDC_EDIT_MACHINE_ASF, m_iMachine_ASF);
	DDX_Text(pDX, IDC_EDIT_MACHINE_POS, m_strPos);
	DDX_Text(pDX, IDC_EDIT_MACHINE_VEL, m_strVel);
	DDX_Text(pDX, IDC_EDIT_MACHINE_POSERROR, m_strPosError);
	DDX_Text(pDX, IDC_EDIT_MACHINE_POSTRACK, m_strPosTrack);
	DDX_Text(pDX, IDC_EDIT_MACHINE_VELERROR, m_strVelError);
	DDX_Text(pDX, IDC_EDIT_MACHINE_VELTRACK, m_strVelTrack);
	DDX_Text(pDX, IDC_EDIT_MACHINE_PATHTIME, m_PathTime);
	DDX_Text(pDX, IDC_EDIT_PATH_TI,  m_Path_TI);
	DDX_Text(pDX, IDC_EDIT_PATH_TF,  m_Path_TF);
	DDX_Text(pDX, IDC_EDIT_PATH_PSI, m_iPath_PSI);
	DDX_Text(pDX, IDC_EDIT_PATH_PSF, m_iPath_PSF);
	DDX_Text(pDX, IDC_EDIT_MACHINE_PATHSTEP, m_iPathStep);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMachineStatusView, CFormView)
	//{{AFX_MSG_MAP(CMachineStatusView)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMachineStatusView diagnostics

#ifdef _DEBUG
void CMachineStatusView::AssertValid() const
{
	CFormView::AssertValid();
}

void CMachineStatusView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CParamDoc* CMachineStatusView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CParamDoc)));
	return (CParamDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMachineStatusView message handlers

void CMachineStatusView::UpdateLimitBufferStatus(SContBufferSpan& span)
{
	if (m_hWnd == NULL)
		return;
	m_Limit_TI   = span.Init.Time;
	m_Limit_TF   = span.Final.Time;
	m_iLimit_PSI = span.Init.PathSeg;
	m_iLimit_PSF = span.Final.PathSeg;
	UpdateData(false);
}

void CMachineStatusView::UpdateAccSegBufferStatus(SContBufferSpan& span)
{
	if (m_hWnd == NULL)
		return;
	m_AccSeg_TI   = span.Init.Time;
	m_AccSeg_TF   = span.Final.Time;
	m_iAccSeg_PSI = span.Init.PathSeg;
	m_iAccSeg_PSF = span.Final.PathSeg;
	m_iAccSeg_ASI = span.Init.AccSeg;
	m_iAccSeg_ASF = span.Final.AccSeg;
	UpdateData(false);
}

void CMachineStatusView::UpdateMachineBufferStatus(SContBufferSpan& span)
{
	if (m_hWnd == NULL)
		return;
	m_Machine_TI   = span.Init.Time;
	m_Machine_TF   = span.Final.Time;
	m_iMachine_ASI = span.Init.AccSeg;
	m_iMachine_ASF = span.Final.AccSeg;
	UpdateData(false);
}

void CMachineStatusView::UpdateMachineState(CMachineState& ms)
{
	if (m_hWnd == NULL)
		return;
	const int STRLEN = 60;
	char* strPos = m_strPos.GetBufferSetLength(STRLEN);		// won't need ReleaseBuffer()
	char* strVel = m_strVel.GetBufferSetLength(STRLEN);
	char* strPosTrack = m_strPosTrack.GetBufferSetLength(STRLEN);
	char* strVelTrack = m_strVelTrack.GetBufferSetLength(STRLEN);
	char* strPosError = m_strPosError.GetBufferSetLength(STRLEN);
	char* strVelError = m_strVelError.GetBufferSetLength(STRLEN);
	char* strPosInit = strPos;
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		strPos += sprintf(strPos, "%10.3f", ms.GetPosServo(ax));
		strVel += sprintf(strVel, "%10.3f", ms.GetVelServo(ax));
		strPosTrack += sprintf(strPosTrack, "%10.3f", ms.GetPosTrack(ax));
		strVelTrack += sprintf(strVelTrack, "%10.3f", ms.GetVelTrack(ax));
		strPosError += sprintf(strPosError, "%10.3f", ms.GetPosError(ax));
		strVelError += sprintf(strVelError, "%10.3f", ms.GetVelError(ax));
	}
	ASSERT(strPos - strPosInit < STRLEN);
	m_PathTime = ms.iTime * ms.dTime;
	m_iPathStep = ms.iTime;

	m_Machine_TI   = ms.PathBuff.Init.Time;
	m_Machine_TF   = ms.PathBuff.Final.Time;
	m_iMachine_ASI = ms.PathBuff.Init.AccSeg;
	m_iMachine_ASF = ms.PathBuff.Final.AccSeg;
	
	UpdateData(false);
}

/*
void UpdateMachineBufferStatus(SContBufferSpan& span)
{
	m_pMachBufferSpan = &span;
	if (IsStatusVisible())
		m_ControlStatusDlg.UpdateMachineBufferStatus(span);
}
void UpdateMachineStateStatus(SMachineState& state)
{
	m_pMachState = &state;
	if (IsStatusVisible())
		m_ControlStatusDlg.UpdateMachineState(state);
}
*/
