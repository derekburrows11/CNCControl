// PathMove.h: interface for the CPathMove class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHMOVE_H__F9BCD682_AA01_11D6_86C3_F2205CD8B726__INCLUDED_)
#define AFX_PATHMOVE_H__F9BCD682_AA01_11D6_86C3_F2205CD8B726__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#include "PathSpeed.h"
#include "PathTracker.h"




/*
struct SStateStore
{
	CMatrixf* pmxsPts;
	CMatrixf* pmxdsdtPts;
	CMatrix* pmxtPts;
	CVectorf (*arvtRes)[3];
	int* piPtIdx;
	int iNumPoints;
};
*/

//////////////////////////////////////////
// CPathMove
//////////////////////////////////////////

/*
	Extends CPathSpeed to move through path

*/


//class CPathMove : public CPathSpeed
class CPathMove : public CPathTracker
{
public:
	CPathMove();
	virtual ~CPathMove();

	virtual void Init();

	void SetToStart();
	void LimitListChanged();			// used to notify of new limit list
	void GetSpeedsAtS(CMotionStateBasicArray& motArray);
	void GetSpeedsAtTime(CMotionStateBasicArray& motArray);

	bool AtEndOfCalcLimits() { return m_bAtEndOfLimits; }
	int GetCurrentSegment() { return ms1.seg; }


// data members for moving through path
protected:
	enum CHANGE_LOC
	{
		NO_CHANGE = 0,
		CHANGE_IS_AT_STEP,
		CHANGE_IS_BEFORE_STEP,	// change is just before closest timestep
		CHANGE_IS_AFTER_STEP,	// change is just after  closest timestep
	};

	SMotionState msPre, msPost;

	double m_TStartLimit;		// time of start of current limit from start of path
	double m_TLimit;				// time on current limit
	double m_TLimitsChanged;	// keeps track of changes for backtracking
	int m_iTStepPath;				// current time step count


	// for discrete time stepping only
	double m_dT;		// Discrete Time Step of controller (sec) = ~ 1ms
	double m_dTOn2;	// dT / 2
	double m_dTInv;	// inverse dT

	double m_dTSegment;	// standard segment time


private:
	enum ADVANCE_TYPE
	{
		ADVANCE_S = 1,
		ADVANCE_TIME,
	};

protected:		// 2 used in CPathTimeStep, change to private!
	int m_nChangeType;			// current change type - flags CHANGE_TYPE
	double m_TNextChange;		// limit time of next change
private:
	int m_nNextChangeType;		// next change type - flags CHANGE_TYPE

	// flags
	bool m_bAtEndOfLimits;
	bool m_bNextLimitIsEndCalc;
	bool m_bGetPreChangeValues;
	bool m_bGetPostChangeValues;

	int m_nAdvanceType;		// enum ADVANCE_TYPE
	int m_nChangeLoc;			// enum CHANGE_LOC
	int m_nPreChangeLoc;		// enum CHANGE_LOC
	int m_nPostChangeLoc;	// enum CHANGE_LOC
	int m_nStopAt;				// flags CHANGE_TYPE


	// for discrete time stepping only
	int m_idTSteps;		// number of m_dT steps this current advance

	// for discrete s stepping only
	double m_dS;			// dS step size
	int m_iSSteps;




// Functions for moving through path
protected:
	// setup functions
	void SetPathMoveFlags(int flags);
	void SetDiscreteTStep(double dT);
	void FindCurrentLimit();

	// moving functions
	void AdvanceBydT(double dt);
	int GetdTSteps() { return m_idTSteps; }
	int GetTStep() { return m_iTStepPath; }
	int GetChangeLoc() { return m_nChangeLoc; }
	double GetPathTime() { return m_TStartLimit + m_TLimit; }
	void GetTimeNextChange();

private:
	void SetDefaults();

	// path state functions
	bool GotChangeState();
	int GotPreChangeState();
	int GotPostChangeState();

	// moving functions
	void SetToStartOfPath();
	void AdvanceSStep();
//	void AdvanceBydS(double ds);
	void AdvanceToSPoint();

	void GetMotion();
	void DoChange();

};

#endif // !defined(AFX_PATHMOVE_H__F9BCD682_AA01_11D6_86C3_F2205CD8B726__INCLUDED_)
