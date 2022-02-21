// PathSpeed.cpp: implementation of the CPathSpeed class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNCControl.h"

#include <fstream.h>
#include <iomanip.h>

//#include "PathDoc.h"
#include "PathSpeed.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



/*
	Pos  = P(s)
	Vel  = V(s).ds/dt
	Acc  = A(s).(ds/dt)^2 + V(s).d2s/dt2
	Jerk = J(s).(ds/dt)^3 + 3.A(s).d2s/dt2.ds/dt + V(s).d3s/dt3
	dJerk= dJ(s).(ds/dt)^4 + 6.J(s).d2s/dt2.(ds/dt)^2 + A(s).(4.d3s/dt3.ds/dt + 3.(d2s/dt2)^2) + V(s).d4s/dt4
			 dJ(s) = 0 for cubic P(s)!

Time step (dt) changes:
	dAcc = AccF - AccI
	dVel = dt * (AccI + AccF)/2
	dPos = dt * VelI + dt^2 * (2*AccI + AccF)/6
	     = dt * VelF - dt^2 * (AccI + 2*AccF)/6
	     = dt * (2*VelI + VelF)/3 + dt^2 * AccI/6
	     = dt * (VelI + 2*VelF)/3 - dt^2 * AccF/6

	AccF = 2 * dVel/dt - AccI
	AccF = 6*(dPos - dt*VelI)/dt^2 - 2*AccI
	     = 3*(dt*VelF - dPos)/dt^2 - AccI/2

	PosWt = 0.1; 	// Weighting of Accel calculated to give no position error
	VelWt = 0.45;	// Weighting of Accel calculated to give no velocity error
	AccWt = 0.45;	// Weighting of Accel calculated to give no acceleration error (=acc value)
	Acc = AccWt * NextAcc + VelWt * AccNoVerr + PosWt * AccNoPerr;


ControllerSim::SetScaling(double dTime, double dAccel)
{	These were the unit size of the integer values used by the controller with 100ms step length
	dT = dTime;
	dAcc = dAccel;
	dVel = dAcc * dT / 2;
	dPos = dAcc * dT*dT / 6;
}

	Solution to quadratic: = (-b +/-sqrt(b^2 - 4*a*c)) / 2a
	If position is quadratic: = (-Vi +/-sqrt(Vi^2 - 2*Ai*Pi)) / Ai
			where Pi = c, Vi = b, Ai = 2a


*/
/*
	 Vector Poly
	| Dx  Dy  Dz  |
	| Cx  Cy  Cz  |
	| Bx  By  Bz  |
	| Ax  Ay  Az  |

	 Vector Bezier
	| NIx NIy NIz |
	| CIx CIy CIz |
	| CFx CFy CFz |
	| NFx NFy NFz |

	 Vector BezierRel
	| NIx  NIy  NIz |
	| CIrx CIry CIrz|
	| CFrx CFry CFrz|
	| NFx  NFy  NFz |
*/



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPathSpeed::CPathSpeed()
{
	m_bSaveLimits = false;
}

CPathSpeed::~CPathSpeed()
{
}

/////////////////////////////////////

void CPathSpeed::Init()
{
	CPathTracker::Init();		// call base
	SetDefaults();
}

void CPathSpeed::SetDefaults()
{
	// path calculating flags
	m_bCalcSegmentLengths = false;
	m_bCalcSegmentTimes = false;

	m_dS = g_Settings.MachPathSeg.SegmentTime;

	double angCuspRad = g_Settings.MachParam.AngCuspSegments * deg2rad;
	m_DirMatchAngSq = angCuspRad*angCuspRad;	// angle squared in radians  0.02 -> 1.14deg	(1deg = 0.0175)

	// Path m_ScanSection settings
	m_ScanSection.start.seg = 0;
	m_ScanSection.end.seg = MAX_SEG;		// or MAX_SEG
	m_ScanSection.start.ms.vtVel = 0;
	m_ScanSection.start.ms.vtAcc = 0;
	m_ScanSection.end.ms.vtVel = 0;
	m_ScanSection.end.ms.vtAcc = 0;
	m_ScanSection.start.driveProps = m_DrvPropCurr;
	// m_ScanSection.end.driveProps set at end of foward scan
	m_ScanSection.length = 0;
	m_ScanSection.time = 0;
	m_ScanSection.timeMin = 60;		// do at least 60 sec worth

	m_StartSeg = m_ScanSection.start.seg;
	m_EndSeg = m_ScanSection.end.seg;

}



///////////////////////////////////////////////////


void CPathSpeed::FindSpeeds()		// find next section worth of limits
{
	ASSERT(m_pPathDoc != NULL);	// must have been set by now!
	ASSERT(m_pLimitList != NULL);	// must have been set by now!

// Find foward scan (acceleration) limits
//	TRACE0("Finding Acceleration Limits\n");
	m_ScanSection.length = 0;		// reset scan trackers
	m_ScanSection.time = 0;			// reset scan trackers
	m_ScanSection.start = m_ScanSection.end;	// start from previous end values
	FindAccelLimits();

	if (m_bSaveLimits)
	{
		ofstream	os("Results Foward Limits.txt");
		os << "Foward Limit List\n";
		os << m_FowardLimitList;		// print out limit list
		os.close();
	}

// Find backward scan (braking) limits
//	TRACE0("Finding Braking Limits\n");
	FindBrakeLimits();
	m_BackLimitList.ReverseInto(*m_pLimitList);

	// change first of end braking limits to end of calculated region
	// remove trailing brake limits if a false stop was imposed
	if (m_bFalseStopLimit)
	{
		int idx = FindLastNonBrakeLimit(*m_pLimitList) + 1;	// index of limit to change
		int numRemove = m_pLimitList->GetSize() - 1 - idx;
		if (numRemove < 0)
		{
			idx += numRemove;		// will be last idx
			numRemove = 0;
			ASSERT(0);		// shouldn't happen!
		}
		m_pLimitList->ElementAt(idx).type = LT_ENDCALC;		// change limit to indicate end
		if (numRemove > 0)
			m_pLimitList->RemoveAt(idx + 1, numRemove);		// remove after LT_ENDCALC
	}
//	TRACE0("Finished Limits\n");
	if (m_bSaveLimits)
	{
		ofstream	os("Results Limits.txt");
		os << "Limit List\n";
		os << *m_pLimitList;		// print out limit list
		os.close();
	}

}

/////////////////////////////////////////////

void CPathSpeed::SetToStart()
{
	m_bAtEndOfPath = false;
	m_StartSeg = 0;
	m_ScanSection.start.seg = 0;
	m_ScanSection.start.s = 0;

	GetInitPathProps();
	SetInitialPathLimit();
	StorePathState(m_ScanSection.start);
	m_ScanSection.end = m_ScanSection.start;	// as FindSpeeds get start from current end

}

void CPathSpeed::SetInitialMotion(const SMotionStateBasic& ms)
{
	SMotionStateBasic& msStart = m_ScanSection.start.ms;
	msStart.vtPos = ms.vtPos;
	msStart.vtVel = ms.vtVel;
	msStart.vtAcc = ms.vtAcc;

	m_ScanSection.end = m_ScanSection.start;	// as FindSpeeds get start from current end
}

SContBufferSpan& CPathSpeed::GetBufferStatus()
{
	m_BuffSpan.Init.PathSeg = m_ScanSection.start.seg;
	m_BuffSpan.Final.PathSeg = m_ScanSection.end.seg;
	m_BuffSpan.Init.Time = m_ScanSection.start.ms.t;
	m_BuffSpan.Final.Time = m_ScanSection.end.ms.t;
	return m_BuffSpan;
}

void CPathSpeed::SetInitialPathLimit()
{
	if (!GetSegPolysAt(m_StartSeg))	// use if not starting from seg 0
		return;						// invalid segment! (or no polys)
	SetDriveProps(m_ScanSection.start.driveProps);
//	FindFowardSegPathPropLoc();

	// set drive properties at start of seg
	ms1.seg = m_StartSeg;
	ms1.s = 0;
	GetPolysAtS(ms1);
	m_nStepChange = CNG_SEG | CNG_LIMIT;	// Indicates a path polyminial or property has changed with no foward movement

	CVector& vtVelStart = m_ScanSection.start.ms.vtVel;
	ms1.dsdt = vtVelStart.Mag() / ms1.vtVofS.Mag();		// depending on initial velocity
	ms1.vtVel = ms1.vtVofS * ms1.dsdt;		// only used to check vel match
	ASSERT((ms1.vtVel - vtVelStart).MagSq() <= 1e-6);


	GetMaxAccelAtVel(ms1, 1, &m_Limit);		// sets up limit
	m_Limit.seg = ms1.seg;
	m_Limit.change = LC_VEL_CONTINUOUS;
	m_Limit.tDuration = 0;				// so 0 if not calculated
	m_pLimitList->Add(m_Limit);
	m_Limit.type = LT_ENDCALC;			// add this to set list in usual state
	m_pLimitList->Add(m_Limit);
}

void CPathSpeed::SetEndVelocityLimit(SDriveLimit& limit)
{
	// set end limit to:  m_EndMS.vtVel & m_EndMS.vtAcc
	// requires polys at s
	ASSERT(ms1.seg <= m_EndSeg);
	ASSERT(ms1.s == 1);		// normally but not necessarily!
	GetPolysAtS(ms1);		// would normlly have been done!

	CVector& vtVelEnd = m_ScanSection.end.ms.vtVel;
	double dsdt = vtVelEnd.Mag() / ms1.vtVofS.Mag();		// depending on initial velocity
	if (dsdt < ms1.dsdt)
	{
		ms1.dsdt = dsdt;
		limit.change = LC_VEL_REDUCED;
		if (dsdt == 0)
			limit.change = LC_VEL_ZEROED;
		ms1.vtVel = ms1.vtVofS * ms1.dsdt;		// only used to check vel match
		ASSERT((ms1.vtVel - vtVelEnd).MagSq() <= 1e-6);
	}
	else
		limit.change = LC_VEL_CONTINUOUS;

	char axMaxV = 0;
	for (char ax = 1; ax < 3; ax++)
		if (fabs(ms1.vtVofS[ax]) > fabs(ms1.vtVofS[axMaxV]))
			axMaxV = ax;
	limit.s = ms1.s;
	limit.seg = ms1.seg;
	limit.dsdt = ms1.dsdt;
	limit.d2sdt2 = 0;
	limit.type = LT_AXIS_VELOCITY;
	limit.axis = axMaxV;
	limit.pos = ms1.vtPofS[axMaxV];
	limit.vel = ms1.vtVofS[axMaxV] * ms1.dsdt;
	limit.acc = 0;
}


//////////////////////////////////////////

void CPathSpeed::StorePathState(SPathLocInfo& pli)
{
	pli.s = ms1.s;
	pli.seg = m_Seg.seg;
	pli.nrSegInit = m_Seg.nrInit;
	pli.nrSegFinal = m_Seg.nrFinal;

	pli.pathProps = m_Seg.pathProps;
	pli.driveProps = m_DrvPropCurr;
}

void CPathSpeed::RetrievePathState(SPathLocInfo& pli)
{
	ms1.s = pli.s;
	ms1.seg = pli.seg;
	m_Seg.seg = pli.seg;
	m_Seg.nrInit = pli.nrSegInit;
	m_Seg.nrFinal = pli.nrSegFinal;

	SetPathProps(pli.pathProps);
	SetDrivePropsFromPathProps();
	SetDriveProps(pli.driveProps);
}

int CPathSpeed::FindLastNonBrakeLimit(CLimitList& limitList)
{
	// Scan back through brake limits at end of list to find last accel or vel tpe limit
	int idxLimit = limitList.GetSize() - 1;			//	last limit
	if (idxLimit < 0)
		return idxLimit;
	SDriveLimit* pLim;
	pLim = &limitList.ElementAt(idxLimit);
	if (pLim->type == LT_AXIS_VELOCITY && pLim->vel == 0)
		if (--idxLimit < 0)		// go before last vel limit unless only limit
			idxLimit++;
	while (idxLimit >= 0)
		if (limitList.ElementAt(idxLimit).type != LT_AXIS_BRAKEFROM)
			break;
		else idxLimit--;
	return idxLimit;		// return -1 if all brake limits
}

void CPathSpeed::FindAccelLimits()
{
	m_FowardLimitList.RemoveAll();
	m_FowardLimitList.SetSize(0, 2048);

	// set appropriate start conditions - to where LT_ENDCALC was
	// start from last limit (must be accel or vel type, if start of path an accel from 0 must have been set)

	RetrievePathState(m_ScanSection.start);		// was set to end of previous section
//	int idxLimit = FindLastNonBrakeLimit(*m_pLimitList);
	int idxLimit = m_pLimitList->GetSize() - 2;	// last limit should be LT_ENDCALC
	ASSERT(idxLimit >= 0);
	m_FowardLimitList.Add(m_pLimitList->ElementAt(idxLimit));	// setup first limit for this scan
	SDriveLimit& endLimit = m_pLimitList->ElementAt(idxLimit + 1);
	ASSERT(endLimit.type == LT_ENDCALC);

	int idx = m_FowardLimitList.GetSize() - 1;			//	last limit
	ASSERT(idx >= 0);
	m_CurrLimit = m_FowardLimitList.ElementAt(idx);		// make this latest one the current one
	int lt = m_CurrLimit.type;
	ASSERT(lt == LT_AXIS_ACCELFROM || lt == LT_AXIS_VELOCITY);	// should not be a brake one!

	// move back through segments to limit location
	ASSERT(GetSegNumber() >= endLimit.seg);
	VERIFY(MoveToLocForFoward(endLimit));		// track path props
	SetDrivePropsFromPathProps();

	ms1 = endLimit;


	// start conditions now set, work through path
	int nPreChange = 0;		// prechange flags
	m_bAtEndOfPath = false;
	for (;;)
	{
		///////////////////////////////////
		// Check it at or past a change and set at earliest change loc
		// Check for drive properties change
		if (ms1 >= GetPathPropChangeLoc())	 // if at or passed a property change
		{
			ASSERT(ms1.seg == GetPathPropChangeLoc().seg);
			if (ms1.s > GetPathPropChangeLoc().s)
			{
				nPreChange = 0;						// drive property change only here
				ms1.s = GetPathPropChangeLoc().s;		// this change occurs first
			}
			nPreChange |= CNG_DRIVEPROPS;			// drive property change
		}
		// nPreChange |= CNG_SEG is set if applicable when advancing s


		///////////////////////////////////
		// Calculate and add any new limits up to ms1 until no new limits found
		for (;;)
		{
			GetVelAccAtS(ms1);	// calc ds/dt, d2s/dt2, d3s/dt3 using current drive limit
			CheckAccelLimits();
			if (m_Limit.type == LT_NONE)		// if no new limits incurred
				break;

			ms0.s = m_Limit.s;
			ms0.seg = m_Limit.seg;
			GetVelAccAtS(ms0);		// uses new limit, function does short-cuts when solving at limit start
							// if corner limit, GetVelAccAtS() calc's dsdtMax and Abreakaway
			if (m_Limit.type != LT_CORNER)
				GetdsdtMaxCorner(ms0);		// do for ms0 if not corner

			//	ms1.s remains the same (using new limit)
		}	// while (new limit was found)



		///////////////////////////////////
		// If no changes to do then advance s, else do changes		
		ms0 = ms1;
		if (!nPreChange)	// s was not set due to a pre property change
		{
			// advance s
			ASSERT(ms1.s < 1);
			ms1.s += m_dS;
			if (ms1.s >= (1 - 0.5 * m_dS))	// if comes to within 50% of increment from 1, move to 1
			{
				ms1.s = 1;
				nPreChange |= CNG_SEG;
			}
			m_nStepChange = 0;		// indicates ms0-ms1 is a span not a step
		}
		else		// there are changes so do them
		{
			// if changing segment, get segment length and time before possible drive props change (feedrate) for next segment
			if (nPreChange & CNG_SEG)	// or if (bPreChangeDone && (nPreChange & CNG_SEG))
			{
				if (m_bCalcSegmentLengths)
					m_ScanSection.length += GetSegLength();		// length to start of next segment
				else if (m_bCalcSegmentTimes)
					m_ScanSection.length += GetSegChordLength();				// end to end quick version if just rought time is needed!
				if (m_bCalcSegmentTimes)
				{
					m_ScanSection.time += m_Seg.length / m_DrvPropCurr.feedRate;	// m_PathPropCurr can have -1 feedrate!
					if (m_ScanSection.time >= m_ScanSection.timeMin)	// time in seconds to calc
						break;
				}
			}

			// do drive prop change first so not changed by new poly and new limits will be set if needed by limit change
			if (nPreChange & CNG_DRIVEPROPS)	// last s was property change s (pre property-change set)
			{
				GetNextPathProp();
				m_nStepChange |= CNG_DRIVEPROPS;
			}

			// do poly change after drive props but before limits so no drive props within m_Seg!
			if (nPreChange & CNG_SEG)	// or if (bPreChangeDone && (nPreChange & CNG_SEG))
			{
				ASSERT(ms1.s == 1);
				ASSERT(GetPathPropChangeLoc().seg > ms1.seg);		// check there's no more drive prop changes on this segment
				if (ms1.seg >= m_EndSeg)				// got to end of required scan
					break;
				if (!GetNextSegPolys())
				{
					m_bAtEndOfPath = true;
					break;
				}
				ms1.s = 0;
				ms1.seg = GetSegNumber();
				m_nStepChange |= CNG_SEG;		// Indicates a path polyminial or property has changed with no foward movement
				CheckPolyKnotDirections();		// will restart at 0 speed if direction changes above tolerance
			}

			nPreChange = 0;		// reset all change flags
		}
	}	// while (1)
	// foward scan done now!
	// store states to use when continuing from here
	StorePathState(m_ScanSection.end);

	m_bFalseStopLimit = !m_bAtEndOfPath;

	SetEndVelocityLimit(m_Limit);
	AddNewFowardLimit(m_Limit);
}

void CPathSpeed::AddNewFowardLimit(SDriveLimit& limit)
{
/*	if (m_bCalcSegmentTimes)
	{
		// get time to end of m_CurrLimit
		double t;
		if (m_CurrLimit.type == LT_CORNER)
			t = m_CurrLimit.tDuration;		// it has been summed incrementally in CPathSpeed::CheckAccelLimits() 
		else
		{
			ASSERT(ms1.seg == limit.seg);
			if (ms1.s == limit.s)
			{
				ms1.dsdt = limit.dsdt;
				t = GetLimitTimeUsingdsdtAt(ms1);	// doesn't work when dsdt reduced to 0!
			}
			else		// don't modify ms1 polys!!
			{
				SMotionState ms;
				ms.s = limit.s;
				ms.seg = limit.seg;
				GetPolysAtS(ms);
				ms.dsdt = limit.dsdt;
				t = GetLimitTimeUsingdsdtAt(ms);	// doesn't work when dsdt reduced to 0!
			}
			m_CurrLimit.tDuration = t;		// needs to be updated in list
		}
		// change to add m_CurrLimit to list at end of it's use
		m_FowardLimitList.ElementAt(m_FowardLimitList.GetSize()-1).tDuration = t;
		m_ScanSection.time += t;
		limit.tDuration = 0;		// this one not known yet, resets m_CurrLimit.tDuration for time summing
	}
*/
	if (m_bCalcSegmentTimes)
		limit.tDuration =	m_ScanSection.time;	// not duration, start time of prev poly!

	m_FowardLimitList.Add(limit);
	m_CurrLimit = limit;		// make this latest limit the current limit
}

void CPathSpeed::AddNewBackwardLimit(SDriveLimit& limit)
{
	// also calculates true limit time
/*	if (m_CurrLimit.type == LT_AXIS_BRAKETO)
	{
		ASSERT(ms1 == limit);
		ASSERT(ms1.dsdt == limit.dsdt);
		ASSERT(limit.type == LT_AXIS_BRAKEFROM);
		limit.tDuration = GetLimitTimeAt(ms1, m_CurrLimit);
	}
	else
	{
		ASSERT(ms1 == limit);
		ASSERT(ms1.dsdt == limit.dsdt);
		m_FowardLimit.tDuration = GetLimitTimeAt(ms1, m_FowardLimit);		// calc time of m_FowardLimit
	}
*/
	m_BackLimitList.Add(limit);
	m_CurrLimit = limit;
}

void CPathSpeed::FindBrakeLimits()
{
	// set appropriate end conditions
	// get end from final foward limit
	m_posFLL = m_FowardLimitList.GetMaxPosition();
	ASSERT(m_posFLL >= 1);
	m_FowardLimit = m_FowardLimitList.GetPrev(m_posFLL);

	m_BackLimitList.RemoveAll();
	m_BackLimitList.SetSize(0, 2048);
	m_FowardLimit.tDuration = 0;
//	AddNewBackwardLimit(m_FowardLimit);	// first add last limit from foward scan
	m_BackLimitList.Add(m_FowardLimit);	// first add last limit from foward scan
	m_CurrLimit = m_FowardLimit;


	// set polys for this limit
	ASSERT(GetSegNumber() == m_FowardLimit.seg);		// or >=
	VERIFY(MoveToLocForBackward(m_FowardLimit));
	SetDrivePropsFromPathProps();		// normally unchanged!

	ms1 = m_FowardLimit;		// s normally = 1

	// work through path
	int nPreChange = CNG_INITIAL;		// prechange flags
	bool bEndOfPath = false;
	bool bEndOfLimits = false;
	for (;;)
	{
		///////////////////////////////////
		// Check it at or past a change and set at earliest change loc
		// Check if s is at or past a foward limit change
		if (ms1 <= m_FowardLimit)	// check for a limit change
		{
			ASSERT(ms1.seg == m_FowardLimit.seg);
			if (ms1.s < m_FowardLimit.s)
			{
				nPreChange = 0;						// limit change only here
				ms1.s = m_FowardLimit.s;			// this change occurs first
			}
			nPreChange |= CNG_LIMIT;				// limit change
		}
		// Check if s is at or past a drive properties change
		if (ms1 <= GetPathPropChangeLoc()) // at or passed a property change
		{
			ASSERT(ms1.seg == GetPathPropChangeLoc().seg);
			if (ms1.s < GetPathPropChangeLoc().s)
			{
				nPreChange = 0;						// drive property change only here
				ms1.s = GetPathPropChangeLoc().s;		// this change occurs first
			}
			nPreChange |= CNG_DRIVEPROPS;			// drive property change
		}
		// nPreChange |= CNG_SEG is set if applicable when advancing s


		///////////////////////////////////
		// Calculate and add any new limits up to ms1 until no new limits found
		for (;;)
		{
			if (m_CurrLimit.type == LT_AXIS_BRAKETO)
			{
				GetVelAtS(ms1, m_FowardLimit);
				ms1.dsdtMax = ms1.dsdt;				// dsdtMax for foward limit
				GetVelAccAtSNoPoly(ms1, m_CurrLimit);	// don't have to recalc polys
			}
			else
				GetVelAccAtS(ms1, m_CurrLimit);
			CheckBrakeLimits();
			if (m_Limit.type == LT_NONE)		// if no new limits incurred
				break;

			ms0.s = m_Limit.s;
			ms0.seg = m_Limit.seg;		// probably not actully used, just for reference
			GetVelAccAtS(ms0);		// uses new limit, function does short-cuts when solving at limit start
		//	ms1.s remains the same (using new limit)
		}	// while (1) - new limit was found


		///////////////////////////////////
		// If no changes to do then advance s, else do changes		
		ms0 = ms1;
		if (!nPreChange)		// s was not set due to a prechange and not at start of poly
		{
			// decrease s
			ASSERT(ms1.s > 0);
			ms1.s -= m_dS;
			if (ms1.s <= (0 + 0.5 * m_dS))	// if comes to within 50% of increment from 0, move to 0
			{
				ms1.s = 0;
				nPreChange |= CNG_SEG;
			}
			m_nStepChange = 0;		// indicates ms0-ms1 is a span not a step
		}
		else		// there are changes so do them
		{
			// do drive prop change first so not changed by new poly and new limits will be set if needed by limit change
			if (nPreChange & CNG_DRIVEPROPS)	// last s was property change s (pre property-change set)
			{
				GetPrevPathProp();
				m_nStepChange |= CNG_DRIVEPROPS;	// Indicates a path polyminial or property has changed with no foward movement
			}

			// do poly change after drive props but before limits so no drive props within m_Seg!
			if (nPreChange & CNG_SEG)	// or if (bPreChangeDone && (nPreChange & CNG_SEG))
			{
				ASSERT(ms1.s == 0);
				ASSERT(GetPathPropChangeLoc().seg < ms1.seg);		// check there's no more drive prop changes on this segment
				if (ms1.seg <= m_StartSeg)			// got back to start of required scan
				{
					int lt = m_CurrLimit.type;
					if (lt >= LT_MIN_NORMAL && lt <= LT_MAX_NORMAL)
						break;			// check if ready to finish - not finalised limit			
				}
				if (!GetPrevSegPolys())
				{
					bEndOfPath = true;
					break;
				}
				ms1.s = 1;
				ms1.seg = GetSegNumber();
				m_nStepChange |= CNG_SEG;		// Indicates a path polyminial or property has changed with no foward movement, ms0/1 is before/after step
				m_Limit.seg = ms1.seg;			// required here??
				//	CheckPolyKnotDirections()	velocities are matched limit changes
			}

			// do limit change after poly so it can use new poly properties, and after drive prop so new limits will be set if needed by limit change
			if (nPreChange & CNG_LIMIT)	// last was pre limit change location
			{
				// m_FowardLimit changes here
				// s remains the same
				int iLastFwdChange = m_FowardLimit.change;
				if (m_posFLL >= 0)
				{
					m_FowardLimit = m_FowardLimitList.GetPrev(m_posFLL);	// end of limit list
					ASSERT(m_FowardLimit.s != 1);		// should start from s=0 of next segment instead - except final limit
				}
				else
				{
					m_FowardLimit.SetPreStart();		// will never reach this
					bEndOfLimits = true;
					break;
				}

				if (m_CurrLimit.type != LT_AXIS_BRAKETO)	// not currently braking at all
				{
					if (iLastFwdChange & LC_VEL_REDUCED)	// or if prev limit is LT_CORNER
					{
						// set BrakeTo limit - can be end of corner limit, or step reduction at poly or drive properties change
						if (nPreChange & CNG_SEG)		// polys are changing too!
						{
							ASSERT(ms1.s == 1 && ms0.s == 0);	// make sure polys have been changed
							// match vel across poly change, ms0 will be s==0 for next segment
							GetPolysAtS(ms1);		// not done yet for ms1 if s==1 cause polys just changed
							if (ms0.dsdt == 0)
								ms1.dsdt = 0;
							else
								ms1.dsdt = ms0.dsdt * sqrt(ms0.vtVofS.MagSq() / ms1.vtVofS.MagSq());		// less sqrt's
						}
						// make sure following is done after any drive prop change so new drive limits are used!
						GetMaxAccelAtVel(ms1, -1, &m_Limit);	// sets max braking axis
						m_Limit.type = LT_AXIS_BRAKETO;
						m_Limit.change = LC_VEL_CONTINUOUS;
						m_CurrLimit = m_Limit;
					}
					else
					{
						AddNewBackwardLimit(m_FowardLimit);	// use existing foward limit
					}
				}
			}	// if (nPreChange & CNG_LIMIT)

			nPreChange = 0;		// reset all change flags
		}	// if (nPreChange)
	}	// while (1)
	// backward scan done now!

}


//////////////////////////////////////////

void CPathSpeed::CheckAccelLimits()
{
	m_Limit.type = LT_NONE;		// key for not set
	m_Limit.seg = ms1.seg;

	// Check for any axis over limit velocity
	CheckVelocity();

	// Check for any axis over limit accelerations
	if (m_CurrLimit.type != LT_CORNER)	// If not on corner limit
	{
		CheckCornerVelocity();
		CheckAcceleration();
	}
	else						// on corner limit
	{
		CheckEndCornerLimit();
/*		if (m_bCalcSegmentTimes)	// approx min time by summing avg(dt/ds).ds
		{
			// if just changed to corner limit, ms0 will have been calc'ed at start
			ASSERT(ms1.s != 0 || ms0.s == 1);	// check 1->0 step
			ASSERT(ms1.s == 0 || ms0.s != 1);
			// if this corner limit ends, use limit.(s, dsdt)
			double s1 = (m_Limit.type == LT_NONE) ? ms1.s : m_Limit.s;	// ms1.seg == m_Limit.seg anyway!
			double dsdt1 = (m_Limit.type == LT_NONE) ? ms1.dsdt : m_Limit.dsdt;
			ASSERT(ms0.dsdt > 0);
			ASSERT(dsdt1 > 0);	// dsdt = dsdtMax for corner
			double dt = (1/ms0.dsdt + 1/dsdt1) * 0.5 * (s1 - ms0.s + (ms1.seg-ms0.seg));	// carefull of seg change
			ASSERT(!m_nStepChange || dt == 0);
			m_CurrLimit.tDuration += dt;
		}
*/
	}
	
	//******** Store any new limits ***********
	if (m_Limit.type != LT_NONE)
		AddNewFowardLimit(m_Limit);
}

void CPathSpeed::CheckBrakeLimits()
{
	m_Limit.type = LT_NONE;		// key for not set
	m_Limit.seg = ms1.seg;

	// Check for any axis over limit accelerations
	CheckDeacceleration();	// check if any axis over braking -> change braking axis
	if (m_CurrLimit.type == LT_AXIS_BRAKETO)
		CheckStartBraking();	// check if braked dsdt >= foward dsdt -> come off braketo limit and apply brake from == point


	//******** Store any new limits ***********
	if (m_Limit.type != LT_NONE)
	{
		// replace current BrakeTo with BrakeFrom
		ASSERT(m_CurrLimit.type == LT_AXIS_BRAKETO);
		m_CurrLimit.type = LT_AXIS_BRAKEFROM;
		m_CurrLimit.change = m_Limit.change;
		m_CurrLimit.seg = m_Limit.seg;
		m_CurrLimit.s = m_Limit.s;
		m_CurrLimit.dsdt = m_Limit.dsdt;
		m_CurrLimit.d2sdt2 = m_EndCurrLimit.d2sdt2;
		m_CurrLimit.pos = m_EndCurrLimit.pos;	// set from CurrLimit axis when m_Limit was set
		m_CurrLimit.vel = m_EndCurrLimit.vel;
//		m_CurrLimit.acc = same;
		AddNewBackwardLimit(m_CurrLimit);

		if (m_Limit.type == LT_STARTBRAKE)	// if start brake add limit from foward limits, else add new (BrakeTo) limit
		{
			ASSERT(m_FowardLimit.type != LT_CORNER);	// should not get to LT_STARTBRAKE while still on LT_CORNER
			AddNewBackwardLimit(m_FowardLimit);		// make this latest one the current one
		}
		else	// generally another LT_AXIS_BRAKETO
		{
			if (m_Limit.type != LT_AXIS_BRAKETO)
				AddNewBackwardLimit(m_Limit);
			else
				m_CurrLimit = m_Limit;		// make this latest one the current one
		}
		ASSERT(m_CurrLimit.type != LT_CORNER);		// must not be an intermediate type
	}
}


///////////////////////////////////////////////////

void CPathSpeed::CheckVelocity()
{
	if (ms1.dsdt == DSDTMAX_NOLIMIT)	// vel not valid anyway - only if still on corner limit and step change
	{
		ASSERT(m_nStepChange != 0);	// should only occur for step change
		return;
	}
	TVector<char> vtOverVel;
	vtOverVel = (ms1.vtVel > vtVelMaxTol) || (ms1.vtVel < vtVelMinTol);
	if (m_nStepChange)	// for a change in drive properties (speed reduction)
	{
		if (vtOverVel.Any())			// overspeed, reduce velocity limit
		{
			char axLimit = -1;
			double dsdtLimit;
			double velLimit;
			for (char ax = 0; ax < 3; ax++)
			{
				if (vtOverVel[ax] == false)		// do only for axis that are over velocity
					continue;
				if (ms1.vtVofS[ax] == 0)
					continue;
				double vel = ms1.vtVofS[ax] > 0 ? vtVelMax[ax] : vtVelMin[ax];
				double dsdt = vel / ms1.vtVofS[ax];
				if (dsdt < dsdtLimit || axLimit == -1)
				{
					dsdtLimit = dsdt;
					axLimit = ax;
					velLimit = vel;
				}
			}
			ASSERT(axLimit >= 0);
			if (dsdtLimit < m_Limit.dsdt || m_Limit.type == LT_NONE)	// store this one if slower
			{
				m_Limit.s = ms1.s;
				m_Limit.dsdt = dsdtLimit;
				m_Limit.axis = axLimit;
				m_Limit.type = LT_AXIS_VELOCITY;
				m_Limit.pos = ms1.vtPos[axLimit];		// not essential
				m_Limit.vel = velLimit;
				m_Limit.acc = 0;						// not essential
				if (m_nStepChange & CNG_DRIVEPROPS)
					m_Limit.change = LC_VEL_REDUCED;		// For velocity reduced
				else						// can allow a small step change due to seg direction matching tolerence
					m_Limit.change = LC_VEL_CONTINUOUS;		// vel change should only be small
			}
		}
		else		// not over speed, but check if on vel limit and limit has increased so can accelerate
		{
			if (m_nStepChange & CNG_DRIVEPROPS)		// check if both vel and corner limits occur on step??;
			{
				if (m_CurrLimit.type != LT_AXIS_VELOCITY)
					return;
				int ax = m_CurrLimit.axis;
				double vel = m_CurrLimit.vel;
				if (fabs(vel) >= (vel>0 ? vtVelMax[ax] : vtVelMin[ax]))	// shouldn't be >
					return;
				// velocity limit increased, can accelerate
				if (m_Limit.type == LT_NONE)
				{
					GetMaxAccelAtVel(ms1, 1, &m_Limit);
					m_Limit.change = LC_VEL_CONTINUOUS;
				}
			}
		}
		return;
	}	// if (m_nStepChange)
	// not step change
	// Find s value of earliest Vel limit
	if (!vtOverVel.Any())
		return;
	SMotionState ms;
	ms.seg = ms1.seg;
	for (char ax = 0; ax < 3; ax++)
	{
		if (vtOverVel[ax] == false)		// do only for axis that are over velocity
			continue;		
		CSeekValue limVel(ms1.vtVel[ax] > 0 ? vtVelMax[ax] : vtVelMin[ax]);
		limVel.Point(ms0.s, ms0.vtVel[ax]);
		limVel.Point(ms1.s, ms1.vtVel[ax]);
		for (;;)
		{
			ms.s = limVel.NextSGuess();
			GetVelAtS(ms);
			if (fabs(limVel.ValueError(ms.vtVel[ax])) <= m_Tol.vel)
				break;
			limVel.Point(ms.s, ms.vtVel[ax]);
		}
		ASSERT(ms.s >= 0 && ms.s <= 1);
		if (ms.s < m_Limit.s || m_Limit.type == LT_NONE)	// store this one if earlier
		{
			m_Limit = ms;
			m_Limit.axis = ax;
			m_Limit.pos = ms.vtPos[ax];		// not essential
			m_Limit.vel = limVel.Value();
			m_Limit.type = LT_AXIS_VELOCITY;
			m_Limit.acc = 0;						// not essential
			m_Limit.change = LC_VEL_CONTINUOUS;		// For velocity continuous
		}
	}
}


void CPathSpeed::CheckAcceleration()
{
// Find s for any axis over accelerating in it's velocity direction (speeding up)
	TVector<char> vtOverAcc;
	CVector vtZero;
	vtZero = 0.0;
	vtOverAcc = ((ms1.vtAcc > vtAccMaxTol) && ms1.vtVofS > vtZero)
				|| ((ms1.vtAcc < vtAccMinTol) && ms1.vtVofS < vtZero);	// Accelerating when Acc & Vel are same direction
	if (m_nStepChange)
	{
		if (vtOverAcc.Any())
		{
			if (m_Limit.type == LT_NONE)		// only set if no other limits applied
			{
				GetMaxAccelAtVel(ms1, 1, &m_Limit);
				m_Limit.change = LC_VEL_CONTINUOUS;		// For velocity continuous
			}
		}
		else if (m_nStepChange & CNG_DRIVEPROPS)
		{	// if not over accel check if on accel limit and limit has increased
			if (m_CurrLimit.type != LT_AXIS_ACCELFROM)
				return;
			int ax = m_CurrLimit.axis;
			double acc = m_CurrLimit.acc;
			if (fabs(acc) >= fabs(acc>0 ? vtAccMax[ax] : vtAccMin[ax]))	// shouldn't be >
				return;
			// acceleration limit increased
			if (m_Limit.type == LT_NONE)
			{
				GetMaxAccelAtVel(ms1, 1, &m_Limit);
				m_Limit.change = LC_VEL_CONTINUOUS;
			}
		}
		return;
	}

	if (!vtOverAcc.Any())
		return;
	// Find s value of earliest Acc limit
	SMotionState ms;
	ms.seg = ms1.seg;
	for (char ax = 0; ax < 3; ax++)
	{
		if (vtOverAcc[ax] == false)
			continue;
		//	GtSatAcc.m in matlab
		CSeekValue limAcc(ms1.vtAcc[ax] > 0 ? vtAccMax[ax] : vtAccMin[ax]);
		limAcc.Point(ms0.s, ms0.vtAcc[ax]);
		limAcc.Point(ms1.s, ms1.vtAcc[ax]);
		while (1)			// find s where axis accel reaches limit
		{
			ms.s = limAcc.NextSGuess();
			GetVelAccAtS(ms);
			if (fabs(limAcc.ValueError(ms.vtAcc[ax])) <= m_Tol.acc)
				break;
			limAcc.Point(ms.s, ms.vtAcc[ax]);
		}
		ASSERT(ms.s >= 0 && ms.s <= 1);
		if (ms.s < m_Limit.s || m_Limit.type == LT_NONE)	// store this one if earlier
		{
			m_Limit = ms;				// set s, dsdt, d2sdt2
			m_Limit.axis = ax;
			m_Limit.type = LT_AXIS_ACCELFROM;
			m_Limit.change = LC_VEL_CONTINUOUS;		// For velocity continuous
			m_Limit.pos = ms.vtPos[ax];			// not essential
			m_Limit.vel = ms.vtVel[ax];			// not essential
			m_Limit.acc = limAcc.Value();
		}
	}	//	for (int ax = 0; ax < 3; ax++)
}

void CPathSpeed::CheckDeacceleration()
{
// Find s for any axis over braking, accel opposite to it's velocity direction
	TVector<char> vtOverAcc;
	CVector vtZero = 0;
	vtOverAcc = ((ms1.vtAcc > vtAccMaxTol) && ms1.vtVofS < vtZero)
				|| ((ms1.vtAcc < vtAccMinTol) && ms1.vtVofS > vtZero);	// Braking when Acc & Vel are opposite direction
	if (m_nStepChange)
	{
		if (vtOverAcc.Any())
		{
			ASSERT(m_CurrLimit.type == LT_AXIS_BRAKETO);	// should only occur when braking
			if (m_Limit.type == LT_NONE)		// only set if no other limits applied
			{
				GetMaxAccelAtVel(ms1, -1, &m_Limit);	// gets max neg accel (brake)
				m_Limit.change = LC_VEL_CONTINUOUS;		// For velocity continuous
				int ax = m_CurrLimit.axis;
				m_EndCurrLimit.pos = ms1.vtPos[ax];	// m_CurrLimit is a BrakeTo!
				m_EndCurrLimit.vel = ms1.vtVel[ax];
				m_EndCurrLimit.d2sdt2 = ms1.d2sdt2;
			}
		}
		else if (m_nStepChange & CNG_DRIVEPROPS)
		{	// if not over deaccel check if on deaccel limit and limit has increased
			if (m_CurrLimit.type != LT_AXIS_BRAKETO)
				return;
			int ax = m_CurrLimit.axis;
			double acc = m_CurrLimit.acc;
			if (fabs(acc) >= fabs(acc>0 ? vtAccMax[ax] : vtAccMin[ax]))	// shouldn't be >
				return;
			// acceleration limit increased
			if (m_Limit.type == LT_NONE)
			{
				GetMaxAccelAtVel(ms1, -1, &m_Limit);
				m_Limit.change = LC_VEL_CONTINUOUS;
				m_EndCurrLimit.pos = ms1.vtPos[ax];	// m_CurrLimit is a BrakeTo!
				m_EndCurrLimit.vel = ms1.vtVel[ax];
				m_EndCurrLimit.d2sdt2 = ms1.d2sdt2;
			}
		}
		return;
	}

	if (!vtOverAcc.Any())
		return;
	ASSERT(m_CurrLimit.type == LT_AXIS_BRAKETO);	// should only occur when braking
	// Find s value of earliest Acc limit
	SMotionState ms;
	ms.seg = ms1.seg;
	for (char ax = 0; ax < 3; ax++)
	{
		if (vtOverAcc[ax] == false)
			continue;
		//	GtSatAcc.m in matlab
		CSeekValue limAcc(ms1.vtAcc[ax] > 0 ? vtAccMax[ax] : vtAccMin[ax]);
		limAcc.Point(ms0.s, ms0.vtAcc[ax]);
		limAcc.Point(ms1.s, ms1.vtAcc[ax]);
		while (1)			// find s where axis accel reaches limit
		{
			ms.s = limAcc.NextSGuess();
			GetVelAccAtS(ms);
			if (fabs(limAcc.ValueError(ms.vtAcc[ax])) <= m_Tol.acc)
				break;
			limAcc.Point(ms.s, ms.vtAcc[ax]);
		}
		ASSERT(ms.s >= 0 && ms.s <= 1);
		if (ms.s > m_Limit.s || m_Limit.type == LT_NONE)	// store this one if later (for Braking)
		{
			m_Limit = ms;				// set s, dsdt, d2sdt2
			m_Limit.axis = ax;
			m_Limit.type = LT_AXIS_BRAKETO;
			m_Limit.change = LC_VEL_CONTINUOUS;		// For velocity continuous
			m_Limit.pos = ms.vtPos[ax];
			m_Limit.vel = ms.vtVel[ax];
			m_Limit.acc = limAcc.Value();
			m_EndCurrLimit.pos = ms.vtPos[m_CurrLimit.axis];	// m_CurrLimit is a BrakeTo!
			m_EndCurrLimit.vel = ms.vtVel[m_CurrLimit.axis];
			m_EndCurrLimit.d2sdt2 = ms.d2sdt2;
		}
	}	//	for (int ax = 0; ax < 3; ax++)
}

void CPathSpeed::CheckStartBraking()
{
// check if braked dsdt > foward dsdt -> come off braketo limit and apply brake from == point and resume foward limit
	ASSERT(m_CurrLimit.type == LT_AXIS_BRAKETO);
	ASSERT(ms1.dsdtMax != DSDTMAX_NOLIMIT);	// can only be if m_FowardLimit.type == LT_CORNER anyway
	if (ms1.dsdt <= ms1.dsdtMax)	// (changed from < to <= on 20/07/02)  hasn't got back to acceleration speed yet
		return;							// dsdtMax is from foward limit, dsdt from current limit
	if (ms1 == m_CurrLimit)
		return;				// still at brake to point so dsdt will match, calc error may make ms1.dsdt > ms1.dsdtMax
	if (m_FowardLimit.type == LT_CORNER)
		return;				// is not normally, sometimes is when dsdt's are close and changing, find start braking later
	ASSERT(m_FowardLimit.type != LT_CORNER);
	if (m_nStepChange)
	{
		if (fabs(ms1.dsdt - ms1.dsdtMax) > (1+ms1.dsdt)*m_Tol.dsdt)	// ms1.dsdt - ms1.dsdtMax should not step, only due to calc tolerences - if dsdt<0 (1+fabs(ms1.dsdt))
		{	// dsdt should not step, apart for small step on seg change if directions close but not exact
			ASSERT(m_nStepChange & CNG_SEG);
		}
		if (m_Limit.type == LT_NONE || m_Limit.type == LT_AXIS_BRAKETO)	// only set if no other limits applied
		{	// overwrite a new braketo limit
			ASSERT(m_FowardLimit.type != LT_CORNER);
			m_Limit = ms1;
			m_Limit.type = LT_STARTBRAKE;
			m_Limit.change = LC_VEL_CONTINUOUS;		// For velocity continuous
			m_EndCurrLimit.pos = ms1.vtPos[m_CurrLimit.axis];	// m_CurrLimit is a BrakeTo!
			m_EndCurrLimit.vel = ms1.vtVel[m_CurrLimit.axis];
			m_EndCurrLimit.d2sdt2 = ms1.d2sdt2;
		}
		return;
	}

	// Find where dsdt(from braking) == dsdtMax(from accel)
	SMotionState ms;
	CSeekValue matchSpeedLoc(0);
	double dsdtOver = ms0.dsdt - ms0.dsdtMax;	// ms0.dsdtMax will have been calc for all limit types!
	matchSpeedLoc.Point(ms0.s, dsdtOver);
	dsdtOver = ms1.dsdt - ms1.dsdtMax;
	ms.s = ms1.s;
	ms.seg = ms1.seg;
	do
	{
		matchSpeedLoc.Point(ms.s, dsdtOver);
		ms.s = matchSpeedLoc.NextSGuess();
		GetVelAtS(ms, m_FowardLimit);	// for dsdt
		ms.dsdtMax = ms.dsdt;			// set dsdtMax from foward (accel) speed
		GetVelAtSNoPoly(ms, m_CurrLimit);	// don't have to recalc polys
		dsdtOver = ms.dsdt - ms.dsdtMax;
	} while (fabs(dsdtOver) > max(ms.dsdt, ms.dsdtMax) * m_Tol.dsdt);	// dsdt is always >= 0
	GetVelAccAtSNoPoly(ms, m_CurrLimit);	// get Acc, d2sdt2 if required?

	ASSERT(ms.s >= 0 && ms.s <= 1);
	if (ms.s > m_Limit.s || m_Limit.type == LT_NONE)	// store this one if later (for Braking)
	{
		m_Limit = ms;				// set s, dsdt, d2sdt2 - use braked ds/dt not foward, matching pos & vel
		m_Limit.type = LT_STARTBRAKE;		// will add the current foward limit
		m_Limit.change = LC_VEL_CONTINUOUS;		// For velocity continuous
		m_EndCurrLimit.pos = ms.vtPos[m_CurrLimit.axis];	// m_CurrLimit is a BrakeTo!
		m_EndCurrLimit.vel = ms.vtVel[m_CurrLimit.axis];
		m_EndCurrLimit.d2sdt2 = ms.d2sdt2;
	}
}

void CPathSpeed::CheckCornerVelocity()
{
	// Check on maximum allowable velocity due to velocity normal (cornering) acceleration limits
	GetdsdtMaxCorner(ms1);
	if (ms1.dsdtMax == DSDTMAX_NOLIMIT)
		return;				// Do only if there is SOME cornering acceleration, ie. not a straight line
	if (ms1.dsdt <= ms1.dsdtMax + m_Tol.dsdt)
		return;				// ds/dt is below maximum
	if (m_nStepChange)		// If step change and too fast, either start corner limit, or reduce velocity and keep accelerating if OK 
	{
		// store this limit over a velocity limit if one exists
		GetCurveBreakaway(ms1);		// If Abreakaway >= 0 OK to accelerate after reducing velocity

		if (ms1.dsdtMax > m_Limit.dsdt && m_Limit.type != LT_NONE)	// if vel is more than an existing new limit
			return;
		if (ms1.Abreakaway >= 0)		// If at S=0 and Abreakaway >= 0, Don't set cornering limit, velocity will drop below max anyway
		{
			double dsdtTemp = ms1.dsdt;			// store temporarily
			ms1.dsdt = ms1.dsdtMax;		// don't change current ms1.dsdt permanently
			GetMaxAccelAtVel(ms1, 1, &m_Limit);		// sets m_Limit - uses current members of ms1
			ms1.dsdt = dsdtTemp;			// restore
			m_Limit.type = LT_AXIS_ACCELFROM;	// done in GetMaxAccelAtVel() anyway
			m_Limit.change = LC_VEL_REDUCED;		// For velocity reduced
		}
		else
		{
			m_Limit = ms1;						// set s, dsdt, d2sdt2
			m_Limit.dsdt = ms1.dsdtMax;
			m_Limit.axis = -1;				// n/a or axis which has to over-brake
			m_Limit.acc = 0;					// n/a
			m_Limit.type = LT_CORNER;		// For Corner Velocity limit
			m_Limit.change = LC_VEL_REDUCED;		// For velocity reduced
		}
		return;
	}

	// Find s where ds/dt goes above allowable max corner ds/dt
	SMotionState ms;
	CSeekValue overSpeedLoc(0);
	double dsdtOver = ms0.dsdt - ms0.dsdtMax;	// ms0.dsdtMax will have been calc for all limit types!
	overSpeedLoc.Point(ms0.s, dsdtOver);
	dsdtOver = ms1.dsdt - ms1.dsdtMax;
	ms.s = ms1.s;
	ms.seg = ms1.seg;
	do
	{
//		dsdtOver was 1e-14 first then 0.37 - not in range but first == 0
		overSpeedLoc.Point(ms.s, dsdtOver);
		ms.s = overSpeedLoc.NextSGuess();
		GetVelAtS(ms);				// for ms.dsdt
//			GetVelAccAtS(ms);				// for testing gets acc as well
		GetdsdtMaxCorner(ms);	// for ms.dsdtMax
		dsdtOver = ms.dsdt - ms.dsdtMax;
	} while (fabs(dsdtOver) > m_Tol.dsdt);		// changed from m_Tol.vel 18/3
	ms.dsdt = ms.dsdtMax;	// doesn't really matter as  it's within m_Tol.dsdt

	ASSERT(ms.s >= 0 && ms.s <= 1);
	if (ms.s < m_Limit.s || m_Limit.type == LT_NONE)	// store this one if earlier
	{
		m_Limit = ms;						// set s, dsdt, d2sdt2
		m_Limit.axis = -1;				// n/a or axis which has to over-brake
		m_Limit.type = LT_CORNER;		// For Corner Velocity limit
		m_Limit.acc = 0;					// n/a
		m_Limit.change = LC_VEL_REDUCED;		// velocity reduced for corner
	}
}


void CPathSpeed::CheckEndCornerLimit()
{
// Check if near a local maximum curvature (or adjusted curve for direction dependent acceleration)
// If at a max curve (therefore velocity minimum) set Acc limit on appropriate axis

//	GetCurveBreakaway(ms1);		// Done in 'GetVelAccAtS()' with limit type == LT_CORNER
	if (m_nStepChange)
	{
		// check if max corner velocity increases over step, if so start off from previous velocity
		// could check vel change is > a vel change tolerence
		if (ms1.vtVel.MagSq() > (ms0.vtVel.MagSq() + m_Tol.vsmall) || ms1.dsdtMax == DSDTMAX_NOLIMIT)		// ms0/1.vtVofS can change magnitude if polys changed! (dsdtMax<0 means no limit)
		{												// if a step inc in allowable vel - can resume an accel limit
			ms1.dsdt = sqrt(ms0.vtVel.MagSq() / ms1.vtVofS.MagSq());	// use lower velocity from before. Velmag = VofSmag * ds/dt
			ms1.Abreakaway = 1;			// Force to set acc using ds/dt above
		}
		if (ms1.Abreakaway >= 0)
		{
			if (ms1.dsdt > m_Limit.dsdt && m_Limit.type != LT_NONE)	// if vel is more than an existing new limit
				return;
			GetMaxAccelAtVel(ms1, 1, &m_Limit);
			m_Limit.change = LC_VEL_REDUCED;		// For velocity reduced
		}
		return;
	}

	if (ms1.Abreakaway <= 0)		// changed from < to <= 5/03, an acceleration limit would over-speed corner
		return;
/*
	Find point where axis acceleration can be resumed without corner accel going too high
	ie. where Abreakaway changes from neg to pos through zero
	Check for any step changes in Abreakaway from ms0 to ms1
	These can occur when bound edge for Accel boxlimit changes
	ie. when a component of vtBperp changes sign

from matlab:	These can occur due to AFoward step changing when edge limit changes if an axis component
	of Anormdir changes sign.  When doing planar move (Vz=0) is at a Vaxis == 0
	but not for a general 3D move.
	Or due to a step change in AccforMaxV when either:
	AccTot step changes due to edge limit change as above condition
	or dAccTot step changes due to AccTot passing through a limit cube apex
	therefor where abs(AccTot) == Amax
*/
	// Axis with max d3S/dT3 gets Accel limit ???
//		GtSMaxCv (matlab)		// Get S at maximum curve where dAdjCurvemag == 0

//	GetAccBound(ms);

	SMotionState ms, msStart, msEnd;
	msStart.SetAccBoundMembers(ms0);	// set start and end bounds which used to find any step changes in Bound
	msEnd.SetAccBoundMembers(ms1);	// msEnd really only needs: s, Abreakaway.

	CSeekValue endCorner(0);
	endCorner.Point(ms0.s, ms0.Abreakaway);	// ms0.Abreakaway will have been calc for corner limit at least!
	ms.SetAccBoundMembers(ms1);
	while (fabs(ms.Abreakaway) > m_Tol.acc)
	{
		int seekRes = endCorner.Point(ms.s, ms.Abreakaway);
		// Only check for bound change if last S becomes an S high or low limit
		if (seekRes & (CSeekValue::SR_HIGHORLOW))
		{
			for (int axChanged = 0; axChanged < 3; axChanged++)	// Check if a box bound changes (determined by sign(vtAnormDir)
				if (ms.vtAccBound[axChanged] != msStart.vtAccBound[axChanged])
					break;
/*
	check if the acceleration bound edge changes over the current span

	if (there is a change of bound sign):
		{	find where sign changes (vtAnorm[ax] == 0)
		{	find Abreakaway before/after at point for +/- bounds
	else if (bound changes 0 <-> +/-1) (two axis will change)
		{	find where change occurs, where Ma(ax1) == Ma(ax2)
		{	find Abreakaway before/after at point for both bounds

	if (AbreakawayBefore > 0)
		{	set point as end point
		{	using 'before' bound
	else if (AbreakawayAfter >= 0)
		{	found Abreakaway == 0 point!
	else
		{	set point as start point
		{	using 'after' bound
*/
			if (axChanged < 3)		// if so, find s where change occurs and check for Abreakaway stepping through 0
			{
				double arAbreakaway[2];			// [0] is Before, [1] is After
				TVector<char> arvtBound[2];	// [0] is Before, [1] is After
				// First check if any bound axis has changed sign (otherwise change is 0 <-> +/-1)
				for (int axCngSign = 0; axCngSign < 3; axCngSign++)	// Check if a box bound changes sign (determined by sign(vtAnormDir)
					if (msStart.vtAccBound[axCngSign] * ms.vtAccBound[axCngSign] == -1)
						break;
				if (axCngSign < 3)	// Bound sign changed, find when vtAnormDir[axCngSign] == 0 (or vtBperp which is same direction)
					FindAnormSignChange(msStart, ms, axCngSign, arAbreakaway, arvtBound);
				else
					FindBoundChange(msStart, ms, arAbreakaway, arvtBound);

				if (arAbreakaway[0] > 0)		// set ms.s point as end point using 'before' condition
				{
					msEnd.s = ms.s;
					msEnd.Abreakaway = arAbreakaway[0];
					msEnd.vtAccBound = arvtBound[0];	// msEnd really only needs: s, Abreakaway.
				}
				else if (arAbreakaway[1] >= 0)
					break;				// Abreakaway steps through 0 at ms.s
				else						// set ms.s point as start point using 'after' condition
				{
					msStart.SetAccBoundMembers(ms);
					msStart.Abreakaway = arAbreakaway[1];
					msStart.vtAccBound = arvtBound[1];
				}
				endCorner.Restart();		// Restart search from Start/End limits after finding step
				endCorner.Point(msStart.s, msStart.Abreakaway);
				endCorner.Point(msEnd.s, msEnd.Abreakaway);

			}	// if (axChanged < 3)	if an axis bound changed
		}		//	if (seekRes & (CSeekValue::SR_HIGHORLOW))
		ms.s = endCorner.NextSGuess();
		GetPolysAtS(ms);
		GetCurveBreakaway(ms);	// sets dsdtMax
	}	// while (fabs(ms.Abreakaway) > m_Tol.acc)
				// breakaway point is at ms.s

	// store the new limit
	ASSERT(ms.s >= 0 && ms.s <= 1);
	if (ms.s == 1)			// ms.s can be 1, if so should start limit from s=0 next seg
		return;				// ignore for now, new limit will be set next segment
	if (ms.s < m_Limit.s || m_Limit.type == LT_NONE)	// store this one if earlier
	{
		ms.dsdt = ms.dsdtMax;
		GetMaxAccelAtVel(ms, 1, &m_Limit);
		m_Limit.type = LT_AXIS_ACCELFROM;	// done in GetMaxAccelAtVel() anyway
		m_Limit.change = LC_VEL_REDUCED;		// For velocity reduced
	}
}


///////////////////////////////

void CPathSpeed::FindAnormSignChange(const SMotionState& msStart, SMotionState& ms,
				int axChanged, double* arAbreakaway, TVector<char>* arvtBound)
{
	// find where bound changes (vtAnormDir[axChanged] == 0)
	//	GtSAnor0.m		// Get S where the component of Anormdir which changed sign is zero
	int ax = axChanged;				// just abbreviate
	CSeekValue boundChange(0);
	CVector vtAnormDirection;
	msStart.GetAnormDirection(vtAnormDirection);		// pre-req: GetPolysAtS()
	boundChange.Point(msStart.s, vtAnormDirection[ax]);
	while (1)
	{
		ms.GetAnormDirection(vtAnormDirection);
		if (fabs(vtAnormDirection[ax]) <= m_Tol.acc)
			break;
		boundChange.Point(ms.s, vtAnormDirection[ax]);
		ms.s = boundChange.NextSGuess();
		GetPolysAtS(ms);
	}

	// check if bound change steps Abreakaway through 0

	//	find vtBound at ms
	//	get Abreakaway with vtBound and vtBound[axChanged] negated
	// GetPolysAtS() has been done
	SMotionStateExt mse;
	ms.GetdCurvedAnorm(mse);	// Get Curve and Normal Acceleration vectors independent of Acc Limit

	ms.vtAnormDir[ax] = 0;		// set to exactly zero
	mse.vtAnormUnit[ax] = 0;	// set to exactly zero
	CBoxLimit bl;
	bl.vtA = ms.vtAnormDir;		// Acceleration normal direction
	bl.vtB = ms.vtVofS;
	bl.vtBoxMax = vtAccMax;
	bl.vtBoxMin = vtAccMin;
	bl.bMaxMbBound = false;			// use min Mb bound - will be the case just after changing bounds
	bl.SolveMaxMa();				// to maximise axis accelerations

	GetAbreakaway(ms, mse, bl);
	arvtBound[0] = arvtBound[1] = bl.vtBound;
	arAbreakaway[0] = arAbreakaway[1] = ms.Abreakaway;
	ASSERT(bl.vtBound[ax] != 0);		// this situation not handled yet, allowable??
	bl.vtBound[ax] = (char)-bl.vtBound[ax];
	bl.vtBoundValue[ax] = bl.vtBound[ax] > 0 ? bl.vtBoxMax[ax] : bl.vtBoxMin[ax];
	GetAbreakaway(ms, mse, bl);
	int BA = (bl.vtBound[ax] == msStart.vtAccBound[ax]) ? 0 : 1;	// Before or After index
	arvtBound[BA] = bl.vtBound;
	arAbreakaway[BA] = ms.Abreakaway;
}

void CPathSpeed::FindBoundChange(SMotionState& msStart, SMotionState& ms,
				double* arAbreakaway, TVector<char>* arvtBound)
{
	// bound change occurs when vtVofS for axis not changing goes through zero!...not calculated this way yet!
	// find the before and after edge bound axis
	TVector<char> vtmsInitAccBound;
	vtmsInitAccBound = ms.vtAccBound;	// get a copy for later reference
	int axBefore = -1, axAfter = -1;
	for (int ax = 0; ax < 3; ax++)
		if (msStart.vtAccBound[ax] == 0 && ms.vtAccBound[ax] != 0)
			axBefore = ax;
		else if (msStart.vtAccBound[ax] != 0 && ms.vtAccBound[ax] == 0)
			axAfter = ax;
	if (axBefore == -1 || axAfter == -1)
		LOGERROR("Bound didn't change on two axis");

	CSeekValue boundChange(0);
	CVector vtMa;
	double MaDiff;
	msStart.GetCurveAnorm();	// vtAnormDir is only required member - which is copied anyway!
	GetAccBound(msStart, &vtMa);
	MaDiff = vtMa[axAfter] - vtMa[axBefore];
	boundChange.Point(msStart.s, MaDiff);
	while (1)
	{
		ms.GetCurveAnorm();		// shouldn't really be needed first time!
		GetAccBound(ms, &vtMa);
		MaDiff = vtMa[axAfter] - vtMa[axBefore];
		if (fabs(MaDiff) <= m_Tol.def)
			break;
		boundChange.Point(ms.s, MaDiff);
		ms.s = boundChange.NextSGuess();
		GetPolysAtS(ms);
	}

	// check if bound change steps Abreakaway through 0

	//	find vtBound at ms
	//	get Abreakaway with vtBound before and after
	// GetPolysAtS() has been done
	SMotionStateExt mse;
	ms.GetdCurvedAnorm(mse);	// Get Curve and Normal Acceleration vectors independent of Acc Limit

	CBoxLimit bl;
	bl.vtA = ms.vtAnormDir;		// Acceleration normal direction
	bl.vtB = ms.vtVofS;
	bl.vtBoxMax = vtAccMax;
	bl.vtBoxMin = vtAccMin;
	bl.bMaxMbBound = false;			// use min Mb bound - will be the case just after changing bounds
	bl.SolveMaxMa();				// to maximise axis accelerations

	bl.vtBound[axBefore] = 0;		// set vtBound to 'Before' condition
	bl.vtBound[axAfter] = msStart.vtAccBound[axAfter];
	GetAbreakaway(ms, mse, bl);
	arvtBound[0] = bl.vtBound;		// before bound
	arAbreakaway[0] = ms.Abreakaway;

	bl.vtBound[axAfter] = 0;		// set vtBound to 'After' condition
	bl.vtBound[axBefore] = vtmsInitAccBound[axBefore];
	GetAbreakaway(ms, mse, bl);
	arvtBound[1] = bl.vtBound;		// after bound
	arAbreakaway[1] = ms.Abreakaway;
}




void CPathSpeed::CheckPolyKnotDirections()
{
//		ms0 is set from s=1 of previous segment, ms1 has s=0 & segment incremented
//		new segment polys have been loaded into m_Seg

	GetPolysAtS(ms1);
	CVector vtVel0Unit, vtVel1Unit;
	ms0.vtVofS.Unit(vtVel0Unit);		// get before and after unit velocity vectors
	ms1.vtVofS.Unit(vtVel1Unit);

	double VelDiffSq = (vtVel1Unit - vtVel0Unit).MagSq();
	if (VelDiffSq <= m_DirMatchAngSq)	// stop and restart if difference is above tolerance, ~1->3deg
		return;
	
	ms1.dsdt = 0;		// Stop and restart
	GetMaxAccelAtVel(ms1, 1, &m_Limit);		// sets up m_Limit
	m_Limit.seg = ms1.seg;
	m_Limit.type = LT_AXIS_ACCELFROM;
	m_Limit.change = LC_VEL_ZEROED;
	AddNewFowardLimit(m_Limit);
}



