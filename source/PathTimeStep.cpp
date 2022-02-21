// PathTimeStep.cpp: implementation of the CPathTimeStep class.
//
// functions for calculating path time steps used by CNC controller
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNCControl.h"

#include "PathTimeStep.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPathTimeStep::CPathTimeStep()
{
	m_pCtrlTrack = NULL;
}

CPathTimeStep::~CPathTimeStep()
{
}

void CPathTimeStep::Init()
{
	CPathMove::Init();		// call base, only tolerances needed!

	SetDefaults();
}

void CPathTimeStep::SetDefaults()
{
	m_iStepMethod = g_Settings.MachPathSeg.iMethod;

	// SM_CONSTSTEP parameters
	m_iStepAdv = NEARINT(m_dTSegment / m_dT);

	// SM_FITPOLY parameters
	m_posTol = g_Settings.MachPathSeg.PosTol;

}

void CPathTimeStep::FindTimeSteps()
{
	switch (m_iStepMethod)
	{
	case SM_CONSTSTEP:
		FindTimeStepsConstStep();		// uses constant poly step (apart from at a poly or limit change)
		break;
	case SM_FITPOLY:
		FindTimeStepsFitPoly();			// determines poly step by the fit of a poly to points;
		break;
	default:
		ASSERT(0);		// invalid m_iStepMethod
	}
}


/*
constant step size algorithm for set step length of 100ms
	normal loop:
L1		inc limit time by 100ms
		check limit time is not past a change, if it is goto J1
			?do change if STEP_AFTER
		find pos then s of limit axis from limit time
START	get pos, vel, acc at s step
			?do change if STEP_BEFORE
			?check and set accel jump if changed
		move controller to step state (no accel jump for next step)
		goto L1
	
J1		set adjusted time of step
	if closest step is just before change (bStepIsBeforeChange == true)
		find pos then s of limit axis from limit time
		get pos, vel, acc at s step
		do change - set accelBefore and accelAfter change
		set previous end accel from step
		set next start accel from post change (accelAfter)
		move controller to step state (possible accel jump for next step)
		goto L1

	else if closest step is just after change (bStepIsBeforeChange == false)
		do change - set accelBefore and accelAfter change
		find pos then s of limit axis from limit time
		get pos, vel, acc at s step
		set previous end accel from pre change (accelBefore)
		set next start accel from step
		move controller to step state (possible accel jump for next step)
		goto L1

*/

/*
	FindTimeSteps() flowchart
	--------------------------
	Initialise:
		t = 0
	let PolySegFit know discrete time step size to align poly changes
		depends on controller poly time resolution (~1ms)

	find next change of poly
	find next change of limit
	find closest of these

loop:
	get pos at time t

	give next poly or limit change to PolySegFit (PolySegFit will break poly here
	give point x, y & z to PolySegFit.NextPoint() for each axis
		adjust current time step size if required by PolySegFit


	if (got a new poly segment?)
		add axis polys to controller buffer for sending
		maybe only need the time span of poly, poly shape determined by final pos, vel


	
	advance by current time step
	if (passed or close to next poly or limit change?)
		find closest discrete coontroller time (just before or after) ideal change time
		set time at change time


	continue loop

*/

void CPathTimeStep::SetBufferInfo(SContBufferInfo& buffInfo)
{
	buffInfo.AccSeg = m_pCtrlTrack->GetCurrentSeg();
	buffInfo.PathSeg = ms1.seg;
	buffInfo.Time = m_pCtrlTrack->GetCurrentTime();
	buffInfo.Loc = ms1.PathLoc();
	buffInfo.CurrLimit = m_CurrLimit;
	ASSERT(buffInfo.Time - ms1.t <= 1e-6 || AtEndOfPath());
}

void CPathTimeStep::SetToStart()	// can be called before limit list set
{
	CPathMove::SetToStart();

	ASSERT(m_pCtrlTrack != NULL);	// must be set to controller tracker

	m_bSegInitAccStep = false;			// doesn't matter
	m_bNextSegInitAccStep = true;		// allow step on first segment
	m_vtNextSegInitAcc = ms1.vtAcc;

	m_iFinishMovesCount = 0;
	m_bFinishedPathSteps = false;

	// set controller for initial state
//	m_pCtrlTrack->Init();	// done by owner of tracker


	// for Constant Step Method (m_iStepMethod == SM_CONSTSTEP)
	m_dtAdv = m_iStepAdv * m_dT;	// 100e-3


	// for Fit Poly Method (m_iStepMethod == SM_FITPOLY)
	m_PolySegs.SetSmallestXInc(m_dT / 2);
}

void CPathTimeStep::FindInitialTimeStep()		// called after limit list set
{
	FindCurrentLimit();	// find current and next limits
	GetVelAccAtS(ms1);
	if (m_pCtrlTrack->RoomInSegBuffer())
	{
		m_pCtrlTrack->ClearMotionLog();
		m_pCtrlTrack->MoveTo(ms1, 0);	// calc's and stores accel ramps to get to ~ms1 state from it's current state in dTStep time steps
	}
	else
		ASSERT(0);
}

void CPathTimeStep::FindTimeStepsConstStep()
{
	bool bRoomBuffer = m_pCtrlTrack->RoomInSegBuffer();
	if (!bRoomBuffer)		// seg accel buffer is full
		return;

	SetBufferInfo(m_BuffSpan.Init);
	for (;;)
	{
		if (!bRoomBuffer)		// seg accel buffer is full
			break;
		if (AtEndOfPath())
		{
			// do a few moves to arrive exactly at final pos
			int idT;				// close in on pos and vel, step 40, 4, 1
			//char str[80];
			if (m_iFinishMovesCount == 0)		// first time
			{
				idT = 40;
				ASSERT(ms1.s == 1);		// check at end of last segment
				if (ms1.dsdt != 0)			// check stopped
				{
					//ASSERT(ms1.dsdt <= 1e-2);
					ms1.dsdt = 0;
					ms1.vtVel = 0;
				}
				ASSERT(ms1.vtVel.SumAbs() == 0);

				//double* vtdPos = m_pCtrlTrack->GetCurrPosError();
				//sprintf(str, "Finish move %i dPos: %.6f  %.6f  %.6f", m_iFinishMovesCount, vtdPos[0], vtdPos[1], vtdPos[2]);
				//LOGMESSAGE(str);
			}
			else if (m_iFinishMovesCount == 1)
				idT = 4;
			else if (m_iFinishMovesCount == 2)
				idT = 1;
			else
			{
				m_bFinishedPathSteps = true;			// done finishing moves
				break;
			}

			bRoomBuffer = m_pCtrlTrack->StopAtPosAccelStep(ms1, idT);
			m_iFinishMovesCount++;
			//double* vtdPos = m_pCtrlTrack->GetCurrPosError();
			//sprintf(str, "Finish move %i dPos: %.6f  %.6f  %.6f", m_iFinishMovesCount, vtdPos[0], vtdPos[1], vtdPos[2]);
			//LOGMESSAGE(str);
			continue;
		}

		AdvanceBydT(m_dtAdv);
		if (!AtEndOfPath())
		{
			if (ms1.seg > m_EndSeg)
				break;
			if (AtEndOfCalcLimits() )		// usual exit point
				break;
		}
		else
			m_pCtrlTrack->StoreNextPosError();

		SortSegmentAccStep();
		// if change occured, gives state at discrete time nearest change
		// ms1 has state at time (post change if discrete time is exactly change time)
		// msPre & msPost have relative states of stepping values (acc)
		// if a change, store prechange states (in ms0.acc?)
		if (SegInitAccStepped())
			m_pCtrlTrack->AllowStepNextSegment(&GetSegInitAcc());
		bRoomBuffer = m_pCtrlTrack->MoveTo(ms1, GetdTSteps());	// calc's and stores accel ramps to get to ~ms1 state from it's current state in dTStep time steps
	}

	//ASSERT(ms1.PathLoc() >= m_CurrLimit.PathLoc());	// ms1 can be just before m_CurrLimit when step was just before limit change
	//ASSERT(m_TLimit >= 0);
	SetBufferInfo(m_BuffSpan.Final);
}

void CPathTimeStep::SortSegmentAccStep()
{
	// called after  AdvanceBydT(dt)  sets ms1.vtAcc to final segment acc
	// and m_bNextSegInitAccStep to initial segment acc
	m_bSegInitAccStep = m_bNextSegInitAccStep;
//	if (m_bSegInitAccStep)
	m_vtSegInitAcc = m_vtNextSegInitAcc;		// used for extrapolations!

	if (GetChangeLoc() == NO_CHANGE)
	{
		m_bNextSegInitAccStep = false;
//		m_vtSegFinalAcc = ms1.vtAcc;
		m_vtNextSegInitAcc = ms1.vtAcc;		// used for extrapolations!
	}
	else if (GetChangeLoc() == CHANGE_IS_BEFORE_STEP)
	{
		m_bNextSegInitAccStep = true;
//		m_vtSegFinalAcc = msPre.vtAcc;		// extrapolate from change time to step time using vtSegInitAcc
		m_vtNextSegInitAcc = ms1.vtAcc;
		ms1.vtAcc = msPre.vtAcc;
	}
	else if (GetChangeLoc() == CHANGE_IS_AFTER_STEP)
	{
		m_bNextSegInitAccStep = true;
//		m_vtSegFinalAcc = ms1.vtAcc;
		m_vtNextSegInitAcc = msPost.vtAcc;	// extrapolate back to step time from change time when vtNextSegFinalAcc is found!
	}
	else
		ASSERT(0);
	// make sure ms1.vtAcc is set to final acc for this segment
//	ms1.vtAcc = m_vtSegFinalAcc;		// ms1.vtAcc only needs changing if CHANGE_IS_BEFORE_STEP

}

////////////////////


void CPathTimeStep::FindTimeStepsFitPoly()
{
	int dTSteps = 10;		// in time unit (1ms) steps, standard = 100
	double dTSecs = m_dT * dTSteps;

	int nDoChange = NO_CHANGE;
	bool bAccStepThisSeg = false;
	bool bAccStepNextSeg = false;
	CVector vtAccInitThisSeg;
	CVector vtAccInitNextSeg;

// fit polys to as many points as possible
	while (1)
	{
		GetVelAccAtS(ms1);		// get pos, vel, acc at s

//		force a break when a change occurs
		if (nDoChange != NO_CHANGE)
			m_PolySegs.BreakPolyAtNextPoint();		// force a break when polys or limit changes
		int fitStat = m_PolySegs.NextPoint(GetPathTime(), ms1.vtPos.x);
		if (fitStat != CPolySegFitPoints::PSF_OK)
			if (fitStat == CPolySegFitPoints::PSF_REDUCESTEPSIZE)
			{
				dTSecs = m_PolySegs.GetSuggestedXInc();
				m_TLimit = m_PolySegs.GetLastGoodXLoc() - m_TStartLimit;
				nDoChange = NO_CHANGE;
				ASSERT(m_TLimit >= 0);		// check m_TLimit is actuly on this limit
			}


		if (nDoChange == CHANGE_IS_BEFORE_STEP)
		{
			//set previous end accel from pre change
			//set next start accel from step
			if (bAccStepNextSeg)
				vtAccInitNextSeg = ms1.vtAcc;		// value at step after change
			ms1.vtAcc = ms0.vtAcc;	// = vtAccPreChange
		}
				
		// use/store vtPos, vtVel, vtAcc
		if (fitStat == CPolySegFitPoints::PSF_GOTNEXTPOLYSEGMENT)
		{
			if (bAccStepThisSeg)
				m_pCtrlTrack->AllowStepNextSegment(&vtAccInitThisSeg);
			m_pCtrlTrack->MoveTo(ms1, dTSteps);	// calc's and stores accel ramps to get to ~ms1 state from it's current state in dTStep time steps
		}

		if (nDoChange == CHANGE_IS_AFTER_STEP)
		{
			if (!DoChange(bAccStepNextSeg))
				break;
			//set previous end accel from step
			//set next start accel from post change (accelAfter)
			if (bAccStepNextSeg)
				vtAccInitNextSeg = ms1.vtAcc;	// = vtAccPostChange
		}

		if (bAccStepNextSeg)
			vtAccInitThisSeg = vtAccInitNextSeg;
		bAccStepThisSeg = bAccStepNextSeg;
		bAccStepNextSeg = false;					// default

	///////////////////////////////////
	// advance time by time increment and check if there was a change
	///////////////////////////////////

		double TLimitPrev = m_TLimit;
		m_TLimit += dTSecs;		// advance to next time step
/*
		If next time step is past a limit or poly change, move back to time of change

		Take pos and vel at time step, before/after change as appropriate (they don't step anyway).
		But if acc steps, get acc at time step or at change from before change condition
*/
		
		nDoChange = NO_CHANGE;
		if (m_TLimit >= m_TNextChange)	// check time is not past end of current limit or poly
		{
			dTSteps = (int)floor(0.5 + (m_TNextChange - TLimitPrev)/m_dT);	// get closest time step
			m_TLimit = TLimitPrev + dTSecs;
			if (m_TLimit < m_TNextChange)		// if step near change is before change
				nDoChange = CHANGE_IS_AFTER_STEP;	// closest step is before change
			else
				nDoChange = CHANGE_IS_BEFORE_STEP;	// closest step is after change
		}
		m_iTStepPath += dTSteps;


	///////////////////////////////////
	// 
	///////////////////////////////////
		if (nDoChange == CHANGE_IS_BEFORE_STEP)
		{
			if (!DoChange(bAccStepNextSeg))			// if last of polys
				break;
			// vtAccPreChange = ms0.vtAcc;		// will be used instead of step value
			// vtAccPostChange = ms1.vtAcc;		// not used anyway
		}

		double PosLA = GetLimitPosAtLimitTime(m_TLimit);		// calc next pos depending on limit type
		FindSAtPos(ms1, PosLA, m_CurrLimit.axis, 2);			// find S at next position, starts search from ms1 location

	}	// while (1)

}




bool CPathTimeStep::DoChange(bool& bAccSteps)	// try and use CPathMove version!
{
	bAccSteps = false;
	// Get Accel at next change location, before change
	if (m_nChangeType & CNG_SEG)
		ms1.s = 1;
	else if (m_nChangeType & CNG_LIMIT)
	{
		ms1.s = m_NextLimit.s;
//		if (m_NextLimit.s == 0 && m_NextLimit.seg == ms1.seg+1)	// check if CNG_SEG should have been set!! check better when setting!
//		{
//			m_nChangeType |= CNG_SEG;
//			ms1.s = 1;
//		}
	}
	else
		ASSERT(0);
	GetVelAccAtS(ms1);		// get accel at change with current limit

	if (m_nChangeType & CNG_SEG)
	{
		if (!GetNextSegPolys())
			return false;
		ms1.seg++;
		ms1.s = 0;
	}
	if (m_nChangeType & CNG_LIMIT)
	{
		m_CurrLimit = m_NextLimit;
		if (m_posLL >= 0)
			m_NextLimit = m_pLimitList->GetNext(m_posLL);
		else
			m_NextLimit.seg = MAX_SEG;		// end of limit list
		m_TStartLimit += m_TNextChange;		// path time is updated at end of limit
		m_TLimit -= m_TNextChange;		// TChange is limit time in this case

//		PosCL = m_CurrLimit.pos;	// set local variables for speed
//		VelCL = m_CurrLimit.vel;
//		AccCL = m_CurrLimit.acc;
//		AxisCL = m_CurrLimit.axis;
	}
	GetTimeNextChange();		// get next limit or poly change time

	// check for a step change in acceleration in any axis
	ms0.vtAcc = ms1.vtAcc;
//	DEBUG_ONLY(ms0.vtPos = ms1.vtPos);
//	DEBUG_ONLY(ms0.vtVel = ms1.vtVel);

	if (m_nChangeType & CNG_SEG)
		GetVelAccAtS(ms1);			// get accel at change with next limit and new polys
	else
		GetVelAccAtSNoPoly(ms1);	// get accel at change with next limit

	for (int ax = 0; ax < 3; ax++)
		if (fabs(ms1.vtAcc[ax] - ms0.vtAcc[ax]) > m_Tol.accStep)
		{
			bAccSteps = true;
			break;
		}
#ifdef _DEBUG
	for (ax = 0; ax < 3; ax++)
	{
//		ASSERT(ms1.vtPos[ax] == ms0.vtPos[ax]);
//		ASSERT(fabs(ms1.vtVel[ax] - ms0.vtVel[ax]) <= m_Tol.vsmall);
	}
#endif
	return true;
}










