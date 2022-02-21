// PathTracker.h: interface for the CPathTracker class.
//
/* CPathTracker object contains all the common path parameter data and 
	functions used to calculate path parametrics
	Intended as the base class of path calculating objects
*/
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHTRACKER_H__DBA33882_4B7D_11D5_8C1E_B947AB702C2F__INCLUDED_)
#define AFX_PATHTRACKER_H__DBA33882_4B7D_11D5_8C1E_B947AB702C2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "PathDataObjects.h"
#include "BoxLimit.h"
#include "PathDoc.h"

#include "ListArray.h"


typedef TListArray<SDriveLimit, SDriveLimit&> CLimitList;
typedef TListArray<SDriveProperties, SDriveProperties&> CDrivePropertiesList;

//////////////////////////////////////////
// CPathTracker
//////////////////////////////////////////

class CPathTracker  
{
public:
	CPathTracker();
	virtual ~CPathTracker();

	void SetPathDoc(CPathDoc* pPathDoc);
	CPathDoc* GetPathDoc() { return m_pPathDoc; }
	bool GotPath() { return m_pPathDoc != NULL; }
	void SetLimitList(CLimitList* pLimitList) { m_pLimitList = pLimitList; }

	void GetPathStartPos(CVector& vtPos) { m_pPathDoc->GetStartPos(vtPos); }
	bool AtEndOfPath() { return m_bAtEndOfPath; }
	int GetNumPathSegments() { return m_pPathDoc->GetNumSegments(); }
	int GetSegNumber() { return m_Seg.seg; }

//	void SetPosMachineOfPathStart(CVector& vtPosMachPathStart);
//	void GetPosMachineOfPathOrigin(CVector& vt) { vt = m_vtPosMachineOfPathOrigin; }




public:
	enum PATHSPEED_ERRORS
	{
		ERROR_BadLimitType = 1,
		ERROR_BrakeThruZeroVel,
		ERROR_BoundChangeTwoAxis,

		FATAL_ERROR,
		FATAL_PosUnreachableWithAcc,

		ERROR_MAX
	};

protected:
	enum CHANGE_TYPE
	{
		CNG_SEG = 0x01,		// flags
		CNG_LIMIT = 0x02,
		CNG_DRIVEPROPS = 0x04,
		CNG_INITIAL = 0x08,
	};

	enum
	{
		DSDTMAX_NOLIMIT = -1,
		MAX_SEG = INT_MAX,
	};
	#define DSDT_ERRORVERYLARGE  1e8

	enum
	{
		DIR_BRAKE = -1,
		DIR_ACCEL = 1,
	};


// data members
protected:
	CPathDoc* m_pPathDoc;
//	CVector m_vtPosMachineOfPathOrigin;	// abs machine position of path origin - used for pos correction

	SSegPolys m_Seg;		// contains SPathProperties
	CMatrix m_vtPowerS;

	STolerance m_Tol;
	bool m_bVofSlaChangedSign;		// set if current vel
	int m_nVofSlaChangedSign;		// inc each time encounted

	// drive properties
	SDriveProperties m_DrvPropMachine;	// max for machine driving parameters
	SDriveProperties m_DrvPropCurr;		// current driving parameters

	// drive limits
	SDriveLimit m_CurrLimit;		// current limit being used to calc vel etc.
	SDriveLimit m_NextLimit;		// Next limit in list
	SDriveLimit m_Limit;				// new limit being found

	CLimitList* m_pLimitList;
	int m_posLL;

	// path scaning extents
	int m_StartSeg;			// segment to start limit calculations from
	int m_EndSeg;				// segment to end limit calculations on
	bool m_bAtEndOfPath;




	int m_nStepChange;		// Change Type flags

// common data members without 'm_'
	SMotionState ms0, ms1;		// motion at previous(ms0) and current(ms1) steps

	CVector vtVelMax;			// = m_DrvProp.posVel;
	CVector vtVelMaxTol;		// = m_DrvProp.posVel + m_Tol.vel;
	CVector vtVelMin;			// = m_DrvProp.negVel;
	CVector vtVelMinTol;		// = m_DrvProp.negVel - m_Tol.vel;

	CVector vtAccMax;			// = m_DrvProp.posAcc;
	CVector vtAccMaxTol;		// = m_DrvProp.posAcc + m_Tol.acc;
	CVector vtAccMin;			// = m_DrvProp.negAcc;
	CVector vtAccMinTol;		// = m_DrvProp.negAcc - m_Tol.acc;


// member functions
protected:
	virtual void Init();
	void SetDefaults();
	void SetToStart();
	void GetInitPathProps();


	void SetFeedRate(double feedRate);
	void SetTool(int tool);

	void GetMachineDriveProp(SDriveProperties& dp);
	void SetPathProps(const SPathProperties& pp);
	void SetDrivePropsFromPathProps();
	void SetDriveProps(const SDriveProperties& dp);
	void SetLimitsFrom(const SDriveProperties& dp);
	CPathLoc& GetPathPropChangeLoc() { return m_Seg.locPathPropChange; }

	bool GetInitSegPolys() { return m_pPathDoc->GetInitSegPolys(m_Seg); }
	bool GetFinalSegPolys() { return m_pPathDoc->GetFinalSegPolys(m_Seg); }
	bool GetNextSegPolys() { 	return m_pPathDoc->GetNextSegPolys(m_Seg); }	// there may be some drive prop change locations in this segment
	bool GetPrevSegPolys() { return m_pPathDoc->GetPrevSegPolys(m_Seg); }
	bool GetSegPolysAt(int seg) { return m_pPathDoc->GetSegPolysAt(m_Seg, seg); }
//	bool MoveToSeg(SEGREF seg) { return m_pPathDoc->MoveToSeg(m_Seg, seg); }
	bool MoveToLocForFoward(CPathLoc& loc) { return m_pPathDoc->MoveToLocForFoward(m_Seg, loc); }
	bool MoveToLocForBackward(CPathLoc& loc) { return m_pPathDoc->MoveToLocForBackward(m_Seg, loc); }

	void FindLocFirstSegPathPropChange() { m_pPathDoc->FindLocFirstSegPathPropChange(m_Seg); }
	void FindLocLastSegPathPropChange() { m_pPathDoc->FindLocLastSegPathPropChange(m_Seg); }
	void GetNextPathProp();
	void GetPrevPathProp();


	void GetPolysAtS(SMotionState& ms);
	void GetPolysArcAtS(SMotionState& ms);
	double GetSegChordLength();
	double GetSegChordLengthBetween(double s0, double s1);
	double GetSegLength();
	double GetSegLengthBetween(double s0, double s1);

	void GetVelAtSNoPoly(SMotionState& ms, SDriveLimit& limit);
	void GetVelAtSNoPoly(SMotionState& ms) { GetVelAtSNoPoly(ms, m_CurrLimit); }
	void GetVelAtS(SMotionState& ms, SDriveLimit& limit) { GetPolysAtS(ms); GetVelAtSNoPoly(ms, limit); }
	void GetVelAtS(SMotionState& ms) { GetPolysAtS(ms); GetVelAtSNoPoly(ms, m_CurrLimit); }
	void GetVelAccAtSNoPolyOld(SMotionState& ms, SDriveLimit& limit);		// not used
	void GetVelAccAtSNoPoly(SMotionState& ms, SDriveLimit& limit);
	void GetVelAccAtSNoPoly(SMotionState& ms) { GetVelAccAtSNoPoly(ms, m_CurrLimit); }
	void GetVelAccAtS(SMotionState& ms, SDriveLimit& limit) { GetPolysAtS(ms); GetVelAccAtSNoPoly(ms, limit); }
	void GetVelAccAtS(SMotionState& ms) { GetPolysAtS(ms); GetVelAccAtSNoPoly(ms, m_CurrLimit); }

	int GetMaxAccelAtVel(const SMotionState& ms, int accDir = 1, SDriveLimit* pLimit = NULL);
	void GetVelAccAtState(SMotionState& ms);
	void GetdsdtMaxCorner(SMotionState& ms);
	double GetdsdtOver(const SMotionState& ms) { return (ms.dsdtMax == DSDTMAX_NOLIMIT) ? -1 : ms.dsdt - ms.dsdtMax; }		// return something negative if no max

	void GetAccBound(SMotionState& ms, CBoxLimit& bl) const;
	void GetCurveBreakaway(SMotionState& ms) const;	// sets ms.Abreakaway
	void GetAbreakaway(SMotionState& ms, const SMotionStateExt& mse, const CBoxLimit& bl) const;

	enum { FSAP_NEG = -1, FSAP_DEFAULT = 0, FSAP_POS = 1, FSAP_SIGNFROMCURRLIMIT = 2 };
	bool FindSAtPos(SMotionState& ms, double Pos, int ax, int signVel = FSAP_DEFAULT);

	// for use after limits are found
	double GetLimitPosAtLimitTime(double tLimit);
	double GetLimitTimeUsingdsdtAt(const SMotionState& ms);	// needs PofS & VofS of limit axis, and dsdt
	double GetLimitTimeAt(const SMotionState& ms);				// needs only PofS of limit axis
	double GetLimitTimeAt(const SMotionState& ms, SDriveLimit& cl);	// needs only PofS of limit axis

	double GetEndSegTime();
	double GetEndLimitTime();



};

#endif // !defined(AFX_PATHTRACKER_H__DBA33882_4B7D_11D5_8C1E_B947AB702C2F__INCLUDED_)
