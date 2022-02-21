// PathTimeStep.h: interface for the CPathTimeStep class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHTIMESTEP_H__C75B07C2_4AB7_11D5_8C1E_8ED569C32F2F__INCLUDED_)
#define AFX_PATHTIMESTEP_H__C75B07C2_4AB7_11D5_8C1E_8ED569C32F2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "PathMove.h"	// base class
#include "PolySegFitPoints.h"
#include "ControllerTracker.h"


//////////////////////////////////////////
// CPathTimeStep
//////////////////////////////////////////

class CPathTimeStep : public CPathMove
{
public:
	CPathTimeStep();
	virtual ~CPathTimeStep();

// functions
	virtual void Init();

	void SetControllerTracker(CControllerTracker* pCT) { m_pCtrlTrack = pCT; }
	void SetToStart();				// can be called before limit list set
	void FindInitialTimeStep();	// called after limit list set
	void FindTimeSteps();
	bool FinishedPathSteps() { return m_bFinishedPathSteps; }

	SContBufferSpan& GetBufferStatus() { return m_BuffSpan; }

protected:
  enum STEP_METHOD
	{
		SM_CONSTSTEP = 0,
		SM_FITPOLY,
	};

// data members
protected:

private:
	CControllerTracker* m_pCtrlTrack;
	
	int m_iStepMethod;

	// members used with all methods
	bool m_bFinishedPathSteps;
	int m_iFinishMovesCount;


	// members used with Constant Step method only
	int m_iStepAdv;		// standard number of discrete steps to advance for a segment
	double m_dtAdv;

	// members used with Fit Poly method only
	double m_posTol;	// not used yet
	CPolySegFitPoints m_PolySegs;	// object which fits polys to points

	// used for tracking segment init and final Acc's
	CVector m_vtSegInitAcc;
	CVector m_vtNextSegInitAcc;
	bool m_bSegInitAccStep;
	bool m_bNextSegInitAccStep;

	SContBufferSpan m_BuffSpan;

// member functions
protected:

private:
	void SetDefaults();

	void FindTimeStepsConstStep();
	void FindTimeStepsFitPoly();

	bool DoChange(bool& bAccSteps);

	void SortSegmentAccStep();
	bool SegInitAccStepped() { return m_bSegInitAccStep; }
	CVector& GetSegInitAcc() { return m_vtSegInitAcc; }
	void SetBufferInfo(SContBufferInfo& buffInfo);



//	void FitCubic();
};

#endif // !defined(AFX_PATHTIMESTEP_H__C75B07C2_4AB7_11D5_8C1E_8ED569C32F2F__INCLUDED_)
