// ControllerPath.cpp: implementation of the CControllerPath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"

#include "ControllerPath.h"

#include "ParamDoc.h"
//#include "PosConverter.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControllerPath::CControllerPath()
{
	m_pComms = NULL;
	m_pParamDoc = NULL;
	m_bTestRun = false;
}

CControllerPath::~CControllerPath()
{
	m_ManualPath.SetModifiedFlag(false);	// so don't get 'destroying unsaved doc'
}

void CControllerPath::Init()
{
	ASSERT(m_pComms != NULL);

	m_bSendingPath = false;

	m_SegAccelFIFO.SetBufferSize(200);
	m_ContTracker.SetSegmentAccelBuffer(&m_SegAccelFIFO);

	m_LimitList.SetSize(0, 1024);
	m_PathSpeed.SetLimitList(&m_LimitList);
	m_PathStepper.SetLimitList(&m_LimitList);
	m_PathStepper.SetControllerTracker(&m_ContTracker);

	m_PathSpeed.Init();
	m_PathSpeed.SetScanMinTime(100);	// scan foward at least 100sec worth
	m_PathSpeed.m_bSaveLimits = true;

	m_PathStepper.Init();

	m_ContTracker.m_bLogMotion = false;
	m_ContTracker.Init();

}

bool CControllerPath::SetPathDoc(CPathDoc* pPathDoc)
{
	// set path if not currently sending
	if (m_bSendingPath)
		return false;
	m_pPathDoc = pPathDoc;
	m_PathSpeed.SetPathDoc(pPathDoc);
	m_PathStepper.SetPathDoc(pPathDoc);
	return true;
}

void CControllerPath::ResetPath()
{
	if (!GetPathDoc())
		return;
	ASSERT(m_PathSpeed.GotPath());
	ASSERT_VALID(m_PathSpeed.GetPathDoc());
	ASSERT(m_PathStepper.GotPath());
	ASSERT_VALID(m_PathStepper.GetPathDoc());

	m_PathSpeed.SetToStart();
	m_PathStepper.SetToStart();

	return;
}

void CControllerPath::StartSendingPath()
{
	// used only when calculating path to send to machine
	// CPathSpeedDoc::FindSpeedsStart() used for test run

	m_bTestRun = !m_pComms->IsCommOpen();


	if (m_bSendingPath)
	{
		LOGERROR("Already sending path");
		return;
	}
	ASSERT(GotPath());
	if (m_pPathDoc->GetNumSegments() == 0)
		return;


	ResetPath();

	CVector vtPosTipOfPathStart;
	if (m_bTestRun)
		vtPosTipOfPathStart = 0;
	else
		m_pParamDoc->m_pMachineState->GetPosTip(vtPosTipOfPathStart);	// get current machine pos


	CVector vtPosPathStart;
	m_pPathDoc->GetStartPos(vtPosPathStart);
	CVector vtPosTipOfPathOrigin;
	vtPosTipOfPathOrigin = vtPosTipOfPathStart - vtPosPathStart;

/*	m_CurrPath.m_vtPosOfStart = vtPosTipOfPathStart;
	m_CurrPath.m_TimeStart = CTime::GetCurrentTime();
	m_CurrPath.m_Duration = 30*60;		// seconds
	CString strT = m_CurrPath.m_TimeStart.Format("%#I:%M%p");
*/

//	m_PathSpeed.SetPosMachineOfPathStart(vtPosTipOfPathStart);
//	m_PathStepper.SetPosMachineOfPathStart(vtPosTipOfPathStart);

//	m_PathSpeed.GetPosMachineOfPathOrigin(vtPosTipOfPathOrigin);

	// match start path to current controller motion
	SMotionStateBasic ms;
	// set controller pos to start of path
	m_PathSpeed.GetPathStartPos(ms.vtPos);
	m_ContTracker.Init();
	m_ContTracker.SetPosTipOfPathOrigin(vtPosTipOfPathOrigin);
	m_ContTracker.SetCurrentMotion(ms);
	m_ContTracker.SetCurrentMotionStopped();
	m_ContTracker.GetCurrentMotion(ms);		// pos will be machine pos
	m_PathSpeed.SetInitialMotion(ms);		// pos should match, set vel and acc



	// set machine axis limits to path bounds + clearance
	CVector vtClearance(4, 4, 4);			// allow 4mm each way
	CVector vtPosMin = m_pPathDoc->GetMinNodeValues() - vtClearance + vtPosTipOfPathOrigin;
	CVector vtPosMax = m_pPathDoc->GetMaxNodeValues() + vtClearance + vtPosTipOfPathOrigin;
	// adjust vtPosMin/Max to servo pos
//	g_PosConvert.GetPosServoFromPosTip(vtPosMin);	// don't run in user thread!!
//	g_PosConvert.GetPosServoFromPosTip(vtPosMax);

//	SendMonitorMessage(WMU_SETPATHAXISLIMITS);
//	LOGERROR("Not finished setting machine pos limits to path!");


	m_bSendingPath = true;
	m_PathSpeed.FindSpeeds();		// fill limits first
//	m_MachineStatusView.UpdateLimitBufferStatus(m_PathSpeed.GetBufferStatus());
	m_PathStepper.FindInitialTimeStep();	// notify stepper of list change

	FillSegAccelBuffer();
	if (!m_bTestRun)
	{
		SendMonitorMessage(WMU_SETPATHBUFFER, 0, (long)&m_SegAccelFIFO);
		SendMonitorMessage(WMU_RESETPATH);
		SendMonitorMessage(WMU_SENDPATH);
	}
}

void CControllerPath::SendPath()		// continuly sends path
{
	CWaitCursor wait;

	if (!m_bSendingPath)
		StartSendingPath();
	else
		SendMonitorMessage(WMU_SENDPATH);

	if (!m_bTestRun)
		return;

	// do test run through path without sending to machine!
	while (m_bSendingPath)
	{
		char strMsg[128];
		sprintf(strMsg, "Got to segment %i of %i\n", m_PathStepper.GetCurrentSegment(), m_pPathDoc->GetNumSegments());
		TRACE(strMsg);
//		sprintf(strMsg, "Got to segment %i of %i.  Calc more limits (in CControllerPath)?\n", m_PathStepper.GetCurrentSegment(), m_pPathDoc->GetNumSegments());
//		if (AfxMessageBox(strMsg, MB_YESNO) != IDYES)
//			break;

		EmptySegAccelBufferTo(4);
		FillSegAccelBuffer();
	}
}

void CControllerPath::EmptySegAccelBufferTo(int numRemain)
{
	int cnt = m_SegAccelFIFO.GetCount();
	int numDiscard = cnt - numRemain;
	if (numDiscard > 0)
		m_SegAccelFIFO.Discard(numDiscard);
}

void CControllerPath::FillSegAccelBuffer()	// calc next section time worth
{
	// send path speed segments to controller in stages
	for (;;)
	{
		m_PathStepper.FindTimeSteps();	// will have added accel segments to buffer
//		m_MachineStatusView.UpdateAccSegBufferStatus(m_PathStepper.GetBufferStatus());

		if (!m_PathStepper.AtEndOfCalcLimits())
			break;
		if (m_PathStepper.FinishedPathSteps())
		{
			m_bSendingPath = false;
			break;
		}
		if (!m_PathSpeed.AtEndOfPath())		// may be at end of path if m_PathStepper still adding final segments
		{
			m_PathSpeed.FindSpeeds();		// find next section limits
			m_PathStepper.LimitListChanged();
		}
//		m_MachineStatusView.UpdateLimitBufferStatus(m_PathSpeed.GetBufferStatus());
	}
}

void CControllerPath::FindAllLimits()	// runs through all limit calc's for to check for problems
{
	// send path speed segments to controller in stages
	for (;;)
	{
		if (m_PathSpeed.AtEndOfPath())
			break;
		m_PathSpeed.FindSpeeds();		// find next section limits
//		m_MachineStatusView.UpdateLimitBufferStatus(m_PathSpeed.GetBufferStatus());
	}
}

void CControllerPath::SendMorePath()		// called to request more path data
{
	if (!GetPathDoc())
		return;
	if (!m_bSendingPath)
		return;

	FillSegAccelBuffer();
	SendMonitorMessage(WMU_SENDMOREPATH);	// will not normally need notification unless tx/rx has finished
}

void CControllerPath::StopSendingPath()
{
	m_bSendingPath = false;
	SendMonitorMessage(WMU_STOPPATH);
}

void CControllerPath::OnPathFinished()
{
//	LOGEVENT("Got Path Finished notification");
	m_bSendingPath = false;
}

/*
void CControllerPath::ShowStatus(bool bShow)	
{
	if (m_MachineStatusView.GetSafeHwnd())
		m_MachineStatusView.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	else if (bShow)
	{
		m_MachineStatusView.Create();
		m_MachineStatusView.ShowWindow(SW_SHOW);
	}
	if (bShow)
		AfxGetMainWnd()->SetFocus();
}

bool CControllerPath::IsStatusVisible()
{
	if (!m_MachineStatusView.GetSafeHwnd())
		return false;
	return m_MachineStatusView.IsWindowVisible() != 0;
}
*/

bool CControllerPath::MoveToPositionRelBase(CVector& vtPosNew)
{
	CVector vtPosCurr;
	if (!m_pParamDoc)
	{
		LOGERROR("m_pParamDoc not set to get current position");
		return false;
	}
	else
		if (m_pParamDoc->m_pMachineState->UsingPosFeedback())
			m_pParamDoc->m_pMachineState->GetPosTipTrackRelBase(vtPosCurr);	// error should be ~0
		else
			m_pParamDoc->m_pMachineState->GetPosTipRelBase(vtPosCurr);			// postrack not valid if no pos feedback

	return MoveLine(vtPosCurr, vtPosNew);
}

bool CControllerPath::MoveRelative(CVector& vtPosRel)
{
	CVector vtPosCurr = 0;
	return MoveLine(vtPosCurr, vtPosRel);
}

bool CControllerPath::MoveLine(CVector& vtStart, CVector& vtEnd)
{
	if (m_bSendingPath)
	{
		LOGERROR("Can't Move - Already sending a path");
		return false;
	}
	if ((vtEnd - vtStart).MagSq() == 0)		// check for null path!!
	{
		LOGMESSAGE("Move To - no change");
		return false;
	}

	m_ManualPath.ErasePath();
	CPathNode nd;
	nd.type = PNT_ENDNODE;
	nd = vtStart;
	m_ManualPath.AddPathNode(nd);
	nd = vtEnd;
	m_ManualPath.AddPathNode(nd);
	m_ManualPath.FinaliseNewPath();

	SetPathDoc(&m_ManualPath);
	StartSendingPath();
	return true;
}


