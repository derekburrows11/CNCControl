// PathDataObjects.cpp: implementation of the PathDataObjects class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "cnccontrol.h"

#include "PathDataObjects.h"
#include <iomanip.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




int SDriveProperties::LimitTo(const SDriveProperties& dpLimit)		// returns number of values altered to limit
{
	int iChanged = 0;
	vtVel.Min(dpLimit.vtVel);
	if (xySpeed > dpLimit.xySpeed) { xySpeed = dpLimit.xySpeed; iChanged++; }
	if (xyzSpeed > dpLimit.xyzSpeed) { xyzSpeed = dpLimit.xyzSpeed; iChanged++; }
	if (feedRate > dpLimit.feedRate) { feedRate = dpLimit.feedRate; iChanged++; }
	vtAcc.Min(dpLimit.vtAcc);
	vtDeAcc.Min(dpLimit.vtDeAcc);
	return iChanged;
}


//////////////////////////////////////////////////////////////////////
// struct SMotionState (change to class)
//////////////////////////////////////////////////////////////////////

void SMotionState::GetAnormDirection(CVector& vtAnormDirection) const
{
// PRE-REQUISIT: GetPolysAtS()
	vtAnormDirection = cross(cross(vtVofS, vtAofS), vtVofS);
}

void SMotionState::GetCurveAnorm()
{
// Get Curve and Normal Acceleration vectors	of polynomials at s given vtVofS, vtAofS
// PRE-REQUISIT: GetPolysAtS()
	double VofSMagSq = vtVofS.MagSq();
	vtCurve = cross(vtVofS, vtAofS) / (VofSMagSq * sqrt(VofSMagSq));
	vtAnormDir = cross(vtCurve, vtVofS);
}

void SMotionState::GetdCurvedAnorm(SMotionStateExt& mse)
{
	// Doesn't need to be a member of class CPathSpeed!!
	// Incorperate into GetCurveBreakaway() if not used anywhere else
/* Get Curve and Normal Acceleration vectors and their derivatives
	of polynomials at ms.s
	given vtVofS, vtAofS & vtJofS
*/
// PRE-REQUISIT: GetPolysAtS()

	double& CurveMag = mse.CurveMag;
	double& dCurveMag = mse.dCurveMag;
	CVector& vtdCurve = mse.vtdCurve;
	CVector& vtAnormUnit = mse.vtAnormUnit;
	CVector& vtdAnormUnit = mse.vtdAnormUnit;

	double VofSMag2 = vtVofS.MagSq();
	double VofSMag = sqrt(VofSMag2);
	double VofSMag3 = VofSMag2 * VofSMag;

	vtCurve = cross(vtVofS, vtAofS) / VofSMag3;
	CurveMag = vtCurve.Mag();

	vtAnormDir = cross(vtCurve, vtVofS);
	double AnormMag = vtAnormDir.Mag();
	vtAnormUnit = vtAnormDir / AnormMag;	// can be /0

	vtdCurve = (cross(vtVofS, vtJofS) / VofSMag3)
				- (vtCurve * (3 * dot(vtVofS, vtAofS) / VofSMag2));
	if (CurveMag != 0)
		dCurveMag = dot(vtCurve, vtdCurve) / CurveMag;		// proper derivative
	else
		dCurveMag = vtdCurve.Mag();

	CVector vtdAnormDir;
	vtdAnormDir = cross(vtdCurve, vtVofS) + cross(vtCurve, vtAofS);
	vtdAnormUnit = (vtdAnormDir - (vtAnormUnit * dot(vtdAnormDir, vtAnormUnit))) / AnormMag;	// can be /0
	// or   vtdAnormUnit = cross(cross(vtAnormUnit, vtdAnormDir / AnormMag), vtAnormUnit);
}

void SMotionState::SetAccBoundMembers(const SMotionState& ms)
{
	s = ms.s;		// set only members which used to find any step changes in Bound
	seg = ms.seg;
	vtPofS = ms.vtPofS;
	vtVofS = ms.vtVofS;
	vtAofS = ms.vtAofS;
	vtJofS = ms.vtJofS;
	dsdtMax = ms.dsdtMax;
	Abreakaway = ms.Abreakaway;
	vtAnormDir = ms.vtAnormDir;
	vtAccBound = ms.vtAccBound;
}


//////////////////////////////////////////////////////////////////////
// struct SMotionStateBasic
//////////////////////////////////////////////////////////////////////

void SMotionStateBasic::StoreMS(const SMotionState& ms, int id /*= 0*/)
{
	// Store ms states,  was in CPathMove
//	t = tStartLimit + GetLimitTimeUsingdsdtAt(ms);	// Get time
	t = ms.t;	//GetPathTime();
	seg = ms.seg;
	s = ms.s;
	dsdt = ms.dsdt;
	vtPos = ms.vtPos;
	vtVel = ms.vtVel;
	vtAcc = ms.vtAcc;
	vtJerk = ms.vtJerk;
	iID = id;
}

ostream& operator<<(ostream& os, const CMotionStateBasicArray& motArray)
{
	int numPts = motArray.GetSize();
	char* szHeading = "     time       seg+s       ds/dt          Xpos       Ypos       Zpos          Xvel       Yvel       Zvel          Xacc       Yacc       Zacc\n";

	os << endl;
	os.setf(ios::fixed | ios::showpoint);
	for (int i = 0; i < numPts; i++)
	{
		if (i % 10 == 0)
			os << szHeading;
		const SMotionStateBasic& ms = const_cast<CMotionStateBasicArray&>(motArray).ElementAt(i);
		if (ms.iID == 0)
			os << " ";
		else if (ms.iID == -1)
			os << "-";
		else if (ms.iID == 1)
			os << "+";
		os.precision(5);
		os << setw(10) << ms.t;
		os << ' ' << setw(4) << ms.seg << '+';
		os << setw(7) << ms.s;
		os << setw(10) << ms.dsdt;
		os.precision(4);
		os << "    " << ms.vtPos;
		os << "    " << ms.vtVel;
		os << "    " << ms.vtAcc;
		os << endl;
	}

// Plot Values
/*
	CMatrix mxVal(numPts);		// vector iPtIdx long
	CMatrix mxT(numPts);
	for (i = 0; i < numPts; i++)
		mxT[i] = motArray[i].t;
	for (int pva = 0; pva < 3; pva++)
		for (int ax = 0; ax < 3; ax++)
		{
			for (i = 0; i < numPts; i++)
				mxVal[i] = motArray[i].vtPVA[pva][ax];
			Plot(mxT.GetArray(), mxVal.GetArray(), numPts, GPS_LINE);
		}
*/
	return os;
}


/////////////////////////////////
// SSegPolys
/////////////////////////////////

void SSegPolys::GetPolysAtS(SMotionState& ms)
{
// PRE-REQUISIT: set ms.s ONLY
	double arPowerS[4];
	CMatrixWrap vtPowerS(arPowerS, 1, 4);		// uses row vector
	vtPowerS.Power3Vector(ms.s);
	pos.SolvePolyAt(vtPowerS, ms.vtPofS);
	vel.SolvePolyAt(vtPowerS, ms.vtVofS);
	acc.SolvePolyAt(vtPowerS, ms.vtAofS);
	jerk.SolvePolyAt(vtPowerS, ms.vtJofS);
}

void SSegPolys::GetPVofSAtS(SMotionState& ms)
{
// PRE-REQUISIT: set ms.s ONLY
	double arPowerS[4];
	CMatrixWrap vtPowerS(arPowerS, 1, 4);		// uses row vector
	vtPowerS.Power3Vector(ms.s);
	pos.SolvePolyAt(vtPowerS, ms.vtPofS);
	vel.SolvePolyAt(vtPowerS, ms.vtVofS);
//	acc.SolvePolyAt(vtPowerS, ms.vtAofS);
//	jerk.SolvePolyAt(vtPowerS, ms.vtJofS);
}



/////////////////////////////////
// SDriveLimit
/////////////////////////////////

// setup limit descriptions
/*
const char* const SDriveLimit::szType[] = {
			"None",
			"Accel",
			"Velocity",
			"Brake",
			"Brake To",
			"Corner",
			"Start Brake", };
*/
/*
const char* const g_szLimitType[] = {
			"None",
			"Accel",
			"Velocity",
			"Brake",
			"Brake To",
			"Corner",
			"Start Brake", };
*/
struct SLimitTypeNames
{
	const char* arNames[LT_MAX];
	SLimitTypeNames();
};
SLimitTypeNames g_LimitNames;
const char** const SDriveLimit::szType = g_LimitNames.arNames;

//const char* const g_szLimitType[LT_MAX] = { NULL };
//	g_szLimitType[LT_NONE]					= "None";

SLimitTypeNames::SLimitTypeNames()
{
	for (int i = 0; i < LT_MAX; i++)
		arNames[i] = "name not set!";
	arNames[LT_NONE]				= "None";
	arNames[LT_AXIS_ACCELFROM]	= "Accel";
	arNames[LT_AXIS_VELOCITY]	= "Velocity";
	arNames[LT_AXIS_BRAKEFROM]	= "Brake";
	arNames[LT_AXIS_BRAKETO]	= "Brake To";
	arNames[LT_SPEED]				= "Speed";
	arNames[LT_STOP]				= "Stop";
	arNames[LT_CORNER]			= "Corner";
	arNames[LT_STARTBRAKE]		= "Start Brake";
	arNames[LT_ENDCALC]			= "End Calc";
	arNames[LT_MIN_NORMAL]		= "Min of Normal Limits";
	arNames[LT_MAX_NORMAL]		= "Max of Normal Limits";
}

/*
const char* const g_szLimitType[]	= { NULL };
	szLimitName[LT_NONE]					= "None";
	szLimitName[LT_AXIS_ACCELFROM]	= "Accel";
	szLimitName[LT_AXIS_VELOCITY]		= "Velocity";
	szLimitName[LT_AXIS_BRAKEFROM]	= "Brake";
	szLimitName[LT_AXIS_BRAKETO]		= "Brake To";
	szLimitName[LT_SPEED]				= "Speed";
	szLimitName[LT_STOP]					= "Stop";
	szLimitName[LT_CORNER]				= "Corner";
	szLimitName[LT_STARTBRAKE]			= "Start Brake";
	szLimitName[LT_MIN_NORMAL]			= "Min of Normal Limits";
	szLimitName[LT_MAX_NORMAL]			= "Max of Normal Limits";
*/

char* SDriveLimit::GetHeading() const
{
	return " seg  s            ds/dt       time    type        axis      pos            vel            acc\n";
}

ostream& operator<<(ostream& os, const SDriveLimit& lim)
{
	os.setf(ios::fixed | ios::showpoint);

	os.precision(8);
	os << setw(3) << lim.seg;
	os << setw(11) << lim.s;
	os << setw(13) << lim.dsdt;
	os.precision(3);
	os << setw(10) << lim.tDuration;
	os.precision(8);
	os << setw(4) << (int)(char)lim.type;
	os.setf(ios::left, ios::adjustfield);
	os << " " << setw(8) << lim.szType[lim.type];
	os.setf(ios::right, ios::adjustfield);
	os << setw(4) << (int)(char)lim.axis;
//	os << setw(4) << (int)(char)lim.change;
	os << setw(16) << lim.pos;
	os << setw(15) << lim.vel;
	os << setw(15) << lim.acc;
	os << endl;
	return os;
}


