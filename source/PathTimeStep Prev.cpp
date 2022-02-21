// PathTimeStep.cpp: implementation of the CPathTimeStep class.
//
// functions for calculating path time steps used by CNC controller
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ControllerTracker.h"

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
}

CPathTimeStep::~CPathTimeStep()
{
}




/*
previous algorithm for set step length of 100ms
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
	find closet of these

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
void CPathTimeStep::FindTimeSteps()
{
	ASSERT(m_pPathDoc != NULL);	// must have been set by now!
	ASSERT(m_pLimitList != NULL);	// must be set to limit list

// Initial setup
	SetDefaultMembers();		// only tolerances needed!

	m_TPath = 0;			// time from start of path
	m_TLimit = 0;			// time on current limit
	double dT = 1e-3;		// step time = 1ms

	// set initial limit
	m_pLimitList->Reset();
	m_pLimitList->GetNext(m_CurrLimit);
	if (!m_pLimitList->GetNext(m_NextLimit))
		m_NextLimit.seg = INVALID_SEG;

	double PosLA;			// position of limit axis

	double& PosCL = m_CurrLimit.pos;	// set reference (or local) variables for speed
	double& VelCL = m_CurrLimit.vel;
	double& AccCL = m_CurrLimit.acc;
	char& AxisCL = m_CurrLimit.axis;

	// set initial poly
	GetInitSegPolys();
	// set initial path location
	ms1.s = 0;
	ms1.seg = 0;

	GetTimeNextChange();		// get next limit or poly change time


	CControllerTracker ctrlTrack;
	ctrlTrack.SetInitial();
	m_TPathStep = 0;
	int dTSteps = 10;		// in time unit (1ms) steps, standard = 100
	double dTSecs = dT * dTSteps;

	int nDoChange = TIMESTEP_NO_CHANGE;
	bool bAccStepThisSeg = false, bAccStepNextSeg = false;
	CVector vtAccInitThisSeg, vtAccInitNextSeg;


	while (1)
	{
		GetVelAccAtS(ms1);		// get pos, vel, acc at s

// testing for CPolySegFit.....
		int fitStat = m_PolySegs.NextPoint(m_TLimit, ms1.vtPos.x);
		if (fitStat != CPolySegFit::PSF_OK)
			if (fitStat == CPolySegFit::PSF_REDUCESTEPSIZE)
			{
				dTSecs = m_PolySegs.GetSuggestedXInc();
				m_TLimit = m_PolySegs.GetLastGoodXLoc();
				nDoChange = TIMESTEP_NO_CHANGE;
			}
			else if (fitStat == CPolySegFit::PSF_ENDLOOPTOSHOWPLOT)		// for testing to exit thread loop
				break;	// from while (1) loop

// end CPolySegFit test code.....

		if (nDoChange == TIMESTEP_AFTER_CHANGE)
		{
			//set previous end accel from pre change
			//set next start accel from step
			if (bAccStepNextSeg)
				vtAccInitNextSeg = ms1.vtAcc;		// value at step after change
			ms1.vtAcc = ms0.vtAcc;	// = vtAccPreChange
		}
				
		// use/store vtPos, vtVel, vtAcc
		ctrlTrack.MoveTo(ms1, dTSteps, bAccStepThisSeg, &vtAccInitThisSeg);	// calc's and stores accel ramps to get to ~ms1 state from it's current state in dTStep time steps

		if (nDoChange == TIMESTEP_BEFORE_CHANGE)
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
	// advance time by time increment
	///////////////////////////////////

		double TLimitPrev = m_TLimit;
		m_TLimit += dTSecs;		// advance to next time step
/*
		If next time step is past a limit or poly change, move back to
		closest discrete time step to the change.
		Take pos and vel at time step, before/after change as appropriate (they don't step anyway).
		But if acc steps, get acc at time step or at change from before change condition
*/
		
		nDoChange = TIMESTEP_NO_CHANGE;
		if (m_TLimit >= m_TChange)	// check time is not past end of current limit or poly
		{
			dTSteps = (int)floor(0.5 + (m_TChange - TLimitPrev)/dT);	// get closest time step
			m_TLimit = TLimitPrev + dTSecs;
			if (m_TLimit < m_TChange)		// if step near change is before change
				nDoChange = TIMESTEP_BEFORE_CHANGE;	// closest step is before change
			else
				nDoChange = TIMESTEP_AFTER_CHANGE;	// closest step is after change
		}
		m_TPathStep += dTSteps;


	///////////////////////////////////
	// 
	///////////////////////////////////
		if (nDoChange == TIMESTEP_AFTER_CHANGE)
		{
			if (!DoChange(bAccStepNextSeg))			// if last of polys
				break;
			// vtAccPreChange = ms0.vtAcc;		// will be used instead of step value
			// vtAccPostChange = ms1.vtAcc;		// not used anyway
		}

	///////////////////////////////////
	// calc next pos depending on limit type
	///////////////////////////////////
		switch (m_CurrLimit.type)
		{
		case LT_AXIS_ACCELFROM:
		case LT_AXIS_BRAKEFROM:
			PosLA = PosCL + m_TLimit * VelCL + (AccCL/2 * m_TLimit*m_TLimit);	// dPos = dt*Vi + A/2*dt^2
			break;
		case LT_AXIS_VELOCITY:
			PosLA = PosCL + m_TLimit * VelCL;
			break;
		default:
			ASSERT(0);	// bad limit type
		}
		// find S at next position
		FindSAtPos(ms1, PosLA, AxisCL);	// starts search from ms1 location

	}	// while (1)


}

void CPathTimeStep::GetTimeNextChange()
{
	if (m_NextLimit.seg == ms1.seg)	// next change is limit
	{
		m_nChangeType = TC_LIMIT;
		if (m_NextLimit.s == 1)		// 	// next limit is at end of this segment - poly change too
			m_nChangeType |= TC_POLY;
		m_TChange = GetEndLimitTime();
	}
	else	// next change is poly
	{
		m_nChangeType = TC_POLY;
		if (m_NextLimit.seg == ms1.seg+1 && m_NextLimit.s == 0)	// next limit is at start of next segment - limit change too
			m_nChangeType = TC_LIMIT;
		m_TChange = GetEndPolyTime();
	}
}

bool CPathTimeStep::DoChange(bool& bAccSteps)
{
	bAccSteps = false;
	// Get Accel at next change location, before change
	if (m_nChangeType & TC_POLY)
		ms1.s = 1;
	else if (m_nChangeType & TC_LIMIT)
		ms1.s = m_NextLimit.s;
	else
		ASSERT(0);
	GetVelAccAtS(ms1);		// get accel at change with current limit

	if (m_nChangeType & TC_POLY)
	{
		if (!GetNextSegPolys())
			return false;
		ms1.seg++;
		ms1.s = 0;
	}
	if (m_nChangeType & TC_LIMIT)
	{
		m_CurrLimit = m_NextLimit;
		if (!m_pLimitList->GetNext(m_NextLimit))
			m_NextLimit.seg = INVALID_SEG;
		m_TPath += m_TChange;		// path time is updated at end of limit
		m_TLimit -= m_TChange;		// TChange is limit time in this case

//		PosCL = m_CurrLimit.pos;	// set local variables for speed
//		VelCL = m_CurrLimit.vel;
//		AccCL = m_CurrLimit.acc;
//		AxisCL = m_CurrLimit.axis;
	}
	GetTimeNextChange();		// get next limit or poly change time

	// check for a step change in acceleration in any axis
	ms0.vtAcc = ms1.vtAcc;
	DEBUG_ONLY(ms0.vtPos = ms1.vtPos);
	DEBUG_ONLY(ms0.vtVel = ms1.vtVel);

	if (m_nChangeType & TC_POLY)
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
		ASSERT(ms1.vtPos[ax] == ms0.vtPos[ax]);
		ASSERT(fabs(ms1.vtVel[ax] - ms0.vtVel[ax]) <= m_Tol.vsmall);
	}
#endif
	return true;
}


double CPathTimeStep::GetEndPolyTime()
{
	SMotionState ms;
	ms.s = 1;
	GetPolysAtS(ms);	// only need PofS of limit axis!
	SDriveLimit& cl = m_CurrLimit;
	double PosLA = ms.vtPofS[cl.axis];
	double TEndPoly;
	switch (cl.type)
	{
	case LT_AXIS_ACCELFROM:
	case LT_AXIS_BRAKEFROM:
		double b24ac;
		b24ac = cl.vel*cl.vel + 2 * cl.acc * (PosLA - cl.pos);	// b^2-4ac
		ASSERT(b24ac >= 0);
		//		if Accelerating time is the later, if Braking time is the earlier
		if (cl.type == LT_AXIS_ACCELFROM)
			TEndPoly = (-cl.vel + (cl.acc>0?1:-1) * sqrt(b24ac)) / cl.acc;	// solu = (-Vi +/-sqrt(Vi^2 - 2*Ai*Pi)) / Ai
		else	// if (cl.type == LT_AXIS_BRAKEFROM)
			TEndPoly = (-cl.vel + (cl.acc>0?-1:1) * sqrt(b24ac)) / cl.acc;
		break;
	case LT_AXIS_VELOCITY:
		TEndPoly = (PosLA - cl.pos) / cl.vel;
		break;
	}
	return TEndPoly;
}

double CPathTimeStep::GetEndLimitTime()
{
	SMotionState ms;
	ms.s = m_NextLimit.s;
	GetPolysAtS(ms);		// only need PofS (and VofS) of limit axis!
	SDriveLimit& cl = m_CurrLimit;
	double PosLA, VelLA;
	double TEndLimit;
	switch (cl.type)
	{
	case LT_AXIS_ACCELFROM:
	case LT_AXIS_BRAKEFROM:
		VelLA = ms.vtVofS[cl.axis] * m_NextLimit.dsdt;
		TEndLimit = (VelLA - cl.vel) / cl.acc;
/*	or if no ds/dt not avaliable for the next limit
		double b24ac;
		PosLA = ms.vtPofS[cl.axis];
		b24ac = cl.Vel*cl.vel + 2 * cl.acc * (PosLA - cl.pos);	// b^2-4ac
		ASSERT(b24ac >= 0);
		if (cl.type == LT_AXIS_ACCELFROM)
			TEndLimit = (-cl.vel + (cl.acc>0?1:-1) * sqrt(b24ac)) / cl.acc;	// solu = (-Vi +/-sqrt(Vi^2 - 2*Ai*Pi)) / Ai
		else	// if (cl.type == LT_AXIS_BRAKEFROM)
			TEndLimit = (-cl.vel + (cl.acc>0?-1:1) * sqrt(b24ac)) / cl.acc;
		//		if Accelerating time is the later, if Braking time is the earlier
*/
		break;
	case LT_AXIS_VELOCITY:
		PosLA = ms.vtPofS[cl.axis];
		TEndLimit = (PosLA - cl.pos) / cl.vel;
		break;
	}
	return TEndLimit;
}









