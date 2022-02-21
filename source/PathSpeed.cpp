// PathSpeed.cpp: implementation of the CPathSpeed class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNCControl.h"

#include "PathSpeed.h"

#include "Settings.h"
#include "SeekValue.h"

#include <fstream.h>
#include <iomanip.h>

//#include "PathDoc.h"



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
/*			for (int i = 0; i < sizeArray; i++)
			{
				if (i % 20 == 0)
					os << ndStore.GetHeading();
				os << CurveNodeArray[i];
			}
*/		os.close();
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
	CPathTracker::SetToStart();

	m_ScanSection.start.seg = 0;
	m_ScanSection.start.s = 0;

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

	m_ScanSection.end = m_ScanSection.start;		// as FindSpeeds get start from current end
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
//	m_nStepChange = CNG_SEG | CNG_LIMIT;	done in FindAccelLimits() // Indicates a path polyminial or property has changed with no foward movement

	CVector& vtVelStart = m_ScanSection.start.ms.vtVel;
	ms1.dsdt = vtVelStart.Mag() / ms1.vtVofS.Mag();		// depending on initial velocity
	ms1.vtVel = ms1.vtVofS * ms1.dsdt;		// only used to check vel match
	ASSERT((ms1.vtVel - vtVelStart).MagSq() <= 1e-6);


	GetMaxAccelAtVel(ms1, DIR_ACCEL, &m_Limit);		// sets up limit
	m_Limit.seg = ms1.seg;
	m_Limit.change = LC_VEL_CONTINUOUS;
	m_Limit.tDuration = 0;				// so 0 if not calculated
	ASSERT(m_pLimitList != NULL);
	m_pLimitList->RemoveAll();
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
	// add one extra previous limit at least - once tried finding location before first limit in PathMove
	// add two extra!!
	if (idxLimit >= 2)
		m_FowardLimitList.Add(m_pLimitList->ElementAt(idxLimit-2));
	if (idxLimit >= 1)
		m_FowardLimitList.Add(m_pLimitList->ElementAt(idxLimit-1));
	m_FowardLimitList.Add(m_pLimitList->ElementAt(idxLimit));	// setup first limit for thi scan
	
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
	int nPreChange = 0;		// prechange flags
	if (ms1.s == 1)
		nPreChange |= CNG_SEG;
	// ms0 not valid at first so set a step change
	m_nStepChange = CNG_LIMIT;
	if (ms1.s == 0)
		m_nStepChange = CNG_SEG;

	// start conditions now set, work through path
	m_bAtEndOfPath = false;
	for (;;)
	{
		///////////////////////////////////
		// Check it at or past a change and set at earliest change loc
		// Check for drive properties change
		if (ms1.PathLoc() >= GetPathPropChangeLoc())	 // if at or passed a property change
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
		int iCount = 0;
		double sGoal = ms1.s;
		for (;;)
		{
			ASSERT(m_nVofSlaChangedSign == 0);
			for (;;)
			{
				GetVelAccAtS(ms1);	// calc ds/dt, d2s/dt2, d3s/dt3 using current drive limit
				// check calculations were OK
				if (!m_bVofSlaChangedSign)
					break;
				ASSERT(m_nStepChange == 0);
				ASSERT(ms1.seg == ms0.seg);
				ASSERT(m_nVofSlaChangedSign == 1);
				m_nVofSlaChangedSign = 0;
				ms1.s = 0.5 * (ms1.s + ms0.s);		// go half as far
			}

			CheckAccelLimits();
			if (m_Limit.type == LT_NONE)		// if no new limits incurred
				break;

			ms0.s = m_Limit.s;
			ms0.seg = m_Limit.seg;
			GetVelAccAtS(ms0);		// uses new limit, function does short-cuts when solving at limit start
							// if corner limit, GetVelAccAtS() calc's dsdtMax and Abreakaway
			if (m_Limit.type != LT_CORNER)
				GetdsdtMaxCorner(ms0);		// do for ms0 if not corner

			ms1.s = sGoal;
			ASSERT(++iCount < 20);
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
				do {
					GetNextPathProp();
				} while (ms1.PathLoc() >= GetPathPropChangeLoc());	 // if at or passed a property change
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
				ms1.seg++;
				ASSERT(ms1.seg == GetSegNumber());
				// set dsdt for ms1 across seg boundary
				GetPolysAtS(ms1);
				ASSERT(ms0.s == 1);		// will be!
				if (ms0.dsdt == 0)
					ms1.dsdt = 0;
				else
					ms1.dsdt = ms0.dsdt * sqrt(ms0.vtVofS.MagSq() / ms1.vtVofS.MagSq());		// less sqrt's
				m_nStepChange |= CNG_SEG;		// Indicates a path polyminial or property has changed with no foward movement
				CheckPolyKnotDirections();		// will restart at 0 speed if direction changes above tolerance
			}

			nPreChange = 0;		// reset all change flags
		}	// if (nPreChange)
	}	// for (;;)
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

	switch (limit.type)
	{
	case LT_AXIS_ACCELFROM:
		ASSERT((limit.acc * limit.vel > 0) || (fabs(limit.vel) <= m_Tol.vel));		// check for accel
		break;
	case LT_AXIS_VELOCITY:
		break;
	case LT_CORNER:
		break;
	default:
		ASSERT(0);
	}

	
	if (m_bCalcSegmentTimes)
		limit.tDuration =	m_ScanSection.time;	// not duration, start time of prev poly!

	// avoid limits starting at s=1
	ASSERT(limit.s != 1 || (limit.type == LT_AXIS_VELOCITY && limit.vel == 0));

	// avoid multiple limits in same location
	SDriveLimit& limitLast = m_FowardLimitList.GetLast();
	if (fabs(limit.Loc() - limitLast.Loc()) <= 1e-8)
	{
		limit.change |= limitLast.change;			// don't overwrite .change
		if (limit.s < 1e-8)						// check if very close to 0 or 1
			limit.s = 0;
		else if (limit.s > 1 - 1e-8)
			limit.s = 1;
		limitLast = limit;
	}
	else
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

	ASSERT(limit.type != LT_CORNER);
	switch (limit.type)
	{
	case LT_AXIS_BRAKEFROM:
		ASSERT((limit.acc * limit.vel < 0) || (fabs(limit.vel) <= m_Tol.vel));		// check for brake
		break;
	case LT_AXIS_ACCELFROM:
		ASSERT((limit.acc * limit.vel > 0) || (fabs(limit.vel) <= m_Tol.vel));		// check for accel
		break;
	case LT_AXIS_VELOCITY:
		break;
	default:
		ASSERT(0);
	}


	//ASSERT(limit.s != 1);			 avoid limits starting at s=1 (allow now!)

	// avoid multiple limits in same location
	//int idxLast = m_BackLimitList.GetSize() - 1;
	SDriveLimit& limitLast = m_BackLimitList.GetLast();
	if (fabs(limit.Loc() - limitLast.Loc()) <= 1e-8)
	{
		//if (limit.type == LT_AXIS_BRAKEFROM && limitLast.type == LT_AXIS_ACCELFROM)	// leave limitLast on list
		//	;
		//else
		//	;	// limitLast = limit;		// leave limitLast on list
	}
	else
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
	int nPreChange = CNG_INITIAL;		// prechange flags
	if (ms1.s == 0)
		nPreChange |= CNG_SEG;
	// ms0 not valid at first so set a step change
	m_nStepChange = CNG_LIMIT;
	if (ms1.s == 1)
		m_nStepChange = CNG_SEG;


	// work through path
	bool bEndOfPath = false;
	bool bEndOfLimits = false;
	for (;;)
	{
		///////////////////////////////////
		// Check it at or past a change and set at earliest change loc
		// Check if s is at or past a foward limit change
		if (ms1.PathLoc() <= m_FowardLimit.PathLoc())	// check for a limit change
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
		if (ms1.PathLoc() <= GetPathPropChangeLoc()) // at or passed a property change
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
		int iCount = 0;
		for (;;)
		{
			ASSERT(m_nVofSlaChangedSign == 0);
			for (;;)
			{
				if (m_CurrLimit.type == LT_AXIS_BRAKETO || m_CurrLimit.type == LT_AXIS_ACCELTO)
				{
					GetVelAtS(ms1, m_FowardLimit);
					ms1.dsdtMax = ms1.dsdt;				// dsdtMax for foward limit
					GetVelAccAtSNoPoly(ms1, m_CurrLimit);	// don't have to recalc polys
				}
				else
					GetVelAccAtS(ms1, m_CurrLimit);
				// check calculations were OK
				if (!m_bVofSlaChangedSign)
					break;
				ASSERT(m_nStepChange == 0);
				ASSERT(ms1.seg == ms0.seg);
				ASSERT(m_nVofSlaChangedSign >= 1 || m_nVofSlaChangedSign <= 2);
				m_nVofSlaChangedSign = 0;
				ms1.s = 0.5 * (ms1.s + ms0.s);		// go half as far
				LOGMESSAGE("Corrected VofSla sign");
			}
			
			CheckBrakeLimits();
			if (m_Limit.type == LT_NONE)		// if no new limits incurred
				break;

			// recalc ms0 using new limit
			ms0.s = m_Limit.s;
			ms0.seg = m_Limit.seg;		// probably not actully used, just for reference
			//if (ms1.PathLoc() == ms0.PathLoc())			// somethimes same but breaks next pass!
			//	LOGERROR("ms1 & ms0 locations same - check");
			if (m_CurrLimit.type == LT_AXIS_BRAKETO || m_CurrLimit.type == LT_AXIS_ACCELTO)
			{
				GetVelAtS(ms0, m_FowardLimit);
				ms0.dsdtMax = ms0.dsdt;				// dsdtMax for foward limit
				GetVelAccAtSNoPoly(ms0, m_CurrLimit);	// don't have to recalc polys
			}
			else
				GetVelAccAtS(ms0, m_CurrLimit);
			//	ms1.s remains the same (using new limit)

			ASSERT(++iCount < 20);
		}	// for (;;) - new limit was found


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
				do {
					GetPrevPathProp();
				} while (ms1.PathLoc() <= GetPathPropChangeLoc());	 // at or passed a property change
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
				ms1.seg--;
				ASSERT(ms1.seg == GetSegNumber());
				// set dsdt for ms1 across seg boundary
				GetPolysAtS(ms1);		// not done yet for ms1 if s==1 cause polys just changed
				ASSERT(ms0.s == 0);		// will be!
				if (ms0.dsdt == 0)
					ms1.dsdt = 0;
				else
					ms1.dsdt = ms0.dsdt * sqrt(ms0.vtVofS.MagSq() / ms1.vtVofS.MagSq());		// less sqrt's
				m_nStepChange |= CNG_SEG;		// Indicates a path polyminial or property has changed with no foward movement, ms0/1 is before/after step
			//	m_Limit.seg = ms1.seg;			// required here??
				// GetPolysAtS(ms1);
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
					//ASSERT(m_FowardLimit.s != 1);		// should start from s=0 of next segment instead - except final limit
				}
				else
				{
					m_FowardLimit.SetPreStart();		// will never reach this
					bEndOfLimits = true;
					break;
				}

				if (m_CurrLimit.type == LT_AXIS_BRAKETO)	//	if currently braking, check for a change in braking axis
				{
					GetMaxAccelAtVel(ms1, DIR_BRAKE, &m_Limit);	// gets max neg accel (brake)
					if (m_Limit.axis != m_CurrLimit.axis)	// braking axis changes
					{
						SetStartCurrBrakeLimitFrom(ms0);
						StoreCurrBrakeLimit();
					}
				}
				else if (m_CurrLimit.type == LT_AXIS_ACCELTO)	// do ???
				{

				}
				else			// not currently braking at all
				{
					if (iLastFwdChange & LC_VEL_REDUCED)	// or if prev limit is LT_CORNER
					{
						// set BrakeTo limit - can be end of corner limit, or step reduction at poly or drive properties change
						// ms1.dsdt has been set if seg changed!
						// make sure following is done after any drive prop change so new drive limits are used!
						GetMaxAccelAtVel(ms1, DIR_BRAKE, &m_Limit);	// sets max braking axis
						m_Limit.change = LC_VEL_CONTINUOUS;
						m_CurrLimit = m_Limit;
					}
					else
					{
						AddNewBackwardLimit(m_FowardLimit);	// use existing foward limit
					}
				}
			}	// if (nPreChange & CNG_LIMIT)
			else if (m_CurrLimit.type == LT_AXIS_BRAKETO)	// no limit change but currently braking, check for a change in braking axis
			{
				GetMaxAccelAtVel(ms1, DIR_BRAKE, &m_Limit);	// gets max neg accel (brake)
				if (m_Limit.axis != m_CurrLimit.axis)	// braking axis changes
				{
					SetStartCurrBrakeLimitFrom(ms0);
					StoreCurrBrakeLimit();
				}
			}

			nPreChange = 0;		// reset all change flags
		}	// if (nPreChange)
	}	// for (;;)
	// backward scan done now!

}


//////////////////////////////////////////

void CPathSpeed::CheckAccelLimits()
{
	m_Limit.type = LT_NONE;		// key for not set
	m_Limit.s = 2;					// key for not set
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
	if (ms1.dsdt != DSDT_ERRORVERYLARGE)		// if VofSla was very small with some velocity then accel's not correct!
		CheckDeacceleration();	// check if any axis over braking -> change braking axis
	if (m_CurrLimit.type == LT_AXIS_BRAKETO || m_CurrLimit.type == LT_AXIS_ACCELTO)
		CheckStartBraking();	// check if braked dsdt >= foward dsdt -> come off braketo limit and apply brake from == point

	//******** Store any new limits ***********
	if (m_Limit.type != LT_NONE)
		StoreCurrBrakeLimit();
}

void CPathSpeed::SetStartCurrBrakeLimitFrom(const SMotionState& ms)
{
	m_StartCurrBrakeLimit.change = LC_VEL_CONTINUOUS;
	m_StartCurrBrakeLimit.type = m_CurrLimit.type;
	m_StartCurrBrakeLimit.tDuration = m_CurrLimit.tDuration;
	m_StartCurrBrakeLimit.seg = ms.seg;
	m_StartCurrBrakeLimit.s = ms.s;
	m_StartCurrBrakeLimit.dsdt = ms.dsdt;
	m_StartCurrBrakeLimit.d2sdt2 = ms.d2sdt2;
	char ax = m_CurrLimit.axis;
	m_StartCurrBrakeLimit.axis = ax;
	m_StartCurrBrakeLimit.pos = ms.vtPos[ax];
	m_StartCurrBrakeLimit.vel = ms.vtVel[ax];
	m_StartCurrBrakeLimit.acc = m_CurrLimit.acc;
}

void CPathSpeed::StoreCurrBrakeLimit()
{
	if (m_CurrLimit.type == LT_AXIS_BRAKETO)	// replace current BrakeTo with BrakeFrom
		m_StartCurrBrakeLimit.type = LT_AXIS_BRAKEFROM;
	else if (m_CurrLimit.type == LT_AXIS_ACCELTO)	// replace current AccelTo with AccelFrom
		m_StartCurrBrakeLimit.type = LT_AXIS_ACCELFROM;
	else				// to fix case where != LT_AXIS_BRAKETO - 18/5/2004
	{
		ASSERT(m_CurrLimit.type == LT_AXIS_VELOCITY);	// brake can go over while on vel limit, check any other cases - maybe OK
		m_BackLimitList.RemoveAt(m_BackLimitList.GetSize() - 1);		// have to replace last one!
	}
	AddNewBackwardLimit(m_StartCurrBrakeLimit);

	if (m_Limit.type == LT_STARTBRAKE)	// if start brake add limit from foward limits, else add new (BrakeTo) limit
	{
		ASSERT(m_FowardLimit.type != LT_CORNER);	// should not get to LT_STARTBRAKE while still on LT_CORNER
		AddNewBackwardLimit(m_FowardLimit);		// make this latest one the current one
	}
	else	// generally another LT_AXIS_BRAKETO
	{
		if (m_Limit.type != LT_AXIS_BRAKETO && m_Limit.type != LT_AXIS_ACCELTO)		// don't add these, they will need to be removed anyway!
			AddNewBackwardLimit(m_Limit);
		else
			m_CurrLimit = m_Limit;		// just make this latest one the current one
	}
	ASSERT(m_CurrLimit.type != LT_CORNER);		// must not be an intermediate type
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
			double dsdtLimit = 0;
			double velLimit = 0;
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
					GetMaxAccelAtVel(ms1, DIR_ACCEL, &m_Limit);
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
		if (limVel.RangeCovered())
			for (;;)
			{
				ms.s = limVel.NextSGuess();
				GetVelAtS(ms);
				if (fabs(limVel.ValueError(ms.vtVel[ax])) <= m_Tol.vel)
					break;
				limVel.Point(ms.s, ms.vtVel[ax]);
			}
		else
		{
			if (limVel.RangeWithinTol())
			{
				ms.s = limVel.NextSGuess();
				GetVelAtS(ms);
			}
			else
				ASSERT(0);
		}

		ASSERT(ms.s >= 0 && ms.s <= 1);
		ASSERT(ms.s >= ms0.s && ms.s <= ms1.s);
		if (ms.s < m_Limit.s || m_Limit.type == LT_NONE)	// store this one if earlier
		{
			m_Limit = ms;
			m_Limit.axis = ax;
			m_Limit.pos = ms.vtPos[ax];		// not essential
			m_Limit.vel = limVel.Value();
			m_Limit.type = LT_AXIS_VELOCITY;
			m_Limit.acc = 0;						// not essential
			m_Limit.change = LC_VEL_CONTINUOUS;		// For velocity continuous
			// adjust ms1
			ms1.s = ms.s;
			GetVelAccAtS(ms1);
			vtOverVel = (ms1.vtVel > vtVelMaxTol) || (ms1.vtVel < vtVelMinTol);
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
				GetMaxAccelAtVel(ms1, DIR_ACCEL, &m_Limit);
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
				GetMaxAccelAtVel(ms1, DIR_ACCEL, &m_Limit);
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

		CSeekValue limAcc(ms1.vtAcc[ax] > 0 ? vtAccMax[ax] : vtAccMin[ax]);
		double tol = m_Tol.acc * (1 + fabs(limAcc.Value()));
		limAcc.Point(ms0.s, ms0.vtAcc[ax]);
		if (limAcc.Point(ms1.s, ms1.vtAcc[ax]))		// check seek val is within range
		{
			// add a third point to handle curving values - needed if value is correct at end but drops before going over!
			ms.s = (ms0.s + ms1.s) * 0.5;		// midway
			GetVelAccAtS(ms);
			for (;;)			// find s where axis accel reaches limit
			{
				limAcc.Point(ms.s, ms.vtAcc[ax]);
				ms.s = limAcc.NextSGuess();
				GetVelAccAtS(ms);
				if (fabs(limAcc.ValueError(ms.vtAcc[ax])) <= tol)
					break;
				if (limAcc.FoundBestS())
					break;
			}
			ASSERT(ms.s >= 0 && ms.s <= 1);
			ASSERT(ms.s >= ms0.s && ms.s <= ms1.s);
		}
		else		// accel passed through limit
		{
			ASSERT((ms0.vtAcc[ax] - limAcc.Value()) * limAcc.Value() > 0);	 // check if accel was above limit at ms0
			ASSERT(sign(ms1.vtVofS[ax]) != sign(ms0.vtVofS[ax]));		// check ms0.vtAcc was not over while braking and velocity changed sign!
			ASSERT((ms0.vtAcc[ax] * ms0.vtVel[ax] < 0) || (fabs(ms0.vtVel[ax]) <= m_Tol.vel));	// check for was braking at ms0
			// accel was already over at ms0 so find where vel == 0 and accel from there
//			continue;		// works normally if continue - else vel has to be reduced so accel not over
			ms = ms0;		// ms0 was braking - will make it seek zero vel
		}

		BYTE nLimitChange = LC_VEL_CONTINUOUS;
		// check if braking when accel reaches limit
		if ((limAcc.Value() * ms.vtVel[ax] < 0) && fabs(ms.vtVel[ax]) > m_Tol.vel)
		{
			// find when vel == 0 and accel from there
			CSeekValue seekVel(0);
			seekVel.Point(ms1.s, ms1.vtVofS[ax]);
			for (;;)
			{
				double val = ms.vtVofS[ax];
				if (fabs(seekVel.ValueError(val)) <= m_Tol.vel)
					break;
				seekVel.Point(ms.s, val);
				ms.s = seekVel.NextSGuess();
				GetPolysAtS(ms);
			}
			GetVelAccAtSNoPoly(ms);
			ASSERT((ms.vtAcc[ax] - limAcc.Value()) * limAcc.Value() > 0);	// check accel is above limit at zero crossing
			nLimitChange = LC_VEL_REDUCED;
			// find speed to reduce accel to limit value
			// as VofS is 0, Accel is only determined by A(s).(ds/dt)^2 so reduce ds/dt and recalc vel
			ms.dsdt *= sqrt(limAcc.Value() / ms.vtAcc[ax]);
			ms.vtVel = ms.vtVofS * ms.dsdt;
			ASSERT(ms.s >= 0 && ms.s <= 1);
			ASSERT(ms.s >= ms0.s && ms.s <= ms1.s);
		}
		
		if (ms.s < m_Limit.s || m_Limit.type == LT_NONE || m_Limit.type == LT_CORNER)	// store this one if earlier (change 12/7/04 - or overwrite a corner limit)
		{
			ASSERT(ms.s < m_Limit.s);
			m_Limit = ms;				// set s, dsdt, d2sdt2
			m_Limit.axis = ax;
			m_Limit.type = LT_AXIS_ACCELFROM;
			m_Limit.change = nLimitChange;
			m_Limit.pos = ms.vtPos[ax];
			m_Limit.vel = ms.vtVel[ax];
			m_Limit.acc = limAcc.Value();
			ASSERT((m_Limit.acc * m_Limit.vel > 0) || (fabs(m_Limit.vel) <= m_Tol.vel));	// check for accel
			// adjust ms1
			ms1 = ms;
			vtOverAcc = ((ms1.vtAcc > vtAccMaxTol) && ms1.vtVofS > vtZero)
						|| ((ms1.vtAcc < vtAccMinTol) && ms1.vtVofS < vtZero);	// Accelerating when Acc & Vel are same direction
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
			ASSERT(m_CurrLimit.type == LT_AXIS_BRAKETO
				 || m_CurrLimit.type == LT_AXIS_VELOCITY);	// should only occur when braking or on vel limit around corner
			if (m_Limit.type == LT_NONE)		// only set if no other limits applied
			{
				GetMaxAccelAtVel(ms1, DIR_BRAKE, &m_Limit);	// gets max neg accel (brake)
				SetStartCurrBrakeLimitFrom(ms1);
				// check limit not the same as m_CurrLimit!
				if (m_Limit.SameTypeAs(m_CurrLimit))
					LOGERROR("New limit same as current! - check");
					//m_Limit.type = LT_NONE;					// don't set limit if same!
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
				GetMaxAccelAtVel(ms1, DIR_BRAKE, &m_Limit);
				SetStartCurrBrakeLimitFrom(ms1);
			}
		}
		return;
	}

	if (!vtOverAcc.Any())
		return;
	ASSERT(m_CurrLimit.type == LT_AXIS_BRAKETO || m_CurrLimit.type == LT_AXIS_VELOCITY || m_CurrLimit.type == LT_AXIS_ACCELTO);	// should normally occur when braking, can occur on vel limit when still below max corner speed
	// Find s value of earliest Acc limit
	SMotionState ms;
	ms.seg = ms1.seg;
	for (char ax = 0; ax < 3; ax++)
	{
		if (vtOverAcc[ax] == false)
			continue;

		CSeekValue limAcc(ms1.vtAcc[ax] > 0 ? vtAccMax[ax] : vtAccMin[ax]);
		double tol = m_Tol.acc * (1 + fabs(limAcc.Value()));
		limAcc.Point(ms0.s, ms0.vtAcc[ax]);
		if (limAcc.Point(ms1.s, ms1.vtAcc[ax]))		// check range is covered
		{
			for (;;)			// find s where axis accel reaches limit
			{
				ms.s = limAcc.NextSGuess();
				GetVelAccAtS(ms);
				if (fabs(limAcc.ValueError(ms.vtAcc[ax])) <= tol)
					break;
				if (limAcc.FoundBestS() && fabs(limAcc.ValueError(ms.vtAcc[ax])) <= tol*1e4)
					break;
				limAcc.Point(ms.s, ms.vtAcc[ax]);
			}
			ASSERT(ms.s >= 0 && ms.s <= 1);
		}
		else
			ASSERT(0);		// range should be covered!!


		// check if accelerating when accel reaches limit
		if ((limAcc.Value() * ms.vtVel[ax] > 0) && fabs(ms.vtVel[ax]) > m_Tol.vel)
		{
			// previously set m_Limit.type = LT_AXIS_ACCELTO;		// vel may change sign, not normally accel, possible between samples
			// find when vel == 0 and brake to there
			CSeekValue seekVel(0);
			seekVel.Point(ms1.s, ms1.vtVofS[ax]);
			for (;;)
			{
				double val = ms.vtVofS[ax];
				if (fabs(seekVel.ValueError(val)) <= m_Tol.vel)
					break;
				seekVel.Point(ms.s, val);
				ms.s = seekVel.NextSGuess();
				GetPolysAtS(ms);
			}
			ASSERT(ms.s >= 0 && ms.s <= 1);
			GetVelAccAtSNoPoly(ms);
			//ASSERT((ms.vtAcc[ax] - limAcc.Value()) * limAcc.Value() > 0);	// check accel is above limit at zero crossing
			ASSERT((limAcc.Value()<0 ? ms.vtAcc[ax]-limAcc.Value() : limAcc.Value()-ms.vtAcc[ax]) < m_Tol.vsmall);	// check accel is above limit at zero crossing
		}

		ASSERT(ms.s >= ms1.s && ms.s <= ms0.s);
		if (ms.s > m_Limit.s || m_Limit.type == LT_NONE)	// store this one if later (for Braking)
		{
			m_Limit = ms;				// set s, dsdt, d2sdt2
			m_Limit.axis = ax;
			m_Limit.type = LT_AXIS_BRAKETO;
			m_Limit.pos = ms.vtPos[ax];
			m_Limit.vel = ms.vtVel[ax];
			m_Limit.acc = limAcc.Value();
			SetStartCurrBrakeLimitFrom(ms);
		}
	}	//	for (int ax = 0; ax < 3; ax++)
}

void CPathSpeed::CheckStartBraking()
{
// check if braked dsdt > foward dsdt -> come off braketo limit and apply brake from == point and resume foward limit
	ASSERT(m_CurrLimit.type == LT_AXIS_BRAKETO || m_CurrLimit.type == LT_AXIS_ACCELTO);
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
			SetStartCurrBrakeLimitFrom(ms1);
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
	for (;;)
	{
		matchSpeedLoc.Point(ms.s, dsdtOver);
		ms.s = matchSpeedLoc.NextSGuess();
		GetVelAtS(ms, m_FowardLimit);	// for dsdt
		ms.dsdtMax = ms.dsdt;			// set dsdtMax from foward (accel) speed
		GetVelAtSNoPoly(ms, m_CurrLimit);	// don't have to recalc polys
		dsdtOver = ms.dsdt - ms.dsdtMax;
		double dsdtOverAbs = fabs(dsdtOver);
		double dsdtTol = max(ms.dsdt, ms.dsdtMax) * m_Tol.dsdt;		// dsdt is always >= 0
		if (dsdtOverAbs <= dsdtTol)
			break;
		if (matchSpeedLoc.NumPoints() > 10 && dsdtOverAbs <= dsdtTol * 100)
			break;
	}
	GetVelAccAtSNoPoly(ms, m_CurrLimit);	// get Acc, d2sdt2 if required?

	ASSERT(ms.s >= 0 && ms.s <= 1);
	if (ms.s > m_Limit.s || m_Limit.type == LT_NONE)	// store this one if later (for Braking)
	{
		m_Limit = ms;				// set s, dsdt, d2sdt2 - use braked ds/dt not foward, matching pos & vel
		m_Limit.type = LT_STARTBRAKE;		// will add the current foward limit
		SetStartCurrBrakeLimitFrom(ms);
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
			GetMaxAccelAtVel(ms1, DIR_ACCEL, &m_Limit);		// sets m_Limit - uses current members of ms1
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
	double dsdtOver;
	dsdtOver = GetdsdtOver(ms0);	// ms0.dsdtMax will have been calc for all limit types!
	overSpeedLoc.Point(ms0.s, dsdtOver);
	dsdtOver = GetdsdtOver(ms1);
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
		dsdtOver = GetdsdtOver(ms);
	} while (fabs(dsdtOver) > m_Tol.dsdt);		// changed from m_Tol.vel 18/3
	ms.dsdt = ms.dsdtMax;	// doesn't really matter as  it's within m_Tol.dsdt

	ASSERT(ms.s >= 0 && ms.s <= 1);
	ASSERT(ms.s >= ms0.s && ms.s <= ms1.s);
	if (ms.s < m_Limit.s || m_Limit.type == LT_NONE)	// store this one if earlier
	{
		m_Limit = ms;						// set s, dsdt, d2sdt2
		m_Limit.axis = -1;				// n/a or axis which has to over-brake
		m_Limit.type = LT_CORNER;		// For Corner Velocity limit
		m_Limit.acc = 0;					// n/a
		m_Limit.change = LC_VEL_REDUCED;		// velocity reduced for corner
		// adjust ms1
		ms1.s = ms.s;
		GetVelAccAtS(ms1);
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
			GetMaxAccelAtVel(ms1, DIR_ACCEL, &m_Limit);
			m_Limit.change = LC_VEL_REDUCED;		// For velocity reduced
		}
		return;
	}

	if (ms1.Abreakaway <= 0)		// changed from < to <= 5/03, an acceleration limit would over-speed corner
		return;
/*
	Find point where axis acceleration can be resumed without corner accel going too high
	ie. where Abreakaway changes from neg to pos through zero
	Check for any step changes in Abreakaway from ms0 to ms1, find earliest point with Abreakaway >=0
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
	ms.SetAccBoundMembers(ms1);			// sets dsdtMax - only needed if search stops straight away
	if (ms0.Abreakaway > 0)
	{
		ms.SetAccBoundMembers(ms0);		// or to ms1?
		LOGERROR("Check why on corner limit");
		ASSERT(0);
		ms.Abreakaway = 0;		// set to break out of following loop
	}
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
/*
				// First check if any bound axis has changed sign (otherwise change is 0 <-> +/-1)
				for (int axCngSign = 0; axCngSign < 3; axCngSign++)	// Check if a box bound changes sign (determined by sign(vtAnormDir)
					if (msStart.vtAccBound[axCngSign] * ms.vtAccBound[axCngSign] == -1)
						break;
				if (axCngSign < 3)	// Bound sign changed, find when vtAnormDir[axCngSign] == 0 (or vtBperp which is same direction)
					FindBoundSignChange(msStart, ms, axCngSign, arAbreakaway, arvtBound);
				else
					FindBoundAxisChange(msStart, ms, arAbreakaway, arvtBound);
*/
// above lines changed 12/7/04
				// check for an axis change first, then sign change
				int axCngAxis = -1;
				int axCngSign = -1;
				for (int ax = 0; ax < 3; ax++)	// Check if a box bound changes axis pair 0 <-> +/-1
				{
					if (((msStart.vtAccBound[ax] == 0) ^ (ms.vtAccBound[ax] == 0)) == 1)
					{
						axCngAxis = ax;
						break;
					}
					if (msStart.vtAccBound[ax] * ms.vtAccBound[ax] == -1)
						if (axCngSign < 0)
							axCngSign = ax;		// get first sign change if more than one
				}
				if (axCngAxis >= 0)
					FindBoundAxisChange(msStart, ms, arAbreakaway, arvtBound);		// sets ms.dsdtMax
				else if (axCngSign >= 0)
					FindBoundSignChange(msStart, ms, axCngSign, arAbreakaway, arvtBound);	// sets ms.dsdtMax
				else
				{
					ASSERT(0);		// should have been one of above cases!
					return;
				}
////// end of change

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
				// recheck at msEnd;
				ms.SetAccBoundMembers(msEnd);
				continue;				// skip endCorner.NextSGuess()

			}	// if (axChanged < 3)	if an axis bound changed
		}		//	if (seekRes & (CSeekValue::SR_HIGHORLOW))
		ms.s = endCorner.NextSGuess();
		GetPolysAtS(ms);
		GetCurveBreakaway(ms);	// sets dsdtMax
	}	// while (fabs(ms.Abreakaway) > m_Tol.acc)
				// breakaway point is at ms.s

	// store the new limit
	ASSERT(ms.s >= 0 && ms.s <= 1);
	ASSERT(ms.s >= ms0.s && ms.s <= ms1.s);
	if (ms.s == 1)			// ms.s can be 1, if so should start limit from s=0 next seg
		return;				// ignore for now, new limit will be set next segment
	if (ms.s < m_Limit.s || m_Limit.type == LT_NONE)	// store this one if earlier
	{
		ms.dsdt = ms.dsdtMax;		// dsdtMax is set before loop breaks
		GetMaxAccelAtVel(ms, DIR_ACCEL, &m_Limit);
		m_Limit.change = LC_VEL_REDUCED;		// For velocity reduced
		// adjust ms1
		ms1.s = ms.s;
		GetVelAccAtS(ms1);
	}
}


///////////////////////////////
// changed to: FindBoundSignChange(...)
/*
void CPathSpeed::FindAnormSignChangeOld(const SMotionState& msStart, SMotionState& ms,
				int axChanged, double* arAbreakaway, TVector<char>* arvtBound)
{
	// find where bound changes (vtAnormDir[axChanged] == 0)
	// find where bound changes (vtVofS[not axChanged, other] == 0)
	//	GtSAnor0.m		// Get S where the component of Anormdir which changed sign is zero
	int ax = axChanged;				// just abbreviate
	CSeekValue boundChange(0);
	CVector vtAnormDirection;
	msStart.GetAnormDirection(vtAnormDirection);		// pre-req: GetPolysAtS()
	boundChange.Point(msStart.s, vtAnormDirection[ax]);
	for (;;)
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
	GetAbreakaway(ms, mse, bl);
	int BA = (bl.vtBound[ax] == msStart.vtAccBound[ax]) ? 0 : 1;	// Before or After index
	arvtBound[BA] = bl.vtBound;
	arAbreakaway[BA] = ms.Abreakaway;
}
*/

void CPathSpeed::FindBoundSignChange(const SMotionState& msStart, SMotionState& ms,
				int axChanged, double* arAbreakaway, TVector<char>* arvtBound)
{
	// find where bound changes (vtVofS[axBound not changed] == 0)
	// get axis of bound edge
	int axEdge, axChange, axSame;
	axEdge = axChange = axSame = -10;
	int nChange = 0;
	for (int ax = 0; ax < 3; ax++)
	{
		if (msStart.vtAccBound[ax] == 0 && ms.vtAccBound[ax] == 0)		// axis both 0
			axEdge = ax;
		else if (msStart.vtAccBound[ax] * ms.vtAccBound[ax] == -1)		// axis changes sign
			{ axChange = ax; nChange++; }
		else if (msStart.vtAccBound[ax] * ms.vtAccBound[ax] == 1)		// axis remains same
			axSame = ax;
		else			// just one is 0!
			ASSERT(0);
	}
	// must have one axis remain 0, one axis remain +/-1, one axis change between +/-1
	// or maybe both non zero axis change sign, will assert - check situation
	ax = -1;		// set ax to axis whose bound stay the same - VofS == 0 for this axis
	ASSERT(axChanged >= 0 && axEdge >= 0);
	if (nChange == 1)
	{
		ASSERT(axSame >= 0);
		ASSERT(axEdge + axChange + axSame == 3);		// can only be if all were set just once!
		ASSERT(axChanged == axChange);
		ax = axSame;
	}
	else if (nChange == 2)
	{
		ASSERT(axChanged != axEdge);
		ASSERT(axSame < 0);			// wasn't set
		ax = (axEdge+1) % 3;
		if (ax == axChanged)
			ax = (axEdge+2) % 3;		// now ax is axis for which VofS changes sign!
		axSame = ax;
	}
	else
		ASSERT(0);		// no axis sign changes!

	ASSERT(axEdge >= 0);		// before and after must be on same edge for this function!
	ASSERT(ax >= 0);

	CSeekValue seekVel(0);
	seekVel.Point(msStart.s, msStart.vtVofS[ax]);
	for (;;)
	{
		double val = ms.vtVofS[ax];
		if (fabs(val) <= m_Tol.vel)
			break;
		seekVel.Point(ms.s, val);
		ms.s = seekVel.NextSGuess();
		GetPolysAtS(ms);
	}

	// check if bound change steps Abreakaway through 0

	//	find vtBound at ms
	//	get Abreakaway with vtBound and vtBound[axChanged] negated
	// GetPolysAtS() has been done
	SMotionStateExt mse;
	ms.vtVofS[ax] = 0;			// set to exactly zero
	ms.GetdCurvedAnorm(mse);	// Get Curve and Normal Acceleration vectors independent of Acc Limit

	CBoxLimit bl;				//Only BoxLimit .vtBound, .vtBoxMax & .vtBoxMin required!
	bl.vtA = ms.vtAnormDir;		// Acceleration normal direction
	bl.vtB = ms.vtVofS;
	bl.vtBoxMax = vtAccMax;
	bl.vtBoxMin = vtAccMin;
	bl.bMaxMbBound = false;			// use min Mb bound - will be the case just after changing bounds
	bl.SolveMaxMa();					// to maximise axis accelerations

	if (bl.vtBound[axEdge] != 0)
	{
		bl.bMaxMbBound = true;		// just a fix - 10/5/05
		bl.SolveMaxMa();
	}
	ASSERT(bl.vtBound[axEdge] == 0);
	ASSERT(bl.vtBound[axSame] == msStart.vtAccBound[axSame]);
	// set before bound
	bl.vtBound[axChanged] = msStart.vtAccBound[axChanged];
//	bl.vtBound = msStart.vtAccBound;
	GetAbreakaway(ms, mse, bl);
	arvtBound[0] = bl.vtBound;
	arAbreakaway[0] = ms.Abreakaway;
	// set after bound
	bl.vtBound[axChanged] = (char)-bl.vtBound[axChanged];
	GetAbreakaway(ms, mse, bl);		// sets ms.dsdtMax
	arvtBound[1] = bl.vtBound;
	arAbreakaway[1] = ms.Abreakaway;
}

void CPathSpeed::FindBoundAxisChange(SMotionState& msStart, SMotionState& ms,
				double* arAbreakaway, TVector<char>* arvtBound)
{
	// bound change occurs NOT when vtVofS for axis not changing goes through zero - can't use VofS method!
	// find the before and after edge bound axis
	// if there are multiple BoundAxis changes or BoundSign change as well, just one BoundAxis change is returned
	TVector<char> vtmsInitAccBound;
	vtmsInitAccBound = ms.vtAccBound;	// get a copy for later reference
	int axBefore, axAfter, axSame, axChangeSign;
	axBefore = axAfter = axSame = axChangeSign = -1;
	for (int ax = 0; ax < 3; ax++)
		if (msStart.vtAccBound[ax] == 0 && ms.vtAccBound[ax] != 0)
			axBefore = ax;
		else if (msStart.vtAccBound[ax] != 0 && ms.vtAccBound[ax] == 0)
			axAfter = ax;
		else if (msStart.vtAccBound[ax] * ms.vtAccBound[ax] == 1)
			axSame = ax;
		else if (msStart.vtAccBound[ax] * ms.vtAccBound[ax] == -1)
			axChangeSign = ax;
		else
			ASSERT(0);

	if (axBefore == -1 || axAfter == -1)
	{
		LOGERROR("Bound didn't change on two axes");
		ASSERT(0);
	}

	if (axSame >= 0)
		ASSERT(axChangeSign < 0);
	else
		ASSERT(axChangeSign >= 0);


// origional version
	// this origional version found when MaDiff = bl.arMa[axAfter] - bl.arMa[axBefore] == 0
	CSeekValue boundChange(0);
	double MaDiff;
	CBoxLimit bl;
	double* arMa = bl.arMa;
	msStart.GetCurveAnorm();	// vtAnormDir is only required member - which is copied anyway!
	GetAccBound(msStart, bl);
	MaDiff = arMa[axAfter] - arMa[axBefore];
	boundChange.Point(msStart.s, MaDiff);
	for (;;)
	{
		ms.GetCurveAnorm();		// shouldn't really be needed first time!
		GetAccBound(ms, bl);
		MaDiff = arMa[axAfter] - arMa[axBefore];
		if (fabs(MaDiff) <= m_Tol.def)
			break;
		boundChange.Point(ms.s, MaDiff);
		ms.s = boundChange.NextSGuess();
		GetPolysAtS(ms);
	}
// end origional version


/*
// new edgeVal version
	// find when edgeVal for axBefore or axAfter goes through bound value
	// find when edgeVal for axBefore goes through bound value from axAfter

	ASSERT(ms.vtAccBound[axBefore] != 0);			// already checked!
	double boundValBefore = ms.vtAccBound[axBefore] > 0 ? vtAccMax[axBefore] : vtAccMin[axBefore];

	CSeekValue boundChange(boundValBefore);
	CBoxLimit bl;
	double& edgeValBefore = bl.arEdgeVal[axBefore];
	msStart.GetCurveAnorm();	// vtAnormDir is only required member - which is copied anyway!
	GetAccBound(msStart, bl);
	boundChange.Point(msStart.s, edgeValBefore);
 	for (;;)
	{
		ms.GetCurveAnorm();		// shouldn't really be needed first time!
		GetAccBound(ms, bl);
		if (fabs(boundChange.ValueError(edgeValBefore)) <= m_Tol.def)
			break;
		boundChange.Point(ms.s, edgeValBefore);
		ms.s = boundChange.NextSGuess();
		GetPolysAtS(ms);
	}
// end new edgeVal version
*/



	// check if bound change steps Abreakaway through 0

	//	find vtBound at ms
	//	get Abreakaway with vtBound before and after
	// GetPolysAtS() has been done
	SMotionStateExt mse;
	ms.GetdCurvedAnorm(mse);	// Get Curve and Normal Acceleration vectors independent of Acc Limit

// changed 13/7/04
/* was:
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
*/
// to:
	//  bl.arvtBound[axBefore] & bl.arvtBound[axAfter] must be different - will be as axis value is 0!
	// Only BoxLimit .vtBound, .vtBoxMax & .vtBoxMin required!
	// set before bound
	bl.vtBound = bl.arvtBound[axBefore];	// set vtBound to 'Before' condition
	GetAbreakaway(ms, mse, bl);
	arvtBound[0] = bl.vtBound;				// before bound (msStart)
	arAbreakaway[0] = ms.Abreakaway;

	// set after bound
	bl.vtBound = bl.arvtBound[axAfter];
	GetAbreakaway(ms, mse, bl);			// sets ms.dsdtMax
	arvtBound[1] = bl.vtBound;				// after bound (ms)
	arAbreakaway[1] = ms.Abreakaway;
// end change 13/7/04
}




void CPathSpeed::CheckPolyKnotDirections()
{
//		ms0 is set from s=1 of previous segment, ms1 has s=0 & segment incremented
//		polys for ms0 & ms1 have been found

	CVector vtVel0Unit, vtVel1Unit;
//	ms0.vtVofS.Unit(vtVel0Unit);		// get before and after unit velocity vectors
//	ms1.vtVofS.Unit(vtVel1Unit);

	vtVel0Unit = ms0.vtVofS;
	vtVel1Unit = ms1.vtVofS;
	double magVel0 = vtVel0Unit.Mag();
	double magVel1 = vtVel1Unit.Mag();
	if (magVel0 > 0 && magVel1 > 0)		// if one is 0 mag then stop and restart
	{
		vtVel0Unit *= 1/magVel0;
		vtVel1Unit *= 1/magVel1;
		double VelDiffSq = (vtVel1Unit - vtVel0Unit).MagSq();
		if (VelDiffSq <= m_DirMatchAngSq)	// stop and restart if difference is above tolerance, ~1->3deg
			return;
	}

	ms1.dsdt = 0;		// Stop and restart
	GetMaxAccelAtVel(ms1, DIR_ACCEL, &m_Limit);		// sets up m_Limit
	m_Limit.seg = ms1.seg;
	m_Limit.type = LT_AXIS_ACCELFROM;
	m_Limit.change = LC_VEL_ZEROED;
	AddNewFowardLimit(m_Limit);
}



