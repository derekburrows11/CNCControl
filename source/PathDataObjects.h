// PathDataObjects.h: interface for the PathDataObjects class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHDATAOBJECTS_H__DBA33883_4B7D_11D5_8C1E_B947AB702C2F__INCLUDED_)
#define AFX_PATHDATAOBJECTS_H__DBA33883_4B7D_11D5_8C1E_B947AB702C2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxtempl.h>

#include "Matrix.h"



/////////////////////////////




typedef long NODEREF;		// node reference type
typedef long SEGREF;			// segment reference type

#define NODEREF_MAX	LONG_MAX
#define SEGREF_MAX	LONG_MAX

#define INVALIDREF	(-1)



class CPathLoc
{
public:
	SEGREF seg;
	double s;

	void SetPreStart() { seg = (SEGREF)-1; s = -1; }
	void SetPostEnd() { seg = SEGREF_MAX; s = 2; }
	bool IsValid() { return seg != -1 && seg != SEGREF_MAX; }
	double Loc() { return seg + s; }
	CPathLoc& PathLoc() { return *this; }

	bool operator==(CPathLoc& rhs)
		{ return seg == rhs.seg && s == rhs.s; }
	bool operator!=(CPathLoc& rhs)
		{ return seg != rhs.seg || s != rhs.s; }
	bool operator<(CPathLoc& rhs)
		{ return seg < rhs.seg || (seg == rhs.seg && s < rhs.s); }
	bool operator<=(CPathLoc& rhs)
		{ return seg < rhs.seg || (seg == rhs.seg && s <= rhs.s); }
	bool operator>(CPathLoc& rhs)
		{ return seg > rhs.seg || (seg == rhs.seg && s > rhs.s); }
	bool operator>=(CPathLoc& rhs)
		{ return seg > rhs.seg || (seg == rhs.seg && s >= rhs.s); }
};


//struct SMotionState;		// foward reference

struct SMotionStateExt
{
	double CurveMag, dCurveMag;
	CVector vtdCurve;
	CVector vtAnormUnit, vtdAnormUnit;
};


struct SMotionState : public CPathLoc
{
public:
	double t;			// time on limit
	double dsdt, d2sdt2, d3sdt3;
	double dsdtMax;	// maximum ds/dt due to cornering accelerations: <0 (-1) = no limit (straight)
	CVector vtPofS, vtVofS, vtAofS, vtJofS;
	CVector vtPos, vtVel, vtAcc, vtJerk;
	double Vella, Accla;			// Vel and Acc of limit axis
	CVector vtCurve, vtAnormDir;
	double Abreakaway;
	TVector<char> vtAccBound;

public:
	void GetAnormDirection(CVector& vtAnormDirection) const;
	void GetCurveAnorm();
	void GetdCurvedAnorm(SMotionStateExt& mse);
	void SetAccBoundMembers(const SMotionState& ms);
	void operator=(CPathLoc& rhs) { *static_cast<CPathLoc*>(this) = rhs; }
};

#pragma
struct SMotionStateBasic : public CPathLoc
{
	double t;
	double dsdt;
	union
	{
#pragma warning(disable : 4201)					// nonstandard extension used : nameless struct/union
		struct { CVector vtPVAJ[4]; };			// [0]pos, [1]vel, [2]acc, [3]jerk (struct needed to compile!)
		struct { CVector vtPos, vtVel, vtAcc, vtJerk; };
#pragma warning(default : 4201)
	};
	int iID;		// to store state type (pre-step, post-step etc.)

	void StoreMS(const SMotionState& ms, int iID = 0);
};

typedef CArray<SMotionStateBasic, SMotionStateBasic&> CMotionStateBasicArray;
ostream& operator<<(ostream& os, const CMotionStateBasicArray& motArray);






enum LIMIT_TYPE
{
	LT_NONE = 0,

	// normal limits
	LT_MIN_NORMAL,
	// individual axis limits
	LT_AXIS_VELOCITY,
	LT_AXIS_ACCELFROM,
	LT_AXIS_BRAKEFROM,

	LT_AXIS_JERKFROM,		// not used yet
	LT_AXIS_JERKTO,		// not used yet

	// non axis limits
	LT_SPEED,
	LT_STOP,
	LT_MAX_NORMAL,

	// intermediate limits
	LT_AXIS_ACCELTO,		// added 15/7/04
	LT_AXIS_BRAKETO,

	LT_CORNER,
	LT_STARTBRAKE,

	LT_ENDCALC,		// end of calculated limit region

	LT_MAX
};
enum LIMIT_CHANGE
{
	LC_VEL_CONTINUOUS = 0,		// For velocity continuous
	LC_VEL_REDUCED = 0x10,		// For velocity reduced
	LC_VEL_ZEROED,					// will have velocity reduced flag
};

struct SDriveLimit : public CPathLoc
{
	double dsdt;			// ds/dt
	double d2sdt2;			// d2s/dt2
	double pos, vel, acc;
	double tDuration;		// to be added to store limit duration
	char axis;
	enum LIMIT_TYPE type;	// LIMIT_TYPE
	BYTE change;				// LIMIT_CHANGE

//	static const char* const szType[];		// set to limit names
	static const char** const szType;		// set to limit names
	void operator=(SMotionState& ms);
	char* GetHeading() const;
	bool SameTypeAs(const SDriveLimit& other) const;
};
ostream& operator<<(ostream& os, const SDriveLimit& lim);

//extern const char* const g_szLimitType[];		// set to limit names

inline void SDriveLimit::operator=(SMotionState& ms)
{
//	seg = ms.seg;		// don't do - ms.seg not set for a local ms
	s = ms.s;
	dsdt = ms.dsdt;
	d2sdt2 = ms.d2sdt2;
}

inline bool SDriveLimit::SameTypeAs(const SDriveLimit& other) const
{
	return type == other.type && axis == other.axis;
}


//////////////////////


struct SPathProperties
{
	double feedRate;
	int iTool;
	double load;		// not used yet
};

struct SDriveProperties
{
	CVector vtVel;
	CVector vtAcc;
	CVector vtDeAcc;
	double xySpeed, xyzSpeed;
	double feedRate;
	double load;		// not used yet

	int LimitTo(const SDriveProperties& dpLimit);	// returns number of values altered to limit
};

struct SSegPolys
{
	SEGREF seg;				// segment number
	NODEREF nrInit;		// reference of initial node of segment
	NODEREF nrFinal;		// reference of final node of segment
	CMatrix pos;			// SetSize(4,3)
	CMatrix vel;			// SetSize(3,3)
	CMatrix acc;			// SetSize(2,3)
	CMatrix jerk;			// SetSize(1,3)
	double length;			// segment length

	SPathProperties pathProps;

	CPathLoc locPathPropChange;
	NODEREF nrPathPropChange;
	int numPathPropChanges;

	void GetPolysAtS(SMotionState& ms);
	void GetPVofSAtS(SMotionState& ms);
};

struct SPathLocInfo : public CPathLoc
{
	NODEREF nrSegInit;		// reference of initial node of segment
	NODEREF nrSegFinal;		// reference of final node of segment
	SMotionStateBasic ms;	// need only PofS, VofS, AofS
	SPathProperties pathProps;
	SDriveProperties driveProps;	// needed??
};

struct SPathSection
{
	double time;			// current stage time
	double length;			// current stage length
	double timeMin;		// minimum time to cover for section
	SPathLocInfo start;
	SPathLocInfo end;
};




struct STolerance
{
	double pos, vel, acc, jerk;
	double dsdt;		// used in velocity matching
	double time;
	double def;		// default tolerance used for general functions
	double vsmall;
	double accStep;		// Accel change to be considered a step change
};


struct SContBufferInfo
{
	int PathSeg;
	int AccSeg;
	double Time;

	CPathLoc Loc;
	SDriveLimit CurrLimit;
};

struct SContBufferSpan
{
	SContBufferInfo Init;
	SContBufferInfo Final;
};




#endif // !defined(AFX_PATHDATAOBJECTS_H__DBA33883_4B7D_11D5_8C1E_B947AB702C2F__INCLUDED_)

