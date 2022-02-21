// PathMove.cpp: implementation of the CPathMove class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"
#include "Settings.h"			// for g_Settings
#include "PathMove.h"

//#include "..\Graph\Plot.h"		// only testing


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPathMove::CPathMove()
{
}

CPathMove::~CPathMove()
{
}

void CPathMove::Init()
{
	CPathTracker::Init();		// call base
	SetDefaults();
}

void CPathMove::SetDefaults()
{
// for t at discrete steps
	SetDiscreteTStep(g_Settings.MachParam.dTime);		// 1e-3   smallest discrete time step
	m_dTSegment = g_Settings.MachPathSeg.SegmentTime;		// ~100e-3   segment time

// for s stepping
	m_dS = 0.02;

}

void CPathMove::SetToStart()
{
	CPathTracker::SetToStart();

	ASSERT(m_pPathDoc != NULL);	// must have been set by now!
	ASSERT(m_pLimitList != NULL);	// must be set to limit list

	SetPathMoveFlags(0);
	SetToStartOfPath();
	FindCurrentLimit();
	GetVelAccAtS(ms1);				// set initial state in case it's used!
}

void CPathMove::GetSpeedsAtS(CMotionStateBasicArray& motArray)
{
// scans through path calculating speeds from start seg to end seg
	
	// calculate approx number of s locations to store - assume each limit change requires 2 extra points
	// = NumSegs * (1/SInc + 1) + 2 * NumLimits + 2 * NumPropChanges
	int iNumLimits = m_pLimitList->GetSize();	
	int iNumSegs = m_pLimitList->ElementAt(iNumLimits-1).seg - m_pLimitList->ElementAt(0).seg;
	iNumSegs = abs(iNumSegs) + 1;
//	int iNumPoints = GetNumPathSegments() * (int)(1 / m_dS + 1.5);
	int iNumPoints = iNumSegs * (int)(1 / m_dS + 1.5);
	iNumPoints += 2 * iNumLimits;
	motArray.SetSize(iNumPoints);
	int iPtIdx = 0;

	for (;;)
	{
		// if a change, store prechange states (in msPre.acc)
		if (GotPreChangeState())
			motArray[iPtIdx++].StoreMS(msPre, -1);		// Store msPre states

		// store states (or postchange states) (in ms1)
		motArray[iPtIdx++].StoreMS(ms1);		// Store ms1 states
			
		ASSERT(iPtIdx < iNumPoints);

		if (AtEndOfPath())
			break;
		AdvanceSStep();
		if (ms1.seg > m_EndSeg)
			break;
		if (AtEndOfCalcLimits())		// usual exit point
			break;
	}
	motArray.SetSize(iPtIdx);		// trucate any extra array memory
}


void CPathMove::GetSpeedsAtTime(CMotionStateBasicArray& motArray)
{
	// calculate number of s locations to store - assume each limit change requires 2 extra points
	// = NumSegs * (1/SInc + 1) + 2 * NumLimits + 2 * NumPropChanges
	int iNumPoints = int(120 / m_dTSegment);		// need to get at path time estimate
	iNumPoints += 2 * m_pLimitList->GetSize();

	if (iNumPoints < motArray.GetSize())
		iNumPoints = motArray.GetSize();			// tempory measure!
	motArray.SetSize(iNumPoints);					// size not correct
	int iPtIdx = 0;

	for (;;)
	{
		// if a change, store prechange states
		if (GotChangeState())
		{
			if (GotPreChangeState() == CHANGE_IS_BEFORE_STEP)
				motArray[iPtIdx++].StoreMS(msPre, -1);		// Store msPre states
			if (GotPostChangeState() == CHANGE_IS_BEFORE_STEP)
				motArray[iPtIdx++].StoreMS(msPost, 1);		// Store msPost states
			
			motArray[iPtIdx++].StoreMS(ms1);			// Store ms1 states

			if (GotPreChangeState() == CHANGE_IS_AFTER_STEP)
				motArray[iPtIdx++].StoreMS(msPre, -1);		// Store msPre states
			if (GotPostChangeState() == CHANGE_IS_AFTER_STEP)
				motArray[iPtIdx++].StoreMS(msPost, 1);		// Store msPost states
		}
		else
			motArray[iPtIdx++].StoreMS(ms1);			// Store ms1 states

		ASSERT(iPtIdx < iNumPoints);
		if (iPtIdx >= iNumPoints - 10)
			motArray.SetSize(iNumPoints += 2000);		// increase size

		if (AtEndOfPath())
			break;
		AdvanceBydT(m_dTSegment);
		if (!AtEndOfPath())
		{
			if (ms1.seg > m_EndSeg)
				break;
			if (AtEndOfCalcLimits())		// usual exit point
				break;
		}
		// if change occured, gives state at discrete time nearest change
		// ms1 has state at time (post change if discrete time is exactly change time)
		// msPre & msPost have relative states of stepping values (acc)
		// if a change, store prechange states (in ms0.acc?)
	}
	motArray.SetSize(iPtIdx);		// trucate any extra
}



///////////////////////////////////////////////////
// Path moving functions
///////////////////////////////////////////////////

void CPathMove::LimitListChanged()		// used to notify of new limit list
{
	FindCurrentLimit();	// find current and next limits
	// solve state only really required on initial time as ms1 will have been calc'ed on prev limit list
}

void CPathMove::FindCurrentLimit()
{
	// limit list has been renewed so find current limit for ms1 in list - from start
	// if m_CurrLimit changes, m_TLimit should also change accordingly!!!
	m_bAtEndOfLimits = false;
	m_bNextLimitIsEndCalc = false;

	m_posLL = -1;
	if (m_pLimitList != NULL)
		m_posLL = m_pLimitList->GetMinPosition();
	if (m_posLL < 0)		// list empty
	{
		m_CurrLimit.SetPostEnd();		// end of limit list
		m_NextLimit.SetPostEnd();		// end of limit list
		m_bAtEndOfLimits = true;
		return;
	}
	for (;;)
	{
		if (m_pLimitList->GetNext(m_posLL) > ms1)
		{
			m_pLimitList->GetPrev(m_posLL);
//			if (m_TLimit >= 0)		// sometimes limit was changed but position is just before limit start - so use limit just after ms1 position
			// do 'GetPrev(m_posLL)' even if m_TLimit < 0 as will become positive with next advance
				m_pLimitList->GetPrev(m_posLL);
			ASSERT(m_posLL >= 0);		// if -1 then no limits <= ms1
			m_CurrLimit = m_pLimitList->GetNext(m_posLL);
			m_NextLimit = m_pLimitList->GetNext(m_posLL);
			break;
		}
		if (m_posLL < 0)		// got to end of list
		{
			m_posLL = m_pLimitList->GetMaxPosition();
			m_CurrLimit = m_pLimitList->GetNext(m_posLL);
			m_NextLimit.SetPostEnd();		// end of limit list
			m_bAtEndOfLimits = true;
			if (m_CurrLimit.type == LT_ENDCALC)			// at end of calc'd limits, list will need updating before use!
				return;
			break;
		}
	}
	m_bNextLimitIsEndCalc = (m_NextLimit.type == LT_ENDCALC);
	GetTimeNextChange();
}


void CPathMove::SetToStartOfPath()	// can be called before limit list set
{
// set appropriate start conditions
	ASSERT(m_pPathDoc != NULL);	// must have been set by now!
	ASSERT(m_pLimitList != NULL);	// must be set to limit list

	// reset trackers
	m_TStartLimit = 0;	// time of start of current limit from start of path
	m_TLimit = 0;			// time on current limit

	m_bAtEndOfLimits = false;
	m_bNextLimitIsEndCalc = false;

	m_nChangeLoc = NO_CHANGE;
	m_nPreChangeLoc = NO_CHANGE;
	m_nPostChangeLoc = NO_CHANGE;

	// for discrete s stepping only
	m_iSSteps = 0;

	// for discrete dt steps only
	m_iTStepPath = 0;
	m_idTSteps = 0;

	// set first poly
	if (!GetSegPolysAt(ms1.seg))
	{
		ASSERT(0);
		return;			// no polys!
	}

	// current limit must still be found, but list may not be set yet!
//	GetPolysAtS(ms1);	// done in SetToStart() after limit in set
}

bool CPathMove::GotChangeState()
{
	return (m_nChangeLoc != NO_CHANGE);
}

int CPathMove::GotPreChangeState()
{
	if (m_bGetPreChangeValues && (m_nPreChangeLoc != NO_CHANGE))
		return m_nPreChangeLoc;
	return 0;
}

int CPathMove::GotPostChangeState()
{
	if (m_bGetPostChangeValues && (m_nPostChangeLoc != NO_CHANGE))
		return m_nPostChangeLoc;
	return 0;
}




// setup functions
void CPathMove::SetPathMoveFlags(int /*flags*/)
{
	m_nStopAt = CNG_SEG | CNG_LIMIT | CNG_DRIVEPROPS;		// flags

	m_bGetPreChangeValues = true;
	m_bGetPostChangeValues = true;

}

void CPathMove::SetDiscreteTStep(double dT)
{
	m_dT = dT;
	m_dTOn2 = 0.5 * dT;
	m_dTInv = 1.0 / dT;
}


// moving functions
void CPathMove::AdvanceSStep()
{
	m_nAdvanceType = ADVANCE_S;

	if (ms1.s == 0 || ms1.s == 1)
		m_iSSteps = 1;				// reset to 0 and inc
	else if (ms1.s == m_dS * m_iSSteps)
		m_iSSteps++;
	ms1.s = m_dS * m_iSSteps;
	if (ms1.s >= (1 - 0.5 * m_dS))	// if comes to within 50% of increment from 1, move to 1 - incase of round off error
		ms1.s = 1;


	while (ms1.s > 1)			// only if past end (not at end)
	{
		ms1.s -= 1;
		ms1.seg++;
	}
	AdvanceToSPoint();
}


void CPathMove::AdvanceBydT(double dt)
{
/*
while (1)
{
	find which -if any- change occurs next (poly, limit or drvprops) 
	if (no change) break
	if (stopping at change) find nearest discrete point to stop at, break
	do next change
}
  got to point to get state
*/
	//	ensure dt is an intergral number of steps
	m_idTSteps = NEARINT(dt * m_dTInv);
	dt = m_dT * m_idTSteps;

	m_nAdvanceType = ADVANCE_TIME;
	const double tLimitPrev = m_TLimit;		// time is at a discrete time step
	m_TLimitsChanged = 0;				// keeps track of changes for backtracking
	CPathLoc locPrev = ms1;				// only required if end calculated limit encountered
	m_TLimit += dt;						// m_TLimit is always at a discrete time step

	m_nChangeLoc = m_nPreChangeLoc = m_nPostChangeLoc = NO_CHANGE;
	for (;;)
	{
		// set to next attempted point, will be brought back to a change if necessary
		m_nChangeType = 0;			// flags
		if (m_bAtEndOfPath || m_bAtEndOfLimits)
			break;
		double tLimitChange = m_TLimit + m_dTOn2;		// check ahead at least 1/2 step so next advance will be >= 1
		ASSERT(m_nNextChangeType != 0);
		//////////////////////////////////////////
		// check if at or past next change. Check poly first to set end of seg state (seg & s=1) if limit changes too
		if (tLimitChange >= m_TNextChange)
		{
			m_nChangeType = m_nNextChangeType;
			tLimitChange = m_TNextChange;
			msPost.t = m_TStartLimit + tLimitChange;
			if (m_nNextChangeType & CNG_SEG)				// check if next change is a segment change
			{
				msPost.s = 1;						// set just before change
				msPost.seg = GetSegNumber();
			}
			else if (m_nNextChangeType & CNG_LIMIT)	// else check if next change is a limit change
			{
				msPost.s = m_NextLimit.s;		// set just before change (on current limit)
				msPost.seg = m_NextLimit.seg;
			}
//			else if (m_nNextChangeType & CNG_DRIVEPROPS)		// won't occur as yet!

		}
		else
			break;			// got to point without any changes

		if (m_bNextLimitIsEndCalc && (m_nChangeType & CNG_LIMIT))	// check if at end calc limit
		{
			// set previous step limit time relative to current limit
			m_TLimit = tLimitPrev - m_TLimitsChanged;					// no change if no limits changed
			// maybe should set ms1 to last pos?
			ms1 = locPrev;
			GetVelAccAtS(ms1);
			m_bAtEndOfLimits = true;
			return;
		}

		if (m_nChangeType & m_nStopAt)		// stopped at a change
		{
			if (m_nChangeLoc == NO_CHANGE)			// if not found closest step yet
			{
				// if stopped at a change, find nearest discrete point
				double timeToChange = tLimitChange - tLimitPrev;
				m_idTSteps = NEARINT(timeToChange * m_dTInv);	// get closest time step
				ASSERT(m_idTSteps > 0);
				m_TLimit = tLimitPrev + m_idTSteps * m_dT;	// m_TLimit is time to check for changes to
				msPre.s = msPost.s;
				msPre.seg = msPost.seg;
				msPre.t = msPost.t;
				if (m_bGetPreChangeValues)			// get msPre before first change
					GetVelAccAtS(msPre);
			}

			if (tLimitChange < m_TLimit)		// if the change is before the nearest step
			{
				// closest step is after change. Recheck up to m_TLimit!
				m_nChangeLoc = m_nPreChangeLoc = m_nPostChangeLoc = CHANGE_IS_BEFORE_STEP;
				// do anymore changes up to m_TLimit, get msPost at last change, get ms1 at m_TLimit
			}
			else										// if the change is after the nearest step
			{
				if (m_nChangeLoc != CHANGE_IS_AFTER_STEP)		// moving from before to after step or first time is after - get at step
				{
					// get ms1 at m_TLimit
					ms1.s = msPost.s;			// msPost has closest starting point to search for m_TLimit from
					// find pos at the new time step, get ms1 at m_TLimit
					double PosLA = GetLimitPosAtLimitTime(m_TLimit);		// calc next pos depending on limit type
					FindSAtPos(ms1, PosLA, m_CurrLimit.axis, FSAP_SIGNFROMCURRLIMIT);			// find S at next position, starts search from ms1 location
					GetVelAccAtS(ms1);
					ms1.t = m_TStartLimit + m_TLimit;		// set ms1 time
				
//					double t = GetLimitTimeUsingdsdtAt(ms1);		// for test
//					ASSERT(fabs(t - m_TLimit) <= 1e2 * m_Tol.time);

					if (m_nChangeLoc == NO_CHANGE)
						m_nPreChangeLoc = CHANGE_IS_AFTER_STEP;
					m_nPostChangeLoc = CHANGE_IS_AFTER_STEP;
					m_nChangeLoc = CHANGE_IS_AFTER_STEP;	// closest step is before change
				}
			}
		}

		DoChange();
		if ((m_nChangeType & CNG_SEG) && msPost.s == 1)	// if it was a seg change
		{
			msPost.s = ms1.s;			// = 0
			msPost.seg = ms1.seg;	// = seg++
		}
	}		// for (;;)


	if (m_nChangeLoc != NO_CHANGE)
		if (m_bGetPostChangeValues)		// changes have been done, get msPost at last change
			GetVelAccAtS(msPost);

	if (m_nChangeLoc != CHANGE_IS_AFTER_STEP)		// haven't got ms1
	{
		if (m_nChangeLoc == CHANGE_IS_BEFORE_STEP)
			ms1.s = msPost.s;			// msPost has closest starting point to search for m_TLimit from
		// find pos at the new time step, get ms1 at m_TLimit
		double PosLA = GetLimitPosAtLimitTime(m_TLimit);		// calc next pos depending on limit type
		FindSAtPos(ms1, PosLA, m_CurrLimit.axis, FSAP_SIGNFROMCURRLIMIT);			// find S at next position, starts search from ms1 location
			// sign Vel = sign Acc if accel, -sign Acc if Braking, sign Vel if Vel limit
		GetVelAccAtS(ms1);
		ms1.t = m_TStartLimit + m_TLimit;		// set ms1 time
	}
	if (m_nChangeLoc == CHANGE_IS_BEFORE_STEP)	// just for checking!
	{
//		double t = GetLimitTimeUsingdsdtAt(ms1);
//		ASSERT(fabs(t - m_TLimit) <= m_Tol.time || m_bAtEndOfPath);
	}

	m_iTStepPath += m_idTSteps;		// keep track of absolute number of steps
	if (m_idTSteps == 0)
		TRACE0("CPathMove::AdvanceBydT() advanced by 0 steps!\n");

}





void CPathMove::AdvanceToSPoint()
{
/*
while (1)
{
	find which -if any- change occurs next (poly, limit or drvprops) 
	if (no change) break
	if (stopping at change) break
	do next change
}
  got to point to get state
*/
	double sNext = ms1.s;
	int segNext = ms1.seg;
	m_nChangeLoc = NO_CHANGE;
	for (;;)
	{
		ms1.s = sNext;			// set to next attempted point, will be brought back to a change it necessary
		ms1.seg = segNext;
		m_nChangeType = 0;			// flags

		//////////////////////////////////////////
		// check if at or past poly change
		if (ms1.s >= 1 || ms1.seg > GetSegNumber())		// current path segment
		{
			m_nChangeType |= CNG_SEG;
			ms1.s = 1;					// set just before change
			ms1.seg = GetSegNumber();
		}

		//////////////////////////////////////////
		// check if at or past next limit change
		if (ms1.Loc() >= m_NextLimit.Loc())		// includes next limit being at start of next seg if currently a end of seg!
		{
			// check if past next limit change
			if (ms1 > m_NextLimit)
			{
				m_nChangeType = CNG_LIMIT;
				ms1.s = m_NextLimit.s;			// set just before change (on current limit)
				ms1.seg = m_NextLimit.seg;
			}
			else
				m_nChangeType |= CNG_LIMIT;		// at change already
		}

		//////////////////////////////////////////
		// check if at or past drive property change
//		if ((ms1.s >= m_NextDrvProp.s && ms1.seg == m_NextDrvProp.seg)
//				|| ms1.seg > m_NextDrvProp.seg)

		if (m_nChangeType == 0)						// got to point without any changes
			break;

		if (m_bNextLimitIsEndCalc && (m_nChangeType & CNG_LIMIT))	// check if at end calc limit
		{
			// maybe should set ms1 to last pos?
			m_bAtEndOfLimits = true;
			return;
		}

		if (m_nChangeType & m_nStopAt)		// stopped at a change
		{
			m_nChangeLoc = CHANGE_IS_AT_STEP;
			msPre.s = msPost.s = ms1.s;
			msPre.seg = msPost.seg = ms1.seg;	// msPost.s will be set after change anyway, required for segment change
			break;
		}

		DoChange();
	}		// while (1)

	GetMotion();		// at point so get motion state
}


/*
void CPathMove::GetMotion1()
{
/*
  got point to get state	(ms1.s has step s)

  if point before change get state
  get state pre change
  do change
  get state post change
  if point at change get state
  if point after change get state

*/ /*
	if (m_nChangeLoc == NO_CHANGE)
	{
		GetVelAccAtS(ms1);	// ms1.s has step s
		m_TLimit = GetLimitTimeUsingdsdtAt(ms1);	
		return;
	}

// if step is before change
	if (m_nChangeLoc == STEP_IS_BEFORE_CHANGE)
	{
		GetVelAccAtS(ms1);					// ms1.s has step s
		m_TLimit = GetLimitTimeUsingdsdtAt(ms1);	
	}
// do prechange calcs
	if (m_bGetPreChangeValues && (m_nChangeLoc != STEP_IS_AFTER_CHANGE))	// if step is after change prechange has been done already!
	{
		GetVelAccAtS(msPre);
	}
// do change
	if (m_nChangeType != 0)
		DoChange();
// do postchange calcs
	if (m_bGetPostChangeValues)
	{
		if (m_nChangeLoc == STEP_IS_AT_CHANGE)
		{
			GetVelAccAtS(ms1);
			m_TLimit = GetLimitTimeUsingdsdtAt(ms1);	
			msPost = ms1;						// step value is post change value
		}
		else
			GetVelAccAtS(msPost);
	}
// if step is at change
	if (m_nChangeLoc == STEP_IS_AT_CHANGE)
		if (!m_bGetPostChangeValues)		// if wasn't peviously calculated!
		{
			GetVelAccAtS(ms1);				// ms1.s has step s
			m_TLimit = GetLimitTimeUsingdsdtAt(ms1);	
		}
// if step is after change
	if (m_nChangeLoc == STEP_IS_AFTER_CHANGE)
	{
		GetVelAccAtS(ms1);					// ms1.s has step s
		m_TLimit = GetLimitTimeUsingdsdtAt(ms1);	
	}
}
*/

void CPathMove::GetMotion()
{
/*
  got point to get state	(ms1.s has step s)

  if point before change get state
  get state pre change
  do change
  get state post change
  if point at change get state
  if point after change get state

*/
	if (m_nChangeLoc == NO_CHANGE)
	{
		GetVelAccAtS(ms1);	// ms1.s has step s
		m_TLimit = GetLimitTimeUsingdsdtAt(ms1);	
		return;
	}
// if step is before change
	if (m_nChangeLoc == CHANGE_IS_AFTER_STEP)
	{
		GetVelAccAtS(ms1);					// ms1.s has step s
		//double t = GetLimitTimeUsingdsdtAt(ms1);
		m_TLimit;
		if (m_bGetPreChangeValues)
			GetVelAccAtS(msPre);
		DoChange();
		if (m_bGetPostChangeValues)
			GetVelAccAtS(msPost);
		return;
	}
// if step is after change
	if (m_nChangeLoc == CHANGE_IS_BEFORE_STEP)
	{
		// if step is after change prechange has been done already!
		DoChange();
		if (m_bGetPostChangeValues)
			GetVelAccAtS(msPost);
		GetVelAccAtS(ms1);					// ms1.s has step s
		//double t = GetLimitTimeUsingdsdtAt(ms1);
		m_TLimit;
		return;
	}
// if step is at change
	if (m_nChangeLoc == CHANGE_IS_AT_STEP)
	{
		if (m_bGetPreChangeValues)
			GetVelAccAtS(msPre);
		DoChange();
		GetVelAccAtS(ms1);
		m_TLimit = GetLimitTimeUsingdsdtAt(ms1);
		msPost.s = ms1.s;
		msPost.seg = ms1.seg;
		if (m_bGetPostChangeValues)
			GetVelAccAtS(msPost);
		return;
	}
}

void CPathMove::GetTimeNextChange()
{
	m_nNextChangeType = 0;		// reset all next change flags
	if (m_NextLimit.seg == GetSegNumber())	// next change is limit
	{
		if (m_NextLimit.s == 1)			// next limit is at end of this segment - poly change too
		{
			m_nNextChangeType = CNG_LIMIT | CNG_SEG;
			m_TNextChange = GetEndSegTime();		// get time from seg pos to avoid problems from step change in vel at knot
		}
		else
		{
			m_nNextChangeType = CNG_LIMIT;
			m_TNextChange = GetEndLimitTime();
		}
	}
	else				// next change is poly
	{
		ASSERT(m_NextLimit.seg > GetSegNumber());
		ASSERT(m_CurrLimit.seg <= GetSegNumber());
		m_nNextChangeType = CNG_SEG;
		if (m_NextLimit.seg == GetSegNumber()+1 && m_NextLimit.s == 0)	// next limit is at start of next segment - limit change too
			m_nNextChangeType = CNG_SEG | CNG_LIMIT;
		m_TNextChange = GetEndSegTime();
	}
}



void CPathMove::DoChange()
{
//	ASSERT(m_nChangeLoc != NO_CHANGE);
	ASSERT(m_nChangeType != 0);		// new is it valid?
	if (m_nChangeType == 0)
		return;

	if (m_nChangeType & CNG_LIMIT)		// do limit change while on current poly
	{
		m_TStartLimit += m_TNextChange;		// path time is updated at end of limit
		m_TLimit -= m_TNextChange;				// m_TNextChange is limit time in this case
		m_TLimitsChanged += m_TNextChange;	// keeps track of changes for backtracking
		m_CurrLimit = m_NextLimit;
		if (m_posLL >= 0)
			m_NextLimit = m_pLimitList->GetNext(m_posLL);
		else					// end of limit list
		{
			m_NextLimit.SetPostEnd();
			m_bAtEndOfLimits = true;
		}
		m_bNextLimitIsEndCalc = (m_NextLimit.type == LT_ENDCALC);	// end, and don't use this final point!
	}
	if (m_nChangeType & CNG_SEG)
	{
		if (GetNextSegPolys())
		{
			ms1.s = 0;	// was ms1.s -= 1;		// set as starting point to find next position
			ms1.seg++;
		}
		else
		{
			m_bAtEndOfPath = true;
			ms1.s = 1;					// set to end of current poly
		}
		ASSERT(ms1.seg == GetSegNumber());
	}
	if (m_nChangeType & CNG_DRIVEPROPS)
	{
	}

	GetTimeNextChange();
}


///////////////////////////////////

#if 0
///////////////////
{
	if (ms1.s < 1)		// not end of segment
	{
		if (bSStepValue)
			nIncStep++;
		ms1.s = m_fSIncrement * nIncStep;
		bSStepValue = true;
		if (ms1.s >= (1 - 0.5 * m_fSIncrement))	// if comes to within 50% of increment from 1, move to 1
			ms1.s = 1;
	}


////////////
	if (!nPreChange)	// s was not set due to a pre limit or property change
	{
		bPreChangeDone = false;
		// Increment s
		if (ms1.s < 1)		// not end of segment
		{
			if (bSStepValue)
				nIncStep++;
			ms1.s = m_fSIncrement * nIncStep;
			bSStepValue = true;
			if (ms1.s >= (1 - 0.5 * m_fSIncrement))	// if comes to within 50% of increment from 1, move to 1
				ms1.s = 1;
		}
		else if (ms1.s == 1)		// at end of segment
		{
			if (!GetNextSegPolys())
				break;
			nIncStep = 0;		// move to start of next segment
			ms1.s = 0;
			ms1.seg++;
		}
		else
			ASSERT(0);		// ms1.s > 1 !!

		// Check if s is at or past a limit change
		if (ms1.s >= m_NextLimit.s && ms1.seg == m_NextLimit.seg)	// check for a limit change
		{
			if (ms1.s != m_NextLimit.s)
				bSStepValue = false;
			ms1.s = m_NextLimit.s;
			nPreChange |= PRECHANGE_LIMIT;
			if (ms1.s == 0)		// Don't use old limit on new polys.  A prechange was done with previous polys
				bPreChangeDone = true;
		}
	}
}

/////////////////////
#endif



#if 0

void CPathMove::GraphResults()
{




	while (1)
	{
		GetVelAccAtS(ms1);	// calc ds/dt, d2s/dt2, d3s/dt3 using current drive limit
		bPreChangeDone = true;		// only relavant if nPreChange was set

		// Store ms1 states
		if (!nPreChange)	// s was not set due to a pre limit or property change
		{
			bPreChangeDone = false;
			// Increment s
			if (ms1.s < 1)		// not end of segment
			{
				if (bSStepValue)
					nIncStep++;
				ms1.s = m_fSIncrement * nIncStep;
				bSStepValue = true;
				if (ms1.s >= (1 - 0.5 * m_fSIncrement))	// if comes to within 50% of increment from 1, move to 1
					ms1.s = 1;
			}
			else if (ms1.s == 1)		// at end of segment
			{
				if (!GetNextSegPolys())
					break;
				nIncStep = 0;		// move to start of next segment
				ms1.s = 0;
				ms1.seg++;
			}
			else
				ASSERT(0);		// ms1.s > 1 !!

			// Check if s is at or past a limit change
			if (ms1.s >= m_NextLimit.s && ms1.seg == m_NextLimit.seg)	// check for a limit change
			{
				if (ms1.s != m_NextLimit.s)
					bSStepValue = false;
				ms1.s = m_NextLimit.s;
				nPreChange |= PRECHANGE_LIMIT;
				if (ms1.s == 0)		// Don't use old limit on new polys.  A prechange was done with previous polys
					bPreChangeDone = true;
			}
		}
///////////
		if (nPreChange & PRECHANGE_LIMIT && bPreChangeDone)	// last was pre limit change location
		{
			// change to next limit
			// get time at end of limit
			tStartLimit += GetEndLimitTime();

			// s remains the same
			m_CurrLimit = m_NextLimit;
			if (!limitList.GetNext(m_NextLimit))	// end of limit list
				m_NextLimit.s = 2;		// will never reach this
			nPreChange &= ~PRECHANGE_LIMIT;	// reset flag
		}
		if (nPreChange & PRECHANGE_DRVPROPS && bPreChangeDone)	// last was pre drive properties change location
		{
			// s remains the same
			nPreChange &= ~PRECHANGE_DRVPROPS;	// reset flag
		}

	}	// while (1)

	ASSERT(m_NextLimit.s > 1);	// check got to end of limit list
	ASSERT(iPtIdx <= iNumPoints);

}


#endif

