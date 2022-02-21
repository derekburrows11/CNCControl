// ControllerTracker.h: interface for the CControllerTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTROLLERTRACKER_H__2F206022_4DE8_11D5_8C1E_F2E209532B2F__INCLUDED_)
#define AFX_CONTROLLERTRACKER_H__2F206022_4DE8_11D5_8C1E_F2E209532B2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "FIFOBuffer.h"
#include "PathDataObjects.h"
#include "Settings.h"

class CPosConverter;

//#define NUM_AXIS			3
#define VALID_AXIS(ax)				(ax >=0 && ax < NUM_AXIS)
#define ASSERT_VALID_AXIS(ax)		ASSERT(VALID_AXIS(ax))


typedef __int64 INT_POS_TYPE;
typedef __int64 INT_VEL_TYPE;
typedef long    INT_ACC_TYPE;		// will need check if this type is changed!


//bool NearIntCheck(char& iVal, double fVal);
//bool NearIntCheck(short& iVal, double fVal);
bool NearIntCheck(long& iVal, double fVal);
//bool NearIntCheck(int& iVal, double fVal);		// equivalent to __int32

bool NearIntCheck(__int8& iVal, double fVal);
bool NearIntCheck(__int16& iVal, double fVal);
bool NearIntCheck(__int32& iVal, double fVal);
bool NearIntCheck(__int64& iVal, double fVal);


enum PATHSEG		// path segment packet types
{
	PATHSEG_dACCEL				= 0,	// was RAMP uses 12 bit time
	PATHSEG_ACCEL				= 1,	// was STEP uses 12 bit time
	PATHSEG_dACCELCOUNT		= 2,	// uses 8 bit time and 4 bit seg count
	PATHSEG_ACCELCOUNT		= 3,	// uses 8 bit time and 4 bit seg count
	PATHSEG_SETdACCELFACT	= 4,
	PATHSEG_SETACCELFACT		= 5,
	
	PATHSEG_SETDRIVEPROPS	= 6,
	PATHSEG_SETCONTROLVALUE	= 7,
	
	PATHSEG_RAMPTOVEL			= 11,
	PATHSEG_STEPTOVEL			= 12,

	PATHSEG_ENDZEROACCEL		= 13,
	PATHSEG_ENDRAMPZEROVEL	= 14,
	PATHSEG_ENDZEROVEL		= 15,
};

struct SSegAccel
{
	long iTimeEnd;
	long iSegCount;		// only if packets need it!
	short idTime;			// time steps (~1ms)
	short nSegType;		// step to iAccel and hold for idTime, or ramp at iAccel for iTime
	INT_ACC_TYPE iAcc[NUM_AXIS];

	long GetTimeStart() const { return iTimeEnd - idTime; }
	long GetTimeEnd() const { return iTimeEnd; }
};

typedef TFIFO<SSegAccel> CSegAccelFIFO;


struct SMotionI
{
	long iTime;							// = time step
	INT_POS_TYPE pos[NUM_AXIS];	// = 6*position
	INT_VEL_TYPE vel[NUM_AXIS];	// = 2*velocity
	INT_ACC_TYPE acc[NUM_AXIS];	// = acceleration

	void Zero();
	void ZeroAcc();
	void ZeroVel();
	void UpdateMotion(const SSegAccel& accSeg);
	bool UpdateMotionAtPathTime(const SSegAccel& accSeg, int iPathTime);
	void BackTrackMotion(const SSegAccel& accSeg);
};

struct SMotionF
{
	double t;					// time of step
	double pos[NUM_AXIS];	// mm??
	double vel[NUM_AXIS];
	double acc[NUM_AXIS];
};

SMotionI operator-(const SMotionI& lhs, const SMotionI& rhs);
bool operator==(const SMotionI& lhs, const SMotionI& rhs);

SMotionF operator-(const SMotionF& lhs, const SMotionF& rhs);


typedef CArray<SSegAccel, SSegAccel&> CSegAccelArray;
typedef CArray<SMotionI, SMotionI&> CMotionIArray;

void SetMotionState(SMotionStateBasic& ms, const SMotionF& mot);


struct SBacklashData
{
	INT_POS_TYPE posIPrev[NUM_AXIS];
	INT_POS_TYPE posI[NUM_AXIS];
	INT_POS_TYPE negI[NUM_AXIS];
	int currDir[NUM_AXIS];
};



////////////////////////////


class CControllerTracker  
{
public:
	CControllerTracker();
	virtual ~CControllerTracker();

	void SetSegmentAccelBuffer(CSegAccelFIFO* pBuffer);
	CSegAccelFIFO* GetSegmentAccelBuffer() { return m_pSegAccelFIFO; }

	void Init();
	void SetPosTipOfPathOrigin(CVector& vtPosTipOfPathOrigin);
//	void SetInitialMotion(const SMotionState& ms);
	void SetCurrentMotion(const SMotionState& ms);
	void SetCurrentMotion(const SMotionStateBasic& ms);
	void SetCurrentMotionStopped();
	void GetCurrentMotion(SMotionStateBasic& ms) { Int2Real(ms, m_CurrMotI); }	// sets time
	double GetCurrentTime() { return m_CurrMotI.iTime * m_dt; }
	int GetCurrentSeg() { return m_iSegCount; }
	void ClearMotionLog();

	void GetInitialMotion(SMotionI& motI) { motI = m_InitMotI; }

	void SetPVWeighting(double PosWeight, double VelWeight);
	void CheckWeighting();


	bool RoomInSegBuffer() { return (m_pSegAccelFIFO != NULL) ? (m_pSegAccelFIFO->GetFree() >= 2) : true; }
	void AllowStepNextSegment(const CVector* pvtAccInit = NULL);
	void StoreNextPosError() { m_bStorePosError = true; }
	double* GetCurrPosError() { return m_vtdPos; }
	bool MoveTo(const SMotionState& ms, int idT);					// returns 0 if buffer now full
	bool StopAtPosAccelStep(const SMotionState& ms, int idT);	// returns 0 if buffer now full
	void SetMotionArray(CMotionStateBasicArray& msArr, CMotionIArray& motIArr);
	void SetActualMotionArray(CMotionStateBasicArray& msArr);


	void Int2Real(SMotionF& fMot, const SMotionI& iMot);
	void Int2Real(SMotionStateBasic& ms, const SMotionI& iMot);
	void Real2Int(SMotionI& iMot, const SMotionF& fMot);
	void Real2Int(SMotionI& iMot, const SMotionStateBasic& ms);
	void Real2Int(SMotionI& iMot, const SMotionState& ms);
	void GetContAccWeights(int iSteps, double* arWts);
	void GetStepAccWeights(int iSteps, double* arWts);

	double Get_dt() { return m_dt; }
	double Get_dPos() { return m_dPos; }
	double Get_dVel() { return m_dVel; }
	double Get_dAcc() { return m_dAcc; }


	// lists for testing and plotting
	bool m_bLogMotion;
	CSegAccelArray m_SegAccelArray;
	CMotionIArray m_ActualMotionIArray;
	CMotionIArray m_RequestMotionIArray;
	int m_iIndexPlotInit;
	int m_iTStepPlotInit;
	int m_iPrevPlotSeg;		// used to indicate plot after each segment
	void PlotMotions();

	void LoadSettings(const CSettings& settings);
	void StoreSettings(CSettings& settings) const;



protected:

	double m_dt, m_dPos, m_dVel, m_dAcc;		// sec, mm, mm/s, mm/s^2
//	double m_dAccFactor;				// to scale dAcc per time step values to appropriate integers								// 
	double m_Invdt, m_InvdPos, m_InvdVel, m_InvdAcc;

	int m_iStepsSegmentWt;
	double m_wtPos, m_wtVel, m_wtAcc;
//	double m_wtSIPosVel, m_wtSIPosAcc, m_wtSIVelAcc;	// weighting for init  accel with step
//	double m_wtSFPosVel, m_wtSFPosAcc, m_wtSFVelAcc;	// weighting for final accel with step
	double m_wtVA, m_wtPA, m_wtPV;
	enum { P, V, A };
	enum { VA, PA, PV, A0A, A0V, A0P };

	SMotionI m_CurrMotI;		// intergral value of controller state
	SMotionI m_InitMotI;		// intergral initial value of controller state
	bool m_bAllowAccStepping;
	bool m_bAllowAccStepNextSegment;
	const CVector* m_pvtAccInit;
	int m_iSegCount;

	// backlash data
	SBacklashData m_Backlash;

	bool m_bStorePosError;
	double m_vtdPos[NUM_AXIS];


	int m_iAccValueMin;			// value size checking for 2byte unsigned
	int m_iAccValueMax;

	CSegAccelFIFO* m_pSegAccelFIFO;	// buffer to send calculated segments

	CVector m_vtPosTipOfPathOrigin;	// head pos machine of path origin - set on start of path

	CPosConverter* m_pPosConverter;

};

#endif // !defined(AFX_CONTROLLERTRACKER_H__2F206022_4DE8_11D5_8C1E_F2E209532B2F__INCLUDED_)
