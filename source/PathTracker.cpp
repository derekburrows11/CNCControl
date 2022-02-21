// PathTracker.cpp: implementation of the CPathTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNCControl.h"

#include "PathTracker.h"
#include "Settings.h"

//#include "PosConverter.h"


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
	dAcc = dt * Jerk = AccF - AccI
	dVel = dt * (AccI + AccF)/2
	dPos = dt * VelI + dt^2 * (2*AccI + AccF)/6
	     = dt * VelF - dt^2 * (AccI + 2*AccF)/6
	     = dt * (2*VelI + VelF)/3 + dt^2 * AccI/6
	     = dt * (VelI + 2*VelF)/3 - dt^2 * AccF/6
or
  	dAcc = dt * Jerk			(Jerk = dA)
	dVel = dt * AccI + dt^2 * Jerk/2
	dPos = dt * VelI + dt^2 * AccI/2 + dt^3 * Jerk/6

	dAcc = AccF - AccI										Acc is intergral values
	d2Vel = dt * (AccI + AccF)								Vel scaled by 2 for intergral values
	d6Pos = 3 * dt * 2VelI + dt^2 * (2*AccI + AccF)	Pos scaled by 6 for intergral values

	Intergral step sizes
	dt = dTime;			smallest time step
	diAcc = dAccel;
	diVel = diAcc * dt / 2		-> di2Vel = diAcc * dt
	diPos = diAcc * dt^2 / 6	->	di6Pos = diAcc * dt^2

	AccF = 2 * dVel/dt - AccI
	AccF = 6*(dPos - dt*VelI)/dt^2 - 2*AccI
	     = 3*(dt*VelF - dPos)/dt^2 - AccI/2

	PosWt = 0.1; 	// Weighting of Accel calculated to give no position error
	VelWt = 0.45;	// Weighting of Accel calculated to give no velocity error
	AccWt = 0.45;	// Weighting of Accel calculated to give no acceleration error (=acc value)
	Acc = AccWt * NextAcc + VelWt * AccNoVerr + PosWt * AccNoPerr;



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

CPathTracker::CPathTracker()
{
	m_pPathDoc = NULL;
	m_pLimitList = NULL;
}

CPathTracker::~CPathTracker()
{
}

void CPathTracker::SetPathDoc(CPathDoc* pPathDoc)
{
	m_pPathDoc = pPathDoc;
	if (m_pPathDoc)
		ASSERT_VALID(m_pPathDoc);
}

void CPathTracker::Init()		// virtual function but this is the lowest base class
{
	SetDefaults();
}

void CPathTracker::SetDefaults()
{
	double fDefTol = 1e-8;
	m_Tol.def = fDefTol;
	m_Tol.vsmall = 1e-6;		// was, 1e-4, 1e2 * m_Tol.def;
	m_Tol.pos = 1e-10;
	m_Tol.vel = 1e1 * fDefTol;
	m_Tol.acc = fDefTol;
	m_Tol.jerk = fDefTol;
	m_Tol.dsdt = 1e-8;		// when pos =~ 2000 dPos =~ 1e-10, *2*Acc =~ 1e-8
	m_Tol.time = 1e-7;
	m_Tol.accStep = 0.2;
	
	m_Seg.seg = INVALIDREF;

	m_StartSeg = 0;
	m_EndSeg = MAX_SEG;

	m_posLL = -1;
	m_bAtEndOfPath = false;

	m_bVofSlaChangedSign = false;
	m_nVofSlaChangedSign = 0;

	GetMachineDriveProp(m_DrvPropMachine);
	SetDriveProps(m_DrvPropMachine);

	SetFeedRate(FEEDRATE_DEFAULT);
	SetTool(TOOL_DEFAULT);

//	m_vtPosMachineOfPathOrigin = 0;
}

void CPathTracker::SetToStart()
{
	m_StartSeg = 0;

	ms1.s = 0;
	ms1.seg = m_StartSeg;
	ms1.t = 0;

	m_posLL = -1;					// invalid
	m_bAtEndOfPath = false;
	m_Seg.seg = INVALIDREF;		// needed to path changed or not used yet

	GetInitPathProps();		// sets m_Seg.seg to 
//	GetInitSegPolys();		// so m_Seg is valid

}
/*
void CPathTracker::SetPosMachineOfPathStart(CVector& vtPosMachPathStart)
{
	ASSERT(GotPath());
	CVector vtPosPathStart;
	GetPathStartPos(vtPosPathStart);
	m_vtPosMachineOfPathOrigin = vtPosMachPathStart - vtPosPathStart;
}
*/
void CPathTracker::GetInitPathProps()
{
	SetFeedRate(FEEDRATE_DEFAULT);		// default path value
	SetTool(TOOL_DEFAULT);		// default path value

	m_Seg.seg = INVALIDREF;
	m_Seg.nrPathPropChange = INVALIDREF;
	m_pPathDoc->GetFirstNode(m_Seg.nrFinal);	// search to first point node
	m_Seg.numPathPropChanges = 2;					// set to search for multiple commands
	m_pPathDoc->FindLocNextSegPathPropChange(m_Seg);

	while (GetPathPropChangeLoc().seg == m_Seg.seg)
		GetNextPathProp();
}

void CPathTracker::GetMachineDriveProp(SDriveProperties& dp)
{
	SMachineParameters& mp = g_Settings.MachParam;
	dp.vtVel = mp.vtVelMax;
	dp.vtAcc = mp.vtAccMax;
	dp.vtDeAcc = mp.vtAccMax;

	dp.feedRate = mp.FeedRateMax;
	dp.xySpeed = mp.SpeedXYMax;
	dp.xyzSpeed = mp.SpeedXYZMax;
//	dp.load = 0;
}

void CPathTracker::SetPathProps(const SPathProperties& pp)
{
	m_Seg.pathProps = pp;
}

void CPathTracker::SetDrivePropsFromPathProps()
{
	SetFeedRate(m_Seg.pathProps.feedRate);
}

void CPathTracker::SetDriveProps(const SDriveProperties& dp)
{
	m_DrvPropCurr = dp;
	SetLimitsFrom(m_DrvPropCurr);
}

void CPathTracker::SetLimitsFrom(const SDriveProperties& dp)
{
	vtVelMax = dp.vtVel;
	vtVelMin = -dp.vtVel;
	vtAccMax = dp.vtAcc;
	vtAccMin = -dp.vtAcc;

	double fact = 100;
	vtVelMaxTol	= vtVelMax + fact * m_Tol.vel;
	vtVelMinTol	= vtVelMin - fact * m_Tol.vel;
	vtAccMinTol	= vtAccMin - fact * m_Tol.acc;
	vtAccMaxTol	= vtAccMax + fact * m_Tol.acc;
}

void CPathTracker::SetFeedRate(double feedRate)		// -1 for rapid feed
{
	m_Seg.pathProps.feedRate = feedRate;
	if (feedRate == FEEDRATE_RAPID)		// for rapid feed
		feedRate = m_DrvPropMachine.feedRate;

	m_DrvPropCurr.feedRate = feedRate;
	m_DrvPropCurr.xySpeed = feedRate;
	m_DrvPropCurr.xyzSpeed = feedRate;
	m_DrvPropCurr.vtVel = feedRate;
	// restrict to machine limits
	m_DrvPropCurr.LimitTo(m_DrvPropMachine);
	SetLimitsFrom(m_DrvPropCurr);
}

void CPathTracker::SetTool(int tool)
{
	m_Seg.pathProps.iTool = tool;
}


void CPathTracker::GetNextPathProp()		// load m_DrvPropCurr with new properties
{
	ASSERT(m_Seg.numPathPropChanges != 0);
	CPathNode* pNode = m_pPathDoc->GetPathNode(m_Seg.nrPathPropChange);
	int type = pNode->type;
	ASSERT(pNode->z == m_Seg.locPathPropChange.s);
	if (type == PNT_SETFEEDRATE)
	{
		ASSERT(pNode->y == m_Seg.pathProps.feedRate);
		SetFeedRate(pNode->x);
	}
	else if (type == PNT_SETDRIVEPROP)
	{
		ASSERT(0);
	}
	else if (type == PNT_SELECTTOOL)
	{
		ASSERT(pNode->y == m_Seg.pathProps.iTool);
		SetTool((int)pNode->x);
	}
	else
		ASSERT(0);

	// find next change if any
	m_pPathDoc->FindLocNextSegPathPropChange(m_Seg);
}

void CPathTracker::GetPrevPathProp()		// load m_PathPropCurr with new properties
{
	ASSERT(m_Seg.numPathPropChanges != 0);
	CPathNode* pNode = m_pPathDoc->GetPathNode(m_Seg.nrPathPropChange);
	int type = pNode->type;
	ASSERT(pNode->z == m_Seg.locPathPropChange.s);
	if (type == PNT_SETFEEDRATE)
	{
		ASSERT(pNode->x == m_Seg.pathProps.feedRate);
		SetFeedRate(pNode->y);
	}
	else if (type == PNT_SETDRIVEPROP)
	{
		ASSERT(0);
	}
	else if (type == PNT_SELECTTOOL)
	{
		ASSERT(pNode->x == m_Seg.pathProps.iTool);
		SetTool((int)pNode->y);
	}
	else
		ASSERT(0);

	// find previous change if any
	m_pPathDoc->FindLocPrevSegPathPropChange(m_Seg);
}



/////////////////////////////////////////////////
// Get Path motion state functions
/////////////////////////////////////////////////

void CPathTracker::GetPolysAtS(SMotionState& ms)
{
// PRE-REQUISIT: set ms.s ONLY
	ASSERT(ms.seg == m_Seg.seg);		// just a check, ms.seg may not always have been set anyway
	CMatrix& vtPowerS = m_vtPowerS;
	vtPowerS.Power3Vector(ms.s);
	m_Seg.pos.SolvePolyAt(vtPowerS, ms.vtPofS);
	m_Seg.vel.SolvePolyAt(vtPowerS, ms.vtVofS);
	m_Seg.acc.SolvePolyAt(vtPowerS, ms.vtAofS);
	m_Seg.jerk.SolvePolyAt(vtPowerS, ms.vtJofS);
}




void CPathTracker::GetPolysArcAtS(SMotionState& ms)
{	// for arc segments
	// given start, end, centre nodes and direction
	// or given 2 axis vectors, centre nodes and arc angle
//	CVector vtStart, vtEnd;
	CVector vtCentre;
	CVector vtAxis1, vtAxis2;
	double angArc = 1;

	double ang = ms.s * angArc;
	double angArcSq = angArc * angArc;
	double cosAng = cos(ang);
	double sinAng = sin(ang);
	CVector vtRadius = vtAxis1 * cosAng + vtAxis2 * sinAng;
	ms.vtPofS = vtCentre + vtRadius;
	ms.vtVofS = vtAxis1 * (-sinAng*angArc) + vtAxis2 * (cosAng*angArc);
	ms.vtAofS = vtRadius * (-angArcSq);
	ms.vtJofS = ms.vtVofS * (-angArcSq);
}

double CPathTracker::GetSegChordLength()	// gives straight length between points - quick
{
	CVector vtPos0, vtPos1;
	m_Seg.pos.SolvePolyAt0(vtPos0);
	m_Seg.pos.SolvePolyAt1(vtPos1);
	double len = (vtPos1 - vtPos0).Mag();
	m_Seg.length = len;
	return len;
}

double CPathTracker::GetSegChordLengthBetween(double s0, double s1)	// gives straight length between s points - quick
{
	CMatrix& vtPowerS = m_vtPowerS;
	CVector vtPos0, vtPos1;
	vtPowerS.Power3Vector(s0);
	m_Seg.pos.SolvePolyAt(vtPowerS, vtPos0);
	vtPowerS.Power3Vector(s1);
	m_Seg.pos.SolvePolyAt(vtPowerS, vtPos1);
	return (vtPos1 - vtPos0).Mag();
}

double CPathTracker::GetSegLength()
{
	CMatrix& vtPowerS = m_vtPowerS;
	CVector vtPos0, vtPos1;
	double len = 0;
	double ds = 0.05;
	double s = ds;
	m_Seg.pos.SolvePolyAt0(vtPos0);
	while (s < 0.999)
	{
		vtPowerS.Power3Vector(s);
		m_Seg.pos.SolvePolyAt(vtPowerS, vtPos1);
		len += (vtPos1 - vtPos0).Mag();
		vtPos0 = vtPos1;
		s += ds;
	}
	m_Seg.pos.SolvePolyAt1(vtPos1);
	len += (vtPos1 - vtPos0).Mag();
	m_Seg.length = len;
	return len;
}

double CPathTracker::GetSegLengthBetween(double s0, double s1)
{
	CMatrix& vtPowerS = m_vtPowerS;
	CVector vtPos0, vtPos1;
	double len = 0;
	double ds = 0.05;
	double s = s0 + ds;
	vtPowerS.Power3Vector(s0);
	m_Seg.pos.SolvePolyAt(vtPowerS, vtPos0);
	while (s < s1-0.001)
	{
		vtPowerS.Power3Vector(s);
		m_Seg.pos.SolvePolyAt(vtPowerS, vtPos1);
		len += (vtPos1 - vtPos0).Mag();
		vtPos0 = vtPos1;
		s += ds;
	}
	vtPowerS.Power3Vector(s1);
	m_Seg.pos.SolvePolyAt(vtPowerS, vtPos1);
	len += (vtPos1 - vtPos0).Mag();
	return len;
}

void CPathTracker::GetVelAccAtState(SMotionState& ms)
{
/* calc Vel and Acc from s & ds/dt using an acceleration limit
	Sets: m_Limit, ms.d2sdt2
	Used if path properties change (and at start/end)
*/
// PRE-REQUISIT: set: ms.s, ms.dsdt

	GetPolysAtS(ms);
	ms.vtPos = ms.vtPofS;
	ms.vtVel = ms.vtVofS * ms.dsdt;
	GetMaxAccelAtVel(ms, 1, &m_Limit);		// sets up m_Limit
	ms.d2sdt2 = m_Limit.d2sdt2;
	ms.vtAcc = ms.vtAofS * (ms.dsdt*ms.dsdt) + ms.vtVofS * ms.d2sdt2;

//	ms.d3sdt3 is not set - relavant??
//	ms.vtJerk = ms.vtJofS * (dsdt*dsdt*dsdt) + ms.vtAofS * (3 * d2sdt2*dsdt) + ms.vtVofS * d3sdt3;
}

void CPathTracker::GetVelAtSNoPoly(SMotionState& ms, SDriveLimit& limit)
{
// calc only ds/dt using current drive limit
// then get velocity vector
// PRE-REQUISIT: set ms.s and GetPolysAtS(ms)
// copy of GetVelAccAtSNoPoly() without Acc bits!

	m_bVofSlaChangedSign = false;
	ms.vtPos = ms.vtPofS;
	double& dsdt  = ms.dsdt;
	int limType = limit.type;

	if (limType == LT_CORNER)
	{
		GetCurveBreakaway(ms);	// could use GetdsdtMaxCorner(ms) but GetCurveBreakaway is required later anyway
		dsdt = ms.dsdtMax;
		ms.vtVel = ms.vtVofS * dsdt;
		return;
	}
	if (ms == limit)	// if still at start of limit span
	{
		dsdt = limit.dsdt;	// just take ds/dt from limit
		ms.vtVel = ms.vtVofS * dsdt;
		return;
	}

	double PofSla, VofSla, AofSla;	// of limit axis at S
	int la = limit.axis;
	PofSla = ms.vtPofS[la];
	VofSla = ms.vtVofS[la];
	AofSla = ms.vtAofS[la];

	double Velsq;
	double dVelsq;
	double& Vella = ms.Vella;			// Vel of limit axis at S
	double& Accla = ms.Accla;
	double dsdtSq;

	double VofSsmall = 100 * m_Tol.vsmall * ms.vtVofS.SumAbs();	// was 10 * m_Tol.vsmall * ...
	Accla = limit.acc;
	Vella = limit.vel;

	if (limType != LT_AXIS_VELOCITY)
	{
		dVelsq = 2 * Accla * (PofSla - limit.pos);
		Velsq = limit.vel*limit.vel + dVelsq;
		if (Velsq < 0)
		{
			if (Velsq < -m_Tol.vel)
			{
				double posZeroVel = limit.pos - (limit.vel*limit.vel / (2*Accla));
				double distFromZeroVel = PofSla - posZeroVel;
				LOGERROR1("Position unreachable with current acceleration by %.3fmm", distFromZeroVel);
			}
		//	else
		//		TRACE3("Velsq is a very small negative (%g) in CPathTracker::GetVelAccAtS() with BRAKETO at %i+%g\n", Velsq, ms.seg, ms.s);
			ASSERT(limType == LT_AXIS_BRAKETO || limType == LT_AXIS_BRAKEFROM);
			Vella = 0;
		}
		else
			Vella = sqrt(Velsq);

		if (limType == LT_AXIS_ACCELFROM || limType == LT_AXIS_ACCELTO)
		{
			if (Accla < 0)
				Vella = -Vella;
		}
		else if (limType == LT_AXIS_BRAKEFROM || limit.type == LT_AXIS_BRAKETO)
		{
			if (Accla > 0)
				Vella = -Vella;
		}
		else
		{
			LOGERROR("Bad limit type");		// Limit type not handled
			ASSERT(0);
		}
	}

	
	// find ds/dt
	if (fabs(VofSla) > VofSsmall)		// if not nearly parallel to axis
	{
		dsdt = Vella / VofSla;
		if (dsdt < 0)
		{
			m_bVofSlaChangedSign = true;		// must check this  after moving along path
			m_nVofSlaChangedSign++;
			LOGMESSAGE("VofSla sign changed");
			dsdt = -dsdt;			// values won't be correct, won't reach zero vel point properly
		}
	}
	else if (fabs(Vella) > m_Tol.vsmall && (VofSla == 0 || fabs(dsdt) > 1e8))	// check for illegal situation
	{
		dsdt = DSDT_ERRORVERYLARGE;
	}
	else if (fabs(AofSla) > m_Tol.vsmall)	// assume VofSla is 0
	{
		// adjusts speed to give correct axis accel!
		dsdtSq = Accla / AofSla;		// won't be correct if just changed segs with step change in AofSla!!!
		ASSERT(dsdtSq > 0);
		dsdt = sqrt(dsdtSq);			// Should always be positive
		ASSERT(fabs(VofSla * dsdt - Vella) <= 20 * VofSsmall);
	}
	else		// on a straight line with perpendicular acc
	{
		dsdt = DSDT_ERRORVERYLARGE;
/*
		if (Vella != 0)		// if 0 then must be a zero speed limit not on orig assigned poly
			TRACE0("VofS and AofS of limit axis are very small in CPathTracker::GetVelAccAtS()!\n");
		dsdt = 0;
		ASSERT(fabs(Vella) <= VofSsmall);
*/
	}

	ms.vtVel = ms.vtVofS * dsdt;
}


void CPathTracker::GetVelAccAtSNoPoly(SMotionState& ms, SDriveLimit& limit)
{
// Vel calc's are copy of GetVelAtSNoPoly()
// calc only ds/dt using current drive limit
// then get velocity vector
// calc d2s/dt2, d3s/dt3 using ds/dt and current drive limit
// then get acceleration and Jerk vectors
// PRE-REQUISIT: set ms.s and GetPolysAtS(ms)

	m_bVofSlaChangedSign = false;
	ms.vtPos = ms.vtPofS;
	double& dsdt   = ms.dsdt;
	int limType = limit.type;

	if (limType == LT_CORNER)
	{
		GetCurveBreakaway(ms);	// could use GetdsdtMaxCorner(ms) but GetCurveBreakaway is required later anyway
		dsdt = ms.dsdtMax;
		ms.vtVel = ms.vtVofS * dsdt;
		ms.d2sdt2 = 0;	// for Acc only	d2s/dt2 & d3s/dt3 not relevant
		ms.d3sdt3 = 0;	// for Acc only
		return;
	}

	double PofSla, VofSla, AofSla;	// of limit axis at S
	double JofSla;		// for Acc only
	int la = limit.axis;
	PofSla = ms.vtPofS[la];
	VofSla = ms.vtVofS[la];
	AofSla = ms.vtAofS[la];
	JofSla = ms.vtJofS[la];	// for Acc only

	double Velsq;
	double dVelsq;
	double& Vella = ms.Vella;			// Vel of limit axis at S
	double& Accla = ms.Accla;
	double Jerkla = 0;				// for Acc only
	double& d2sdt2 = ms.d2sdt2;	// for Acc only
	double& d3sdt3 = ms.d3sdt3;	// for Acc only
	double dsdtSq;

	double VofSsmall = 100 * m_Tol.vsmall * ms.vtVofS.SumAbs();	// was 10 * m_Tol.vsmall * ...
	Accla = limit.acc;
	Vella = limit.vel;

	bool bdsdtSure = false;
	if (ms == limit)	// if still at start of limit span
	{
		dsdt = limit.dsdt;	// just take ds/dt from limit
		ASSERT(fabs(VofSla * dsdt - Vella) <= 20 * VofSsmall);	// Vella valid??
//		bdsdtSure = true;
	}
	else		// if is limit dsdt is from limit!
	{
//		bdsdtSure = false;
		if (limType != LT_AXIS_VELOCITY)
		{
			dVelsq = 2 * Accla * (PofSla - limit.pos);
			Velsq = limit.vel*limit.vel + dVelsq;
			if (Velsq < 0)
			{
				if (Velsq < -m_Tol.vel)
				{
					double posZeroVel = limit.pos - (limit.vel*limit.vel / (2*Accla));
					double distFromZeroVel = PofSla - posZeroVel;
					LOGERROR1("Position unreachable with current acceleration by %.3fmm", distFromZeroVel);
					ASSERT(limType == LT_AXIS_BRAKETO || limType == LT_AXIS_BRAKEFROM);	// usualy
				}
			//	else
			//		TRACE3("Velsq is a very small negative (%g) in CPathTracker::GetVelAccAtS() with BRAKETO at %i+%g\n", Velsq, ms.seg, ms.s);
				Vella = 0;
			}
			else
				Vella = sqrt(Velsq);

			if (limType == LT_AXIS_ACCELFROM || limType == LT_AXIS_ACCELTO)
			{
				if (Accla < 0)
					Vella = -Vella;
			}
			else if (limType == LT_AXIS_BRAKEFROM || limit.type == LT_AXIS_BRAKETO)
			{
				if (Accla > 0)
					Vella = -Vella;
			}
			else
			{
				LOGERROR("Bad limit type");		// Limit type not handled
				ASSERT(0);
			}

		}
		// find ds/dt
		dsdt = Vella / VofSla;		// may be changed if VofSla is small
		if (dsdt < 0 && fabs(VofSla) > VofSsmall)
		{
			m_bVofSlaChangedSign = true;		// must check this  after moving along path
			m_nVofSlaChangedSign++;
			LOGMESSAGE("VofSla sign changed");
			dsdt = -dsdt;			// values won't be correct, won't reach zero vel point properly
		}
	}	// if (ms != limit)


	if (fabs(VofSla) > VofSsmall)		// if not nearly parallel to axis
	{
		ASSERT(dsdt >= 0);
		// dsdt is OK
		// following for Acc only
		// Calculate d2sdt2 for limit axis acceleration - Jerkla is 0 for Vel and Accel limits
		dsdtSq = dsdt*dsdt;
		d2sdt2 = (Accla - AofSla * dsdtSq) / VofSla;
		d3sdt3 = - (JofSla * dsdtSq*dsdt + 3 * AofSla * d2sdt2*dsdt) / VofSla;
	}
	//else if (fabs(Vella) > m_Tol.vsmall && (VofSla == 0 || fabs(dsdt) > 1e8))	// check for illegal situation
	else if (fabs(Vella) > VofSsmall && (VofSla == 0 || fabs(dsdt) > 1e8))	// check for illegal situation		changed 9/5/05
	{
		dsdt = DSDT_ERRORVERYLARGE;
		// following for Acc only
		dsdtSq = dsdt*dsdt;
		d2sdt2 = 0;
		d3sdt3 = 0;
	}
	else if (fabs(AofSla) > m_Tol.vsmall)	// assume VofSla is 0
	{
		if (bdsdtSure)
			dsdtSq = dsdt*dsdt;
		else
		{
			// adjusts speed to give correct axis accel!
			dsdtSq = Accla / AofSla;		// won't be correct if just changed segs with step change in AofSla!!!
			ASSERT(dsdtSq > 0);
			dsdt = sqrt(dsdtSq);			// Should always be positive

			// following for Acc only
			// iterate
			d2sdt2 = (Jerkla / dsdt - JofSla * dsdtSq) / (3*AofSla);
			dsdtSq = (Accla - VofSla*d2sdt2) / AofSla;
			dsdt = sqrt(dsdtSq);			// Should always be positive
		}
		// found dsdt
		// iterate - assume VofSla is 0
		d2sdt2 = (Jerkla / dsdt - JofSla * dsdtSq) / (3*AofSla);	// or d2sdt2 = - JofSla * Accla / (3 * AofSla*AofSla);	requires Accla to be correct sign, determine by AofS if VofS is small!
		d3sdt3 = - (6 * JofSla * d2sdt2*dsdtSq + 3 * AofSla * d2sdt2*d2sdt2) / (4 * AofSla * dsdt);
				// or d3sdt3 = - ((6 * JofSla * d2sdt2*dsdt*dsdt)/AofSla + 3 * d2sdt2*d2sdt2) / (4 * dsdt);	this has two / though!
		// iterate - with VofSla and d3sdt3 from 1st iteration
		d2sdt2 = (Jerkla - JofSla * dsdtSq*dsdt - VofSla * d3sdt3) / (3*AofSla*dsdt);	// or d2sdt2 = - JofSla * Accla / (3 * AofSla*AofSla);	requires Accla to be correct sign, determine by AofS if VofS is small!
		//d3sdt3 = - (6 * JofSla * d2sdt2*dsdtSq + 3 * AofSla * d2sdt2*d2sdt2) / (4 * AofSla * dsdt);

		ASSERT(fabs(VofSla * dsdt - Vella) <= 20 * VofSsmall);
	}
	else		// on a straight line with perpendicular acc
	{
		dsdt = DSDT_ERRORVERYLARGE;
		// following for Acc only
		dsdtSq = dsdt*dsdt;
		d2sdt2 = 0;
		d3sdt3 = 0;
/*
		if (Vella != 0)		// if 0 then must be a zero speed limit not on orig assigned poly
			TRACE0("VofS and AofS of limit axis are very small in CPathTracker::GetVelAccAtS()!\n");
		dsdt = 0;
		// following for Acc only
		if (Accla != 0)		// if 0 then must be a zero speed limit not on orig assigned poly
			TRACE0("VofS and AofS of limit axis are very small in CPathTracker::GetVelAccAtS()!\n");
		dsdtSq = 0;
		d2sdt2 = 0;
		d3sdt3 = 0;
		ASSERT(fabs(Vella) <= VofSsmall);
*/
	}

	ms.vtVel = ms.vtVofS * dsdt;
	// following for Acc only
	ms.vtAcc = ms.vtAofS * (dsdtSq) + ms.vtVofS * d2sdt2;
	ms.vtJerk = ms.vtJofS * (dsdtSq*dsdt) + ms.vtAofS * (3 * d2sdt2*dsdt) + ms.vtVofS * d3sdt3;

}

// start of orig
/*
void CPathTracker::GetVelAtSNoPolyOld(SMotionState& ms, SDriveLimit& limit)
{
	ms.vtPos = ms.vtPofS;

	if (limit.type == LT_CORNER)
	{
		GetCurveBreakaway(ms);	// could use GetdsdtMaxCorner(ms) but GetCurveBreakaway is required later anyway
		ms.dsdt = ms.dsdtMax;
		ms.vtVel = ms.vtVofS * ms.dsdt;
		return;
	}
	if (ms == limit)	// if still at start of limit span
	{
		ms.dsdt = limit.dsdt;	// just take ds/dt from limit
		ms.Vella = limit.vel;
		ms.Accla = limit.acc;
		ms.vtVel = ms.vtVofS * ms.dsdt;
		return;
	}

	double PofSla, VofSla, AofSla;		// of limit axis at S
	int la = limit.axis;
	PofSla = ms.vtPofS[la];
	VofSla = ms.vtVofS[la];
	AofSla = ms.vtAofS[la];

	double Velsq;
	double& Vella = ms.Vella;			// Vel of limit axis at S
	double& Accla = ms.Accla;
	double& dsdt = ms.dsdt;


	switch (limit.type)
	{
	case LT_AXIS_ACCELFROM:
	case LT_AXIS_BRAKETO:
		Accla = limit.acc;
		Velsq = limit.vel*limit.vel + 2*Accla * (PofSla - limit.pos);
		if (Velsq < 0)
		{
			if (Velsq < -m_Tol.vel)
				LOGERROR("Position unreachable with current acceleration");		// Position unreachable with current acceleration
			else
				TRACE1("Velsq is a very small negative in CPathTracker::GetVelAtS with BRAKETO (%g)\n", Velsq);
			Vella = 0;
		}
		else
			Vella = sqrt(Velsq);
		if (VofSla < 0)
			Vella = -Vella;
		if (limit.type == LT_AXIS_ACCELFROM)
			ASSERT((Vella - limit.vel) * Accla >= 0);		// check vel change and accel are same sign!
		else	// (limit.type == LT_AXIS_BRAKETO)
			if (fabs(VofSla) > m_Tol.vsmall)
				ASSERT((Vella - limit.vel) * Accla <= 0);		// check vel change and accel are opposite sign!
		break;

	case LT_AXIS_VELOCITY:
		Vella = limit.vel;
		Accla = 0;
		break;

	case LT_AXIS_BRAKEFROM:
		Accla = limit.acc;
		Velsq = limit.vel*limit.vel + 2*Accla * (PofSla - limit.pos);	// was  - fabs(2*Accla * (PofSla - limit.pos))
		if (Velsq < 0)
		{
			if (Velsq < -m_Tol.vel)
				LOGERROR("Braked through zero velocity");
			else
				TRACE3("Velsq is a very small negative (%g) in CPathTracker::GetVelAtS() with BRAKEFROM at %i+%g\n", Velsq, ms.seg, ms.s);
			Vella = 0;
		}
		else
			Vella = sqrt(Velsq);
		if (VofSla < 0)
			Vella = -Vella;
		ASSERT((Vella - limit.vel) * Accla >= 0);		// check vel change and accel are same sign!
		break;

	default:
		LOGERROR("Bad limit type");		// Limit type not handled
		ASSERT(0);
	}

	if (fabs(VofSla) > m_Tol.vsmall)
		dsdt = Vella / VofSla;
	else if (fabs(Vella) > m_Tol.vsmall)
		dsdt = DSDT_ERRORVERYLARGE;
	else if (fabs(AofSla) > m_Tol.vsmall)
	{
//			TRACE0("VofS of limit axis is very small in CPathTracker::GetVelAtS()\n");
		ASSERT(Accla / AofSla > 0);
		dsdt = sqrt(Accla / AofSla);			// Should always be positive
	}
	else
	{
		if (Vella != 0)		// if 0 then must be a zero speed limit not on orig assigned poly
		{
			TRACE0("VofS and AofS of limit axis are very small in CPathTracker::GetVelAtS()!\n");
			ASSERT(0);
		}
		dsdt = 0;
	}
	if (dsdt != DSDT_ERRORVERYLARGE)
		ASSERT(fabs(VofSla * dsdt - Vella) <= m_Tol.vsmall);
	ASSERT(dsdt >= 0);
	ms.vtVel = ms.vtVofS * dsdt;
}
*/

void CPathTracker::GetVelAccAtSNoPolyOld(SMotionState& ms, SDriveLimit& limit)
{
// PRE-REQUISIT: set ms.s ONLY

	GetVelAtSNoPoly(ms, limit);

// calc d2s/dt2, d3s/dt3 using ds/dt and current drive limit
// then get acceleration and Jerk vectors

	if (limit.type == LT_CORNER)
	{
		ms.d2sdt2 = 0;			// d2s/dt2 & d3s/dt3 not relevant
		ms.d3sdt3 = 0;
		return;
	}

	double VofSla, AofSla, JofSla;		// of limit axis at S
	VofSla = ms.vtVofS[limit.axis];
	AofSla = ms.vtAofS[limit.axis];
	JofSla = ms.vtJofS[limit.axis];

	double& Accla = ms.Accla;

	double& dsdt = ms.dsdt;
	double& d2sdt2 = ms.d2sdt2;
	double& d3sdt3 = ms.d3sdt3;

	if (fabs(VofSla) > m_Tol.vsmall)
	{
		// Calculate d2sdt2 for limit axis acceleration - Jerkla is 0 for Vel and Accel limits
		d2sdt2 = (Accla - AofSla * dsdt*dsdt) / VofSla;
		d3sdt3 = - (JofSla * dsdt*dsdt*dsdt + 3 * AofSla * d2sdt2*dsdt) / VofSla;
	}
	else if (fabs(AofSla) > m_Tol.vsmall)	// assume VofSla is 0
	{
//		TRACE0("VofS of limit axis is very small in CPathTracker::GetVelAccAtS()\n");
		d2sdt2 = - JofSla * dsdt*dsdt / (3*AofSla);	// or d2sdt2 = - JofSla * Accla / (3 * AofSla*AofSla);	requires Accla to be correct sign, determine by AofS if VofS is small!
		d3sdt3 = - (6 * JofSla * d2sdt2*dsdt*dsdt + 3 * AofSla * d2sdt2*d2sdt2) / (4 * AofSla * dsdt);
				// or d3sdt3 = - ((6 * JofSla * d2sdt2*dsdt*dsdt)/AofSla + 3 * d2sdt2*d2sdt2) / (4 * dsdt);	this has two / though!
	}
	else
	{
		if (Accla != 0)		// if 0 then must be a zero speed limit not on orig assigned poly
		{
			TRACE0("VofS and AofS of limit axis are very small in CPathTracker::GetVelAccAtS()!\n");
			ASSERT(0);
		}
		d2sdt2 = 0;
		d3sdt3 = 0;
	}

	// Acc and Jerk not correct or relevant for Corner limit
	ms.vtAcc = ms.vtAofS * (dsdt*dsdt) + ms.vtVofS * d2sdt2;
	ms.vtJerk = ms.vtJofS * (dsdt*dsdt*dsdt) + ms.vtAofS * (3 * d2sdt2*dsdt) + ms.vtVofS * d3sdt3;
}


int CPathTracker::GetMaxAccelAtVel(const SMotionState& ms, int accDir /*=1*/, SDriveLimit* pLimit /*=NULL*/)
{
/*	Handles positive acceleration (accDir=1) or braking (accDir=-1)
	Uses: ms.dsdt, ms.vtVofS, ms.vtAofS
	Sets: limit.(type, axis, pos, vel, acc, s, dsdt, d2sdt2)
	Just copies: limit.s, limit.dsdt
	Also returns limit.axis
*/
//	ms.vtAcc = ms.vtAofS * (ms.dsdt*ms.dsdt) + ms.vtVofS * ms.d2sdt2; maximise d2s/dt2
	const CVector& vtVofS = ms.vtVofS;
	const CVector& vtAofS = ms.vtAofS;
	const CVector& vtJofS = ms.vtJofS;
	double dsdtSq = ms.dsdt * ms.dsdt;
	double d2sdt2AccMax;
	char limAxis = -1;
	double limAccel, AxisAcc, d2sdt2;
	accDir = accDir >= 0 ? 1 : -1;		// control value
	double VofSvsmall = m_Tol.vsmall * vtVofS.SumAbs();
	double AofSvsmall = m_Tol.vsmall * vtAofS.SumAbs();

	for (char ax = 0; ax < 3; ax++)
	{
//		if (fabs(vtVofS[ax]) > VofSvsmall)
		if (vtVofS[ax] != 0)
		{
			AxisAcc = accDir * vtVofS[ax] > 0 ? vtAccMax[ax] : vtAccMin[ax];
			d2sdt2 = (AxisAcc - vtAofS[ax] * dsdtSq) / vtVofS[ax];
		}
		else if (fabs(vtAofS[ax]) > AofSvsmall)
		{
			AxisAcc = vtAofS[ax] > 0 ? vtAccMax[ax] : vtAccMin[ax];
			// following if (...) line taken out 18/10/04, changed/reinstated 19/10/04
			if (fabs(AxisAcc - vtAofS[ax] * dsdtSq) < VofSvsmall)	// accel due to vel is close to max accel
//			if (fabs(AxisAcc - vtAofS[ax] * dsdtSq) <= 1e-4 * AxisAcc)	// accel due to vel is close to max accel
				d2sdt2 = -vtJofS[ax] * dsdtSq / (3*vtAofS[ax]);		// set d2sdt2 so Jerk is 0, assuming vtVofS[ax] is 0
			else
				continue;	// if ds/dt does not create enough accel then acc limit is not possible
		}
		else
			continue;		// if VofS and AofS very small, there'll be no accel on axis
		if (accDir * d2sdt2 < accDir * d2sdt2AccMax || limAxis == -1)	// min of axes d2s/dt2 for accel, or max of axes d2s/dt2 for brake
		{
			d2sdt2AccMax = d2sdt2;
			limAxis = ax;
			limAccel = AxisAcc;
		}
	}
	if (limAxis != -1)		// if at least one axis vel is non zero
	{
		CVector vtAcc = vtAofS * dsdtSq + vtVofS * d2sdt2AccMax;
		if (pLimit)
		{
			pLimit->s = ms.s;
			pLimit->seg = ms.seg;
			pLimit->dsdt = ms.dsdt;
			pLimit->d2sdt2 = d2sdt2AccMax;
			pLimit->axis = limAxis;
			pLimit->acc = limAccel;
			pLimit->vel = vtVofS[limAxis] * ms.dsdt;
			pLimit->pos = ms.vtPofS[limAxis];
			pLimit->type = (accDir > 0) ? LT_AXIS_ACCELFROM : LT_AXIS_BRAKETO;
		}
	}
	else
		ASSERT(0);		// ms.vtVofS is 0
	return limAxis;
}


void CPathTracker::GetdsdtMaxCorner(SMotionState& ms)
{
// Get maximum allowable ds/dt due to velocity normal (cornering) acceleration limits
// PRE-REQUISIT: GetPolysAtS()

	CVector& vtVofS = ms.vtVofS;
	CVector vtCurve, vtAnormofS;
	double VofSMag = vtVofS.Mag();
	vtCurve = cross(vtVofS, ms.vtAofS) / (VofSMag*VofSMag*VofSMag);	// A=V^2/R -> C=A/V^2 where C=1/R
// Do only if there is SOME cornering acceleration, ie. not a straight line
	if (vtCurve.MagSq() < 1e-4)	// = 10m radius!
	{
		ms.dsdtMax = DSDTMAX_NOLIMIT;
		return;
	}
	vtAnormofS = cross(vtCurve, vtVofS) * VofSMag;	// AofS component normal to velocity
	// vtAnorm = vtAnormofS * ms.dsdt*ms.dsdt;			// A = AofS * (ds/dt)^2 + VofS * d2s/dt2
	// or vtAnorm = cross(vtCurve, vtVel) * vtVel.Mag();	// A = V^2 * C
/*	if (!vtAnormofS.Any())		
	{
		ms.dsdtMax = DSDTMAX_NOLIMIT;
		return;
	}
*/
	CBoxLimit bl;
	bl.vtA = vtAnormofS;
	bl.vtB = vtVofS;
	bl.vtBoxMax = vtAccMax;
	bl.vtBoxMin = vtAccMin;
	bl.bMaxMbBound = false;			// use min Mb bound - will be the case just after changing bounds
	bl.SolveMaxMa();					// to maximise axis accelerations
	ms.dsdtMax = sqrt(bl.Ma);
}


void CPathTracker::GetCurveBreakaway(SMotionState& ms) const
{
// PRE-REQUISIT: GetPolysAtS()
	SMotionStateExt mse;
	ms.GetdCurvedAnorm(mse);	// Get Curve and Normal Acceleration vectors independent of Acc Limit
	if (mse.CurveMag == 0)
	{
		ms.dsdtMax = DSDTMAX_NOLIMIT;		// No limit on velocity so set < 0
		ms.Abreakaway = 1;			// positive as cannot overspeed
		return;
	}	

	CBoxLimit bl;
	bl.vtA = mse.vtAnormUnit;		// Acceleration normal direction unit vector
	bl.vtB = ms.vtVofS;
	bl.vtBoxMax = vtAccMax;
	bl.vtBoxMin = vtAccMin;
	bl.bMaxMbBound = false;			// use min Mb bound - will be the case just after changing bounds
	bl.SolveMaxMa();				// to maximise velocity normal accelerations -> corner speed

	ms.vtAccBound = bl.vtBound;
// bl.vtBound = [1 0 -1] eg. according the edge limit point is derived from the direction vtAnorm and vtVofS
	GetAbreakaway(ms, mse, bl);
}


void CPathTracker::GetAccBound(SMotionState& ms, CBoxLimit& bl) const
{
// PRE-REQUISIT: GetPolysAtS(), GetCurveAnorm()
	bl.vtA = ms.vtAnormDir;		// Acceleration normal direction unit vector
	bl.vtB = ms.vtVofS;
	if (!bl.bSet)
	{
		bl.vtBoxMax = vtAccMax;
		bl.vtBoxMin = vtAccMin;
		bl.bMaxMbBound = false;			// use min Mb bound - will be the case just after changing bounds
		bl.bSet = true;
	}
	bl.SolveMaxMa();				// to maximise axis accelerations

//	ms.vtAccBound = bl.vtBound;
// bl.vtBound = [1 0 -1] eg. according the edge limit point is derived from the direction vtAnorm and vtVofS
}

void CPathTracker::GetAbreakaway(SMotionState& ms, const SMotionStateExt& mse, const CBoxLimit& bl) const
{
/*	Gets acceleration required to keep on max allowable cornering velocity
	and the foward (tangential) acceleration at max allowable cornering velocity
	'AccForMaxVel' and 'AfowardMag' respectively.
	If foward acceleration is greater than AccForMaxVel it will accelerate
	above maximum corner velocity.
	Must be less or equal to set an acceleration limit and keep within
	corner acceleration limits
	returns Abreakaway = AccForMaxVel - AfowardMag  (OK when >= 0)
*/
// PRE-REQUISIT: GetPolysAtS(ms), GetdCurvedAnorm(mse)
// Only BoxLimit .vtBound, .vtBoxMax & .vtBoxMin required!

	const double& CurveMag = mse.CurveMag;
	if (CurveMag == 0)
	{
		ms.dsdtMax = DSDTMAX_NOLIMIT;		// No limit on velocity so set < 0
		ms.Abreakaway = 1;			// positive as cannot overspeed
		return;
	}	
	const double& dCurveMag = mse.dCurveMag;
	const CVector& vtCurve = ms.vtCurve;
	const CVector& vtdCurve = mse.vtdCurve;
	const CVector& vtAnormUnit = mse.vtAnormUnit;
	const CVector& vtdAnormUnit = mse.vtdAnormUnit;

// bl.vtBound = [1 0 -1] eg. according the edge limit point is derived from the direction vtAnorm and vtVofS

	CVector vtAccFixed, vtAccFree;
	vtAccFree = 0.0;					// unit vector along edge
//	vtAccFixed = bl.vtBoundValue;	// will be vector to edge
	int axFree = -1;
	for (int ax = 0; ax < 3; ax++)
		if (bl.vtBound[ax] == 0)
		{
			vtAccFixed[ax] = 0;
			vtAccFree[ax] = 1;		// unit vector along edge
			axFree = ax;
		}
		else
			vtAccFixed[ax] = (bl.vtBound[ax] > 0) ? bl.vtBoxMax[ax] : bl.vtBoxMin[ax];

	ASSERT(axFree != -1);			// bl.vtBound should have one 0

	double AFixed_C, AFree_C, AFixed_dC, AFree_dC;
	AFixed_C  = dot(vtAccFixed, vtCurve);
	AFree_C   = dot(vtAccFree, vtCurve);
	AFixed_dC = dot(vtAccFixed, vtdCurve);
	AFree_dC  = dot(vtAccFree, vtdCurve);

	CVector vtAccTot, vtdAccTot;
	vtAccTot = vtAccFixed + vtAccFree * (-AFixed_C / AFree_C);
	vtdAccTot = vtAccFree * (((AFixed_C * AFree_dC) - (AFixed_dC * AFree_C)) / (AFree_C*AFree_C));
		// above changed to negative of matlab file in C++ routine!!

// Alternative method
/*
	vtAnorm = vtAnormUnit * bl.Ma;	// = bl.Ma * bl.vtA
	vtAfwd = ms.vtVofS * bl.Mb;		// = bl.Mb * bl.vtB
*/
// End alternative method

// find normal and foward components of acceleration
	double VofSMag = ms.vtVofS.Mag();

	double AnormMag, AfowardMag, dAnormMag;
	AnormMag = dot(vtAccTot, vtAnormUnit);
	// Anorm = vtAnormUnit * AnormMag;						// Don't need
	AfowardMag = dot(vtAccTot, ms.vtVofS) / VofSMag;
	// Afoward = vtVofS * (AfowardMag / VofSMag);	// Don't need
	dAnormMag = dot(vtAccTot, vtdAnormUnit) + dot(vtdAccTot, vtAnormUnit);

	//******** Get AdjCurve = 1/Vmax due to curve and Max Acc in the required direction *******
//	AdjCurvemag = Curvemag ./ AnormActualmag;		// use 1/VelMax instead
//	dAdjCurvemag = (AnormActualmag.*dCurvemag - dAnormActualmag.*Curvemag) ./ (AnormActualmag.^2);	// use 1/dVelMax * ?? instead

	double  dsdtMax;
	double VelMax = sqrt(AnormMag / CurveMag);	// velocity for max cornering accel, A = C * V^2 can give V = 1/0
	dsdtMax = VelMax / VofSMag;				// VelMag = VofSMag * ds/dt
			// This is absolute max ds/dt with curve accel on limits
	double dVelMax = (dAnormMag * CurveMag - AnormMag * dCurveMag) / (2 * CurveMag*CurveMag * VelMax);
	double AccForMaxVel = dVelMax * dsdtMax;	// acceleration required to keep on max allowable cornering velocity
	ms.Abreakaway = AccForMaxVel - AfowardMag;	// if AfowardMag is greater than AccForMaxVel it will accel past maximum permitable velocity.
	ms.dsdtMax = dsdtMax;	// don't alter ms.dsdt, should remain limit calculated value
}


bool CPathTracker::FindSAtPos(SMotionState& ms, double Pos, int ax, int signVel /*= 0*/)
{
//	Finds s of segment at given position of limit axis, starting from ms.s
	double& PofSla = ms.vtPofS[ax];
	double& VofSla = ms.vtVofS[ax];
	double& AofSla = ms.vtAofS[ax];
	double dPos, ds, b24ac;

	if (signVel == FSAP_SIGNFROMCURRLIMIT)		// 0 is default, 2 to use m_CurrLimit, -1 or 1 to specify sign
		switch (m_CurrLimit.type)
		{
		case LT_AXIS_ACCELFROM: signVel = m_CurrLimit.acc > 0 ?  1 : -1; break;
		case LT_AXIS_BRAKEFROM: signVel = m_CurrLimit.acc > 0 ? -1 :  1; break;
		case LT_AXIS_VELOCITY:  signVel = m_CurrLimit.vel > 0 ?  1 : -1; break;
		default: signVel = 0; ASSERT(0);
		}
	else if (signVel == FSAP_DEFAULT)
		if (fabs(VofSla) >= m_Tol.vsmall)
			signVel = VofSla > 0 ? 1 : -1;
	else
		signVel = signVel > 0 ? 1 : -1;

	double posTol = (fabs(Pos) + 1) * m_Tol.pos;
	ds = 1;			// start value large to avoid small ds method
	int iIterations = 0;
	for (;;)
	{
		GetPolysAtS(ms);
		dPos = Pos - PofSla;
		if (fabs(dPos) <= posTol && iIterations >= 2)
			break;
		if (dPos == 0)
			break;
		if (fabs(AofSla) < 1e-3 || fabs(ds) < 1e-8) // better resolution for small changes! && fabs(AofSla) < fabs(VofSla))
			ds = dPos / VofSla;			// Take tangent at current Pos if curve is negligable
		else		// solve a.s^2 + b.s + c = 0  where:  c = -dPos, b = VofSla, 2a = AofSla
		{			// s = (-b +/-sqrt(b^2 - 4ac)) / 2a
			b24ac = VofSla*VofSla + 2 * AofSla * dPos;	// b^2-4ac
			if (b24ac >= 0)
				if (signVel != 0)		// check if sign of required velocity is given
					ds = (-VofSla + signVel * sqrt(b24ac)) / AofSla;	// solu = (-Vi +/-sqrt(Vi^2 - 2*Ai*Pi)) / Ai
				else
					ds = (-VofSla + ((AofSla>0) ? 1:-1) * sqrt(b24ac)) / AofSla;	// solu = (-Vi/Ai + sqrt(Vi^2 - 2*Ai*Pi) / abs(Ai) (larger ds)
			else				// just go to closest point (quadratic minimum) -> s = -b / 2a
				ds = -VofSla / AofSla;			// go to quadratic max/min
		}
// Add sqrt() with the sign of velocity at required position
// Hence generally same sign as VofSla unless VofSla changes sign between
// initial position and required position
		double sNew = ms.s + ds;
		if (sNew == ms.s)		// can't do any better!
			break;
//		ASSERT(sNew > -1 && sNew < 2);
		if (sNew < 0) sNew = 0;
		if (sNew > 1) sNew = 1;
		ms.s = sNew;
		iIterations++;
		ASSERT(iIterations < 20);
	}
	return true;
}


double CPathTracker::GetLimitPosAtLimitTime(double tLimit)
{
	// calc next pos depending on limit type
	double PosLA;			// position of limit axis
	switch (m_CurrLimit.type)
	{
	case LT_AXIS_ACCELFROM:
	case LT_AXIS_BRAKEFROM:
		PosLA = m_CurrLimit.pos + tLimit * m_CurrLimit.vel + (m_CurrLimit.acc/2 * tLimit*tLimit);	// dPos = dt*Vi + A/2*dt^2
		break;
	case LT_AXIS_VELOCITY:
		PosLA = m_CurrLimit.pos + tLimit * m_CurrLimit.vel;
		break;
	case LT_STOP:
		PosLA = m_CurrLimit.pos;
		break;
	default:
		PosLA = 0;
		ASSERT(0);	// bad limit type
	}
	return PosLA;
}

double CPathTracker::GetLimitTimeUsingdsdtAt(const SMotionState& ms)
{	// needs PofS & VofS of limit axis, and dsdt
	SDriveLimit& cl = m_CurrLimit;
	double PosLA, VelLA;
	double tLimit;
	switch (cl.type)
	{
	case LT_AXIS_ACCELFROM:
	case LT_AXIS_BRAKEFROM:
		VelLA = ms.vtVofS[cl.axis] * ms.dsdt;	// if ds/dt is avaliable at ms
		tLimit = (VelLA - cl.vel) / cl.acc;
		break;
	case LT_AXIS_VELOCITY:
		PosLA = ms.vtPofS[cl.axis];
		if (cl.vel == 0)
		{
			tLimit = 0;
			ASSERT(PosLA == cl.pos);
		}
		else
			tLimit = (PosLA - cl.pos) / cl.vel;
		break;
	case LT_STOP:
		tLimit = 0;
		break;
	case LT_CORNER:
		tLimit = cl.tDuration;		// it has been summed incrementally in CPathSpeed::CheckAccelLimits() 
		break;
	default:
		tLimit = 0;
		ASSERT(0);	// bad limit type
	}
	if (tLimit < 0)
		if (tLimit >= -m_Tol.time)
			tLimit = 0;
		else
			ASSERT(tLimit >= 0);
	return tLimit;
}

double CPathTracker::GetLimitTimeAt(const SMotionState& ms)	// needs only PofS of limit axis
{
	SDriveLimit& cl = m_CurrLimit;
	double PosLA;
	double tLimit;
	switch (cl.type)
	{
	case LT_AXIS_ACCELFROM:
	case LT_AXIS_BRAKEFROM:
		PosLA = ms.vtPofS[cl.axis];
		double b24ac;
		b24ac = cl.vel*cl.vel + 2 * cl.acc * (PosLA - cl.pos);	// b^2-4ac
		if (b24ac <= 0)
		{
			if (b24ac < -m_Tol.vel)			// was m_Tol.vsmall*m_Tol.vsmall) but this is too small for errors
				ASSERT(0);		// b24ac < 0
			b24ac = 0;
		}
		else
			b24ac = sqrt(b24ac);
		//		if Accelerating time is the later, if Braking time is the earlier
		if (cl.type == LT_AXIS_ACCELFROM)
			{ if (cl.acc < 0) b24ac = -b24ac; }
		else		// if (cl.type == LT_AXIS_BRAKEFROM)
			{ if (cl.acc > 0) b24ac = -b24ac; }
		tLimit = (-cl.vel + b24ac) / cl.acc;	// solu = (-Vi +/-sqrt(Vi^2 - 2*Ai*Pi)) / Ai
		break;
	case LT_AXIS_VELOCITY:
		PosLA = ms.vtPofS[cl.axis];
		if (cl.vel == 0)
		{
			tLimit = 0;
			ASSERT(PosLA == cl.pos);
		}
		else
			tLimit = (PosLA - cl.pos) / cl.vel;
		break;
	case LT_STOP:
		tLimit = 0;
		break;
	default:
		tLimit = 0;
		ASSERT(0);	// bad limit type
	}
	return tLimit;
}
double CPathTracker::GetLimitTimeAt(const SMotionState& ms, SDriveLimit& cl)	// needs only PofS of limit axis
{
	double PosLA;
	double tLimit;
	switch (cl.type)
	{
	case LT_AXIS_ACCELFROM:
	case LT_AXIS_BRAKEFROM:
		PosLA = ms.vtPofS[cl.axis];
		double b24ac;
		b24ac = cl.vel*cl.vel + 2 * cl.acc * (PosLA - cl.pos);	// b^2-4ac
		if (b24ac <= 0)
		{
			if (b24ac < -m_Tol.vel)			// was m_Tol.vsmall*m_Tol.vsmall) but this is too small for errors
				ASSERT(0);		// b24ac < 0
			b24ac = 0;
		}
		else
			b24ac = sqrt(b24ac);
		//		if Accelerating time is the later, if Braking time is the earlier
		if (cl.type == LT_AXIS_ACCELFROM)
			{ if (cl.acc < 0) b24ac = -b24ac; }
		else		// if (cl.type == LT_AXIS_BRAKEFROM)
			{ if (cl.acc > 0) b24ac = -b24ac; }
		tLimit = (-cl.vel + b24ac) / cl.acc;	// solu = (-Vi +/-sqrt(Vi^2 - 2*Ai*Pi)) / Ai
		break;
	case LT_AXIS_VELOCITY:
		PosLA = ms.vtPofS[cl.axis];
		if (cl.vel == 0)
		{
			tLimit = 0;
			ASSERT(PosLA == cl.pos);
		}
		else
			tLimit = (PosLA - cl.pos) / cl.vel;
		break;
	case LT_STOP:
		tLimit = 0;
		break;
	default:
		tLimit = 0;
		ASSERT(0);	// bad limit type
	}
	return tLimit;
}

double CPathTracker::GetEndSegTime()
{
	SMotionState ms;
	ms.s = 1;
	ms.seg = GetSegNumber();
	ASSERT(m_NextLimit >= ms);
	GetPolysAtS(ms);		// only need PofS of limit axis!
	double t = GetLimitTimeAt(ms);		// dsdt not known
	ASSERT(t >= 0);
	return t;
}

double CPathTracker::GetEndLimitTime()		// uses m_NextLimit
{
	SMotionState ms;
	ms.s = m_NextLimit.s;
	ms.seg = m_NextLimit.seg;
	ASSERT(m_NextLimit.seg == GetSegNumber());	// need correct polys
	GetPolysAtS(ms);		// only need PofS & VofS of limit axis!
	ms.dsdt = m_NextLimit.dsdt;
	double t = GetLimitTimeUsingdsdtAt(ms);
	ASSERT(t >= 0);
	return t;
}











