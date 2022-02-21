// ManualControlView.cpp : implementation file
//

#include "stdafx.h"
#include <float.h>

#include "cnccontrol.h"
#include "CNCComms.h"
#include "ParamDoc.h"
#include "ControllerPath.h"

#include "PathDoc.h"
#include "ManualControlView.h"

#include "MachineCodes.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int POSSTRLEN = 15;			// -10000.000

/////////////////////////////////////////////////////////////////////////////
// CManualControlView

IMPLEMENT_DYNCREATE(CManualControlView, CFormView)

CManualControlView::CManualControlView()
	: CFormView(CManualControlView::IDD)
{
	m_pCNCComms = NULL;
	m_pControllerPath = NULL;
	m_bRegularUpdate = false;

	//{{AFX_DATA_INIT(CManualControlView)
	m_bEnableMachine = FALSE;
	m_iRampHoldTime = 0;
	m_iRampRate = 2000;
	m_iRampTime = 100;
	m_iRampVelChange = 0;
	m_iStepHoldTime = 100;
	m_iStepAcc = 200000;
	m_iStepVelChange = 0;
	m_iAccType = 0;		// 0=ramp, 1=step
	m_nFeedBack = 0;
	m_nFeedFoward = 0;
	m_strErrorLast = _T("");
	//}}AFX_DATA_INIT
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		m_arbEnable[ax] = FALSE;
		m_arPosNew[ax] = 0.0;
		m_arbUseLoc[ax] = false;
		m_arbUseEntered[ax] = false;
		m_arnPosNewSource[ax] = USE_CURRENT;
		m_arbAxisPosLocated[ax] = false;

		m_strPosServo[ax] = _T("");
		m_strPosCurr[ax] = _T("");
		m_strPosError[ax] = _T("");
		m_strVelCurr[ax] = _T("");
		m_strPosNew[ax] = _T("");
	}

}

CManualControlView::~CManualControlView()
{
}

void CManualControlView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	// controls modified by user only
	if (!m_bRegularUpdate)
	{
		//{{AFX_DATA_MAP(CManualControlView)
		DDX_Control(pDX, IDC_COMBO_POSNEW, m_PosNewCombo);
//		DDX_Text(pDX, IDC_EDIT_ACCRAMPHOLDTIME, m_iRampHoldTime);
//		DDX_Text(pDX, IDC_EDIT_ACCRAMPRATE, m_iRampRate);
//		DDX_Text(pDX, IDC_EDIT_ACCRAMPTIME, m_iRampTime);
//		DDX_Text(pDX, IDC_EDIT_ACCRAMPVELCHANGE, m_iRampVelChange);
//		DDX_Text(pDX, IDC_EDIT_ACCSTEPHOLDTIME, m_iStepHoldTime);
//		DDX_Text(pDX, IDC_EDIT_ACCSTEP, m_iStepAcc);
//		DDX_Text(pDX, IDC_EDIT_ACCSTEPVELCHANGE, m_iStepVelChange);
//		DDX_Radio(pDX, IDC_RADIO_ACCTYPE1, m_iAccType);
	//}}AFX_DATA_MAP

		for (int ax = 0; ax < NUM_AXIS; ax++)
		{
			DDX_Text(pDX, IDC_EDIT_POSNEWX+ax, m_strPosNew[ax]);		// don't want to set edit if currently typing
			DDX_Check(pDX, IDC_BUTTON_USEX+ax, m_arbUseLoc[ax]);
		}
	}

	// controls modified by user or automaticaly
	DDX_Check(pDX, IDC_BUTTON_ENABLE, m_bEnableMachine);
	DDX_CBIndex(pDX, IDC_COMBO_FEEDBACK, m_nFeedBack);
	DDX_CBIndex(pDX, IDC_COMBO_FEEDFOWARD, m_nFeedFoward);
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		DDX_Check(pDX, IDC_BUTTON_ENABLEX+ax, m_arbEnable[ax]);
	}

	// readonly controls modified automaticaly only - set only when not saving
	if (!pDX->m_bSaveAndValidate)
	{
		DDX_Text(pDX, IDC_EDIT_ERRORLAST, m_strErrorLast);
		for (int ax = 0; ax < NUM_AXIS; ax++)
		{
			DDX_Text(pDX, IDC_EDIT_POSSERVOX+ax, m_strPosServo[ax]);
			DDX_Text(pDX, IDC_EDIT_POSCURRX+ax, m_strPosCurr[ax]);
			DDX_Text(pDX, IDC_EDIT_POSERRORX+ax, m_strPosError[ax]);
			DDX_Text(pDX, IDC_EDIT_VELCURRX+ax, m_strVelCurr[ax]);
		}
	}

	m_bRegularUpdate = false;
}


BEGIN_MESSAGE_MAP(CManualControlView, CFormView)
	//{{AFX_MSG_MAP(CManualControlView)
	ON_BN_CLICKED(IDC_BUTTON_STOPXY, OnButtonstopxy)
	ON_BN_CLICKED(IDC_BUTTON_STOPZ, OnButtonstopz)
	ON_BN_CLICKED(IDC_BUTTON_SETPARAM, OnButtonSetParam)
	ON_BN_CLICKED(IDC_BUTTON_ENABLE, OnButtonEnable)
	ON_BN_CLICKED(IDC_BUTTON_ENABLEX, OnButtonEnableX)
	ON_BN_CLICKED(IDC_BUTTON_ENABLEY, OnButtonEnableY)
	ON_BN_CLICKED(IDC_BUTTON_ENABLEZ, OnButtonEnableZ)
	ON_EN_KILLFOCUS(IDC_EDIT_ACCRAMPRATE, OnKillfocusEditAcc)
	ON_BN_CLICKED(IDC_BUTTON_MATCHTRACKER, OnButtonMatchTracker)
	ON_BN_CLICKED(IDC_BUTTON_MOVETOPOS, OnButtonMoveToPos)
	ON_WM_CTLCOLOR()
	ON_CBN_SELENDOK(IDC_COMBO_POSNEW, OnSelendokComboPosNew)
	ON_CBN_SELENDOK(IDC_COMBO_FEEDBACK, OnSelendokComboControlType)
	ON_WM_DESTROY()
	ON_EN_KILLFOCUS(IDC_EDIT_ACCRAMPTIME, OnKillfocusEditAcc)
	ON_EN_KILLFOCUS(IDC_EDIT_ACCRAMPHOLDTIME, OnKillfocusEditAcc)
	ON_EN_KILLFOCUS(IDC_EDIT_ACCSTEP, OnKillfocusEditAcc)
	ON_EN_KILLFOCUS(IDC_EDIT_ACCSTEPHOLDTIME, OnKillfocusEditAcc)
	ON_CBN_SELENDOK(IDC_COMBO_FEEDFOWARD, OnSelendokComboControlType)
	ON_BN_CLICKED(IDC_BUTTON_PROBESAMPLER, OnButtonProbeSampler)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_INCZ, IDC_BUTTON_DXIY, OnButtonChangeVel)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_USEX, IDC_BUTTON_USEZ, OnButtonUseLoc)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_EDIT_POSNEWX, IDC_EDIT_POSNEWZ, OnChangeEditPosNew)
	ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_POSNEWX, IDC_EDIT_POSNEWZ, OnKillfocusEditPosNew)

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CManualControlView diagnostics

#ifdef _DEBUG
void CManualControlView::AssertValid() const
{
	CFormView::AssertValid();
}

void CManualControlView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CParamDoc* CManualControlView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CParamDoc)));
	return (CParamDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CManualControlView message handlers


void CManualControlView::OnInitialUpdate() 
{
	ResizeParentToFit();
	CFormView::OnInitialUpdate();

	GetParentFrame()->SetWindowText("Manual Machine Controller");

	// load bitmaps to buttons
	CBitmap bm;

	VERIFY(bm.LoadBitmap(IDB_BUTTONLEFT));
	((CButton*)GetDlgItem(IDC_BUTTON_DECX))->SetBitmap(bm);
	bm.Detach();			// don't delete GDI object

	VERIFY(bm.LoadBitmap(IDB_BUTTONRIGHT));
	((CButton*)GetDlgItem(IDC_BUTTON_INCX))->SetBitmap(bm);
	bm.Detach();			// don't delete GDI object

	VERIFY(bm.LoadBitmap(IDB_BUTTONUP));
	((CButton*)GetDlgItem(IDC_BUTTON_INCY))->SetBitmap(bm);
	((CButton*)GetDlgItem(IDC_BUTTON_INCZ))->SetBitmap(bm);
	bm.Detach();			// don't delete GDI object

	VERIFY(bm.LoadBitmap(IDB_BUTTONDOWN));
	((CButton*)GetDlgItem(IDC_BUTTON_DECY))->SetBitmap(bm);
	((CButton*)GetDlgItem(IDC_BUTTON_DECZ))->SetBitmap(bm);
	bm.Detach();			// don't delete GDI object

	VERIFY(bm.LoadBitmap(IDB_BUTTONSTOP));
	((CButton*)GetDlgItem(IDC_BUTTON_STOPXY))->SetBitmap(bm);
	((CButton*)GetDlgItem(IDC_BUTTON_STOPZ))->SetBitmap(bm);
	bm.Detach();			// don't delete GDI object

	VERIFY(bm.LoadBitmap(IDB_BUTTONUPLEFT));
	((CButton*)GetDlgItem(IDC_BUTTON_DXIY))->SetBitmap(bm);
	bm.Detach();			// don't delete GDI object

	VERIFY(bm.LoadBitmap(IDB_BUTTONUPRIGHT));
	((CButton*)GetDlgItem(IDC_BUTTON_IXIY))->SetBitmap(bm);
	bm.Detach();			// don't delete GDI object

	VERIFY(bm.LoadBitmap(IDB_BUTTONDOWNLEFT));
	((CButton*)GetDlgItem(IDC_BUTTON_DXDY))->SetBitmap(bm);
	bm.Detach();			// don't delete GDI object

	VERIFY(bm.LoadBitmap(IDB_BUTTONDOWNRIGHT));
	((CButton*)GetDlgItem(IDC_BUTTON_IXDY))->SetBitmap(bm);
	bm.Detach();			// don't delete GDI object




	OnKillfocusEditAcc();	// sets relevant variables


	double nan = 0;
	nan = 0/nan;		// _finite(nan) == false;
	m_arLocations[0][0] = nan;
	m_arLocations[0][1] = nan;
	m_arLocations[0][2] = 65.5 + 50;
	m_arLocations[1][0] = 1;
	m_arLocations[1][1] = 2;
	m_arLocations[1][2] = 3;


	int idxItem;
	idxItem = m_PosNewCombo.AddString("Cut Depth +50mm");
	m_PosNewCombo.SetItemData(idxItem, 0);
	idxItem = m_PosNewCombo.AddString("Previous Start");
	m_PosNewCombo.SetItemData(idxItem, 1);


	VERIFY(m_FontPos.CreatePointFont(100, "courier"));
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		GetDlgItem(IDC_EDIT_POSSERVOX+ax)->SetFont(&m_FontPos);
		GetDlgItem(IDC_EDIT_POSCURRX+ax)->SetFont(&m_FontPos);
		GetDlgItem(IDC_EDIT_POSERRORX+ax)->SetFont(&m_FontPos);
		GetDlgItem(IDC_EDIT_VELCURRX+ax)->SetFont(&m_FontPos);
		GetDlgItem(IDC_EDIT_POSNEWX+ax)->SetFont(&m_FontPos);
		m_arPosLoc[ax] = nan;
	}

	m_clrPosNotLocated = RGB(255,130,0);
	m_brPosNotLocated.CreateSolidBrush(m_clrPosNotLocated);


}

bool CManualControlView::SendMonitorMessage(int iMsg, int wParam, long lParam)
{
	return m_pCNCComms->SendMonitorMessage(iMsg, wParam, lParam);
}


void CManualControlView::OnButtonstopxy() 
{
	SendMonitorMessage(WMU_STOPMOVEMENT);
}

void CManualControlView::OnButtonstopz() 
{
	SendMonitorMessage(WMU_STOPMOVEMENT);
}

void CManualControlView::OnButtonChangeVel(UINT nID) 
{
	UpdateData(true);				// get accelerations
	if (m_ADXcng.iState == 0)		// locked - last setting hasn't been used yet
		return;

	m_ADXcng.ZeroData();
	int iAxisValue = 0;

	if (m_iAccType == 0)		// ramp accel
	{
		m_ADXcng.iRampTime = m_iRampTime;
		m_ADXcng.iHoldTime = m_iRampHoldTime;
		m_ADXcng.iMethod = PATHSEG_dACCEL;
		iAxisValue = m_iRampRate;
	}
	else if (m_iAccType == 1)		// step accel
	{
		m_ADXcng.iRampTime = 0;
		m_ADXcng.iHoldTime = m_iStepHoldTime;
		m_ADXcng.iMethod = PATHSEG_ACCEL;
		iAxisValue = m_iStepAcc;
	}

	switch(nID)
	{
	case IDC_BUTTON_INCX:
		m_ADXcng.iAxisData[0] = iAxisValue;
		break;
	case IDC_BUTTON_DECX:
		m_ADXcng.iAxisData[0] = -iAxisValue;
		break;
	case IDC_BUTTON_INCY:
		m_ADXcng.iAxisData[1] = iAxisValue;
		break;
	case IDC_BUTTON_DECY:
		m_ADXcng.iAxisData[1] = -iAxisValue;
		break;
	case IDC_BUTTON_INCZ:
		m_ADXcng.iAxisData[2] = iAxisValue;
		break;
	case IDC_BUTTON_DECZ:
		m_ADXcng.iAxisData[2] = -iAxisValue;
		break;

	case IDC_BUTTON_IXIY:
		m_ADXcng.iAxisData[0] = iAxisValue;
		m_ADXcng.iAxisData[1] = iAxisValue;
		break;
	case IDC_BUTTON_IXDY:
		m_ADXcng.iAxisData[0] = iAxisValue;
		m_ADXcng.iAxisData[1] = -iAxisValue;
		break;
	case IDC_BUTTON_DXIY:
		m_ADXcng.iAxisData[0] = -iAxisValue;
		m_ADXcng.iAxisData[1] = iAxisValue;
		break;
	case IDC_BUTTON_DXDY:
		m_ADXcng.iAxisData[0] = -iAxisValue;
		m_ADXcng.iAxisData[1] = -iAxisValue;
		break;
	default:
		return;
	}

	m_ADXcng.iState = 0;				// lock
	if (!SendMonitorMessage(WMU_CHANGEVELOCITY, (int)&m_ADXcng))
		m_ADXcng.iState = 1;				// unlock, not sent

}


void CManualControlView::OnButtonSetParam() 
{
	SendMonitorMessage(WMU_SETPARAMETERS);
}

void CManualControlView::OnButtonEnable()
{
	UpdateData();
	SendMonitorMessage(WMU_SENDPARAM, GP_MachineDisable, !m_bEnableMachine);
}

void CManualControlView::OnButtonEnableX() 
{
	UpdateData();
	SendMonitorMessage(WMU_SENDPARAM, CAxPar(1, AP_AxisDisable), !m_arbEnable[0]);
}
void CManualControlView::OnButtonEnableY() 
{
	UpdateData();
	SendMonitorMessage(WMU_SENDPARAM, CAxPar(2, AP_AxisDisable), !m_arbEnable[1]);
}
void CManualControlView::OnButtonEnableZ() 
{
	UpdateData();
	SendMonitorMessage(WMU_SENDPARAM, CAxPar(3, AP_AxisDisable), !m_arbEnable[2]);
}

void CManualControlView::OnKillfocusEditAcc() 
{
	UpdateData(true);
	m_iRampVelChange = m_iRampRate * m_iRampTime * (m_iRampTime + m_iRampHoldTime);
	m_iStepVelChange = m_iStepAcc * m_iStepHoldTime;
	UpdateData(false);
}


void CManualControlView::OnButtonMatchTracker() 
{
	SendMonitorMessage(WMU_SENDPARAM, GP_SetPosTrackToPos, 1);
}






void CManualControlView::OnUpdate(CView*, LPARAM lHint, CObject*) 
{
//	TRACE("Got CManualControlView::OnUpdate()\n");
	CAxPar axPar = lHint;
	switch (lHint & HINT_MASK)
	{
	case 0:
		OnUpdateParam(lHint);
		OnUpdateState();
		break;
	case HINT_AXPAR:
		OnUpdateParam(lHint);
		m_bRegularUpdate = true;
		break;
	case HINT_MACHSTATE:
		OnUpdateState();
		m_bRegularUpdate = true;
		break;
	}
	UpdateData(false);
}



void CManualControlView::OnUpdateParam(CAxPar axPar)
{
	// do update if axPar is a used one
	CParamDoc* pDoc = GetDocument();
	int iVal;
	pDoc->GetParamValue(axPar, &iVal);
	if (axPar.IsGeneralParam())
		switch (axPar.param)
		{
		case GP_MachineDisable:
			break;
		case GP_ControlCode:
			m_nFeedFoward = iVal & 0x0f;
			m_nFeedBack = (iVal >> 4) & 0x0f;
			if (m_nFeedFoward > 2) m_nFeedFoward = 2;		// interperate as machine does
			if (m_nFeedBack > 2) m_nFeedBack = 0;
			break;
		case GP_ErrorLast:
			m_strErrorLast = pDoc->GetErrorDescription(iVal);
			break;
		default:
			return;
		}
	else
		switch (axPar.param)
		{
		case AP_AxisLocated:
			GetDlgItem(IDC_EDIT_POSCURRX)->Invalidate();
			GetDlgItem(IDC_EDIT_POSCURRY)->Invalidate();
			GetDlgItem(IDC_EDIT_POSCURRZ)->Invalidate();
			//Invalidate();
		case AP_AxisDisable:
			break;
		default:
			return;
		}

	// get enable states from ParamDoc
	pDoc->GetParamValue(GP_MachineDisable, &iVal);
	m_bEnableMachine = (iVal == 0);

	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		pDoc->GetParamValue(CAxPar(ax+1, AP_AxisDisable), &iVal);
		m_arbEnable[ax] = (iVal == 0);

		pDoc->GetParamValue(CAxPar(ax+1, AP_AxisLocated), &iVal);
		m_arbAxisPosLocated[ax] = (iVal != 0);
	}


}

void CManualControlView::OnUpdateState()
{
	CParamDoc* pDoc = GetDocument();
	// get states from 'MachineState' in ParamDoc
	if (pDoc->m_pMachineState == NULL)
		return;
	CVector vtPos, vtPosError, vtVel, vtPosServo;
	CMachineState& machState = *pDoc->m_pMachineState;
	machState.GetPosServo(vtPosServo);
	machState.GetPosTipRelBase(vtPos);
	machState.GetPosError(vtPosError);
	machState.GetVel(vtVel);
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		sprintf(m_strPosServo[ax].GetBufferSetLength(POSSTRLEN), "%.3f", vtPosServo[ax]);
		sprintf(m_strPosCurr[ax].GetBufferSetLength(POSSTRLEN), "%.3f", vtPos[ax]);
		sprintf(m_strPosError[ax].GetBufferSetLength(POSSTRLEN), "%.3f", vtPosError[ax]);
		sprintf(m_strVelCurr[ax].GetBufferSetLength(POSSTRLEN), "%.3f", vtVel[ax]);
	}
}


HBRUSH CManualControlView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
/*	if (nCtlColor == CTLCOLOR_EDIT)	// check if a IDC_EDIT_POSNEW or IDC_EDIT_POSCURR
	{
		int ax = pWnd->GetDlgCtrlID() - IDC_EDIT_POSNEWX;
		if (ax >= 0 && ax < NUM_AXIS && !m_arbAxisPosLocated[ax])
		{
			pDC->SetBkColor(m_clrPosNotLocated);
			hbr = m_brPosNotLocated;
		}
	}
*/	
	if (nCtlColor == CTLCOLOR_STATIC)	// check if a IDC_EDIT_POSNEW or IDC_EDIT_POSCURR
	{
		int ax = pWnd->GetDlgCtrlID() - IDC_EDIT_POSCURRX;		// readonly's are statics
		if (ax >= 0 && ax < NUM_AXIS && !m_arbAxisPosLocated[ax])
		{
			pDC->SetBkColor(m_clrPosNotLocated);
			hbr = m_brPosNotLocated;
		}
	}

	// TODO: Change any attributes of the DC here
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CManualControlView::OnSelendokComboPosNew() 
{
	int sel = m_PosNewCombo.GetCurSel();
	if (sel == CB_ERR)
		return;
	int idxItem = m_PosNewCombo.GetItemData(sel);
	double* arLoc = m_arLocations[idxItem];
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		m_arPosLoc[ax] = arLoc[ax];
		m_arbUseLoc[ax] = _finite(arLoc[ax]);
	}
	OnUpdatePosNew();
	UpdateData(false);
}

void CManualControlView::OnButtonUseLoc(UINT) 
{
	UpdateData(true);
	for (int ax = 0; ax < NUM_AXIS; ax++)
		if (m_arbUseLoc[ax] && !_finite(m_arPosLoc[ax]))
			m_arbUseLoc[ax] = false;
	OnUpdatePosNew();
	UpdateData(false);
}

void CManualControlView::OnChangeEditPosNew(UINT nID) 
{
	int ax = nID - IDC_EDIT_POSNEWX;
	if (m_arbUseLoc[ax])
	{
		m_arbUseLoc[ax] = false;
	}
	m_arbUseEntered[ax] = true;
}

void CManualControlView::OnKillfocusEditPosNew(UINT nID)
{
	int ax = nID - IDC_EDIT_POSNEWX;
	if (!m_arbUseLoc[ax] && m_arbUseEntered[ax])
	{
		UpdateData(true);

		if (sscanf(m_strPosNew[ax], "%lf", &m_arPosEntered[ax]) != 1)	// check if valid
			m_arbUseEntered[ax] = false;
		//	m_arPosEntered[ax] = atof(m_strPosNew[ax]);
		OnUpdatePosNew();
		UpdateData(false);
	}
}

void CManualControlView::OnUpdatePosNew()
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		if (m_arbUseLoc[ax])
		{
			m_arPosNew[ax] = m_arPosLoc[ax];
			m_arnPosNewSource[ax] = USE_LOCATION;
		}
		else if (m_arbUseEntered[ax])
		{
			m_arPosNew[ax] = m_arPosEntered[ax];
			m_arnPosNewSource[ax] = USE_ENTERED;
		}
		else
			m_arnPosNewSource[ax] = USE_CURRENT;

		if (m_arnPosNewSource[ax] == USE_CURRENT)
			m_strPosNew[ax].GetBufferSetLength(POSSTRLEN)[0] = 0;		// blank it
		else
			sprintf(m_strPosNew[ax].GetBufferSetLength(POSSTRLEN), "%.3f", m_arPosNew[ax]);
	}
}

void CManualControlView::OnButtonMoveToPos() 
{
	UpdateData();
	CVector vtPosCurr;
	GetDocument()->m_pMachineState->GetPosTipRelBase(vtPosCurr);
	CVector vtPosNew;
	for (int ax = 0; ax < NUM_AXIS; ax++)
		if (m_arnPosNewSource[ax] == USE_CURRENT)
			vtPosNew[ax] = vtPosCurr[ax];
		else
			vtPosNew[ax] = m_arPosNew[ax];
	m_pControllerPath->MoveToPositionRelBase(vtPosNew);
}

void CManualControlView::OnSelendokComboControlType() 
{
	UpdateData();
	int nCode = 0;
	switch (m_nFeedFoward)
	{
	case 0:		// velocity
		nCode |= 0x00; break;
	case 1:		// full
		nCode |= 0x01; break;
	case 2:		// none
		nCode |= 0x02; break;
	default:
		ASSERT(0);
	}
	switch (m_nFeedBack)
	{
	case 0:		// none
		nCode |= 0x00; break;
	case 1:		// PID-pos error
		nCode |= 0x10; break;
	case 2:		// PID-vel error
		nCode |= 0x20; break;
	default:
		ASSERT(0);
	}
//	ASSERT(m_nFeedFoward >= 0 && m_nFeedFoward <= 2);
//	ASSERT(m_nFeedBack >= 0 && m_nFeedBack <= 2);
//	nCode = m_nFeedFoward + (0x10 * m_nFeedBack);
	SendMonitorMessage(WMU_SENDPARAM, GP_ControlCode, nCode);
}

void CManualControlView::OnDestroy() 
{
	CFormView::OnDestroy();
	AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_MANUALCONTROL_CLOSED);
}


void CManualControlView::OnButtonProbeSampler() 
{
	AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_MACHINE_PROBESAMPLE);
}
