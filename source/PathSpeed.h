// PathSpeed.h: interface for the CPathSpeed class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHSPEED_H__26748121_0E11_11D5_8C1E_D5461D71E42C__INCLUDED_)
#define AFX_PATHSPEED_H__26748121_0E11_11D5_8C1E_D5461D71E42C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "PathTracker.h"	// base class





//////////////////////////////////////////
// CPathSpeed
//////////////////////////////////////////
/*
	Extends CPathTracker to find limit lists

*/


class CPathSpeed : public CPathTracker
{
public:
	CPathSpeed();
	virtual ~CPathSpeed();

// functions
	virtual void Init();

	void SetScanMinTime(double minTime) { m_ScanSection.timeMin = minTime; m_bCalcSegmentTimes = true; }

	void SetToStart();
	void SetInitialMotion(const SMotionStateBasic& ms);
	void FindSpeeds();
	SContBufferSpan& GetBufferStatus();
	int GetLastScanSegment() { return m_ScanSection.end.seg; }


// data
public:
	bool m_bSaveLimits;		// for testing


protected:

private:
	double m_dS;			// dS step size

	// path calculating flags
	bool m_bCalcSegmentLengths;
	bool m_bCalcSegmentTimes;

	// drive limits
	SDriveLimit m_StartCurrBrakeLimit;	// used to store some end conditions of current limit when braking
	SDriveLimit m_FowardLimit;		// current limit from foward scan limit list, used to find braking limits
	CLimitList m_FowardLimitList;
	CLimitList m_BackLimitList;
	int m_posFLL;

	double m_DirMatchAngSq;		// angle squared in radians

	// path scaning extents
	SPathSection m_ScanSection;	// contains start.seg & end.seg
	bool m_bFalseStopLimit;		// if stop at end of section is just to get safe end speeds

	SContBufferSpan m_BuffSpan;


// functions
protected:

private:
	void SetDefaults();
	void SetInitialPathLimit();
	void StorePathState(SPathLocInfo& pli);
	void RetrievePathState(SPathLocInfo& pli);
	int FindLastNonBrakeLimit(CLimitList& limitList);		// returns index of LastNonBrakeLimit


	void FindAccelLimits();
	void FindBrakeLimits();

	void SetStartCurrBrakeLimitFrom(const SMotionState& ms);
	void StoreCurrBrakeLimit();
	void AddNewFowardLimit(SDriveLimit& limit);
	void AddNewBackwardLimit(SDriveLimit& limit);



	void FindBoundSignChange(const SMotionState& msStart, SMotionState& ms,
				int axChanged, double* arAbreakaway, TVector<char>* arvtBound);
	void FindBoundAxisChange(SMotionState& msStart, SMotionState& ms,
				double* arAbreakaway, TVector<char>* arvtBound);

	void SetEndVelocityLimit(SDriveLimit& limit);

	void CheckAccelLimits();
	void CheckVelocity();
	void CheckCornerVelocity();
	void CheckEndCornerLimit();
	void CheckAcceleration();

	// used for reverse scan - FindBrakeLimits
	void CheckBrakeLimits();
	void CheckStartBraking();
	void CheckDeacceleration();

	void CheckPolyKnotDirections();

};

#endif // !defined(AFX_PATHSPEED_H__26748121_0E11_11D5_8C1E_D5461D71E42C__INCLUDED_)
