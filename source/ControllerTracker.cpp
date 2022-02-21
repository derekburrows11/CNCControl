// ControllerTracker.cpp: implementation of the CControllerTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNCControl.h"

#include "ControllerTracker.h"

#include "PosConverter.h"

//#include "Settings.h"			// for g_Settings

//#include "Points.h"		// for NEARINT(x)

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


///////////////////////////////////////////////////////
// limit values defined in "Limits.h"

bool NearIntCheck(long& iVal, double fVal)
{
	fVal = floor(fVal + 0.5);		// round to nearest whole value
	if (fVal > LONG_MAX)
	{
		iVal = LONG_MAX;
		return false;
	}
	if (fVal < LONG_MIN)
	{
		iVal = LONG_MIN;
		return false;
	}
	iVal = (long)fVal;
	return true;
}

bool NearIntCheck(__int8& iVal, double fVal)
{
	fVal = floor(fVal + 0.5);		// round to nearest whole value
	if (fVal > _I8_MAX)
	{
		iVal = _I8_MAX;
		return false;
	}
	if (fVal < _I8_MIN)
	{
		iVal = _I8_MIN;
		return false;
	}
	iVal = (__int8)fVal;
	return true;
}

bool NearIntCheck(__int16& iVal, double fVal)
{
	fVal = floor(fVal + 0.5);		// round to nearest whole value
	if (fVal > _I16_MAX)
	{
		iVal = _I16_MAX;
		return false;
	}
	if (fVal < _I16_MIN)
	{
		iVal = _I16_MIN;
		return false;
	}
	iVal = (__int16)fVal;
	return true;
}

bool NearIntCheck(__int32& iVal, double fVal)
{
	fVal = floor(fVal + 0.5);		// round to nearest whole value
	if (fVal > _I32_MAX)
	{
		iVal = _I32_MAX;
		return false;
	}
	if (fVal < _I32_MIN)
	{
		iVal = _I32_MIN;
		return false;
	}
	iVal = (__int32)fVal;
	return true;
}

bool NearIntCheck(__int64& iVal, double fVal)
{
	fVal = floor(fVal + 0.5);		// round to nearest whole value
	if (fVal > _I64_MAX)
	{
		iVal = _I64_MAX;
		return false;
	}
	if (fVal < _I64_MIN)
	{
		iVal = _I64_MIN;
		return false;
	}
	iVal = (__int64)fVal;
	return true;
}

///////////////////////////////////////////////////////

void SMotionI::Zero()
{
	iTime = 0;
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		pos[ax] = 0;
		vel[ax] = 0;
		acc[ax] = 0;
	}
}
void SMotionI::ZeroAcc()
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
		acc[ax] = 0;
}
void SMotionI::ZeroVel()
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
		vel[ax] = 0;
}

void SMotionI::UpdateMotion(const SSegAccel& accSeg)
{
	// update current motion of controller at end of step, given Init Accel, dAcc & idTime in segAcc
	// dp = dt * v0 + dt^2 * (a0/3 + a1/6)
	// d6p = dt * 3*2v0 + dt^2 * (2*a0 + a1)
	int idTime = accSeg.idTime;
	iTime += idTime;
	ASSERT(iTime == accSeg.iTimeEnd);
	INT_ACC_TYPE iAccInit, iAccFinal;
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		if (accSeg.nSegType == PATHSEG_dACCEL)
		{
			iAccInit = acc[ax];
			iAccFinal = iAccInit + idTime * accSeg.iAcc[ax];
		}
		else if (accSeg.nSegType == PATHSEG_ACCEL)
		{
			iAccInit = accSeg.iAcc[ax];
			iAccFinal = iAccInit;			// const accel
		}
		else
			return;
		// note: pos = 6*position, vel = 2*velocity, acc = acceleration
		pos[ax] += 3 * idTime * (INT_POS_TYPE)vel[ax]
					  + idTime*idTime * (INT_POS_TYPE)(2*iAccInit + iAccFinal);	// is 6*pos
		vel[ax] += idTime * (INT_VEL_TYPE)(iAccInit + iAccFinal);		// is 2*vel
		acc[ax]  = iAccFinal;
	}
}

bool SMotionI::UpdateMotionAtPathTime(const SSegAccel& accSeg, int iPathTime)
{
	// update current motion of controller at end of step, given Init Accel, dAcc & idTime in segAcc
	int idTime = iPathTime - iTime;
	if (idTime < 0 || idTime > accSeg.idTime)		// check idTime is within this segment
	{
		ASSERT(0);
		return false;
	}
	ASSERT(iTime == accSeg.GetTimeStart());
	iTime += idTime;
	INT_ACC_TYPE iAccInit, iAccFinal;
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		if (accSeg.nSegType == PATHSEG_dACCEL)
		{
			iAccInit = acc[ax];
			iAccFinal = iAccInit + idTime * accSeg.iAcc[ax];
		}
		else if (accSeg.nSegType == PATHSEG_ACCEL)
		{
			iAccInit = accSeg.iAcc[ax];
			iAccFinal = iAccInit;			// const accel
		}
		else
			return false;
		// note: pos is 6*pos and vel is 2*vel
		pos[ax] += 3 * idTime * (INT_POS_TYPE)vel[ax]
					  + idTime*idTime * (INT_POS_TYPE)(2*iAccInit + iAccFinal);	// is 6*pos
		vel[ax] += idTime * (INT_VEL_TYPE)(iAccInit + iAccFinal);		// is 2*vel
		acc[ax]  = iAccFinal;
	}
	return true;
}

void SMotionI::BackTrackMotion(const SSegAccel& accSeg)
{
	// find motion at start of segment given end motion
	int idTime = accSeg.idTime;
	ASSERT(iTime == accSeg.iTimeEnd);
	iTime -= idTime;
	INT_ACC_TYPE iAccInit, iAccFinal;
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		if (accSeg.nSegType == PATHSEG_dACCEL)
		{
			iAccFinal = acc[ax];
			iAccInit = iAccFinal - idTime * accSeg.iAcc[ax];
		}
		else if (accSeg.nSegType == PATHSEG_ACCEL)
		{
			iAccInit = accSeg.iAcc[ax];
			iAccFinal = iAccInit;			// const accel
		}
		else
			return;
		// note: pos is 6*pos and vel is 2*vel
		acc[ax]  = iAccInit;
		vel[ax] -= idTime * (INT_VEL_TYPE)(iAccInit + iAccFinal);		// is 2*vel
		pos[ax] -= 3 * idTime * (INT_POS_TYPE)vel[ax]
					  + idTime*idTime * (INT_POS_TYPE)(2*iAccInit + iAccFinal);	// is 6*pos
	}
}







SMotionI operator-(const SMotionI& lhs, const SMotionI& rhs)
{
	SMotionI res;
	res.iTime = lhs.iTime - rhs.iTime;
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		res.pos[ax] = lhs.pos[ax] - rhs.pos[ax];
		res.vel[ax] = lhs.vel[ax] - rhs.vel[ax];
		res.acc[ax] = lhs.acc[ax] - rhs.acc[ax];
	}
	return res;
}

bool operator==(const SMotionI& lhs, const SMotionI& rhs)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
		if ((lhs.pos[ax] != rhs.pos[ax]) || (lhs.vel[ax] != rhs.vel[ax]) || (lhs.acc[ax] != rhs.acc[ax]))
			return false;
	if (lhs.iTime != rhs.iTime)
		return false;
	return true;
}

SMotionF operator-(const SMotionF& lhs, const SMotionF& rhs)
{
	SMotionF res;
	res.t = lhs.t - rhs.t;
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		res.pos[ax] = lhs.pos[ax] - rhs.pos[ax];
		res.vel[ax] = lhs.vel[ax] - rhs.vel[ax];
		res.acc[ax] = lhs.acc[ax] - rhs.acc[ax];
	}
	return res;
}


void SetMotionState(SMotionStateBasic& ms, const SMotionF& mot)
{
	ms.t = mot.t;
	for (int i = 0; i < 3; i++)
	{
		ms.vtPos[i] = mot.pos[i];
		ms.vtVel[i] = mot.vel[i];
		ms.vtAcc[i] = mot.acc[i];
	}
}





//////////////////////////////////////////////////////////////////////
// class CControllerTracker
//////////////////////////////////////////////////////////////////////

// Construction/Destruction

CControllerTracker::CControllerTracker()
{
	m_pSegAccelFIFO = NULL;
	m_pPosConverter = NULL;

	m_dt = 0;		// indicates Scaling not set yet!

	m_bLogMotion = false;
}

CControllerTracker::~CControllerTracker()
{
}

// functions

void CControllerTracker::SetSegmentAccelBuffer(CSegAccelFIFO* pBuffer)
{
//	ASSERT(!m_pSegAccelFIFO);
	m_pSegAccelFIFO = pBuffer;
}

void CControllerTracker::LoadSettings(const CSettings& settings)
{
	const SControllerTracker& ct = settings.ContTrack;
	m_wtPos = ct.wtPos;
	m_wtVel = ct.wtVel;
	m_wtAcc = ct.wtAcc;
	m_wtVA = ct.wtVA;
	m_wtPA = ct.wtPA;
	m_wtPV = ct.wtPV;
	m_bAllowAccStepping = ct.bAllowAccStep;

	const SMachineParameters& mp = settings.MachParam;

	m_dt = mp.dTime;			// 1.024ms steps
	m_iStepsSegmentWt = NEARINT(settings.MachPathSeg.SegmentTime / m_dt);		// num steps in standard segment

/* These members are the real sizes of a unit of their intergral equivalents
	the intergral unit size is determined by intergral values sent to controller.
	These are generally 'accel change/time step' and accel
	dVel, dPos are the smallest change produced due to a unit changes of dAcc and dt
		dTime = 1.024ms
		resolution dPos = 2.5mm / 4096 ~= 0.61um
		dPosTrack = dPos / 2^24 for 6 byte PosTrack
		dAccel determined by range accepted by controller (16bit)
		max vel ~500mm/s; max acc ~500mm/s / 0.1s = 5000mm/s^2
		unit acc size = 5000/32768 ~= 0.15mm/s^2
*/
	m_dPos = mp.dPosTrack;
	m_dVel = mp.dVelTrack;
	m_dAcc = mp.dAccTrack;
	m_Invdt   = 1.0 / m_dt;
	m_InvdPos = 1.0 / m_dPos;
	m_InvdVel = 1.0 / m_dVel;
	m_InvdAcc = 1.0 / m_dAcc;		// precalc inverses


	m_iAccValueMin = -0x7fffff;		// sends 24 bit values
	m_iAccValueMax = +0x7fffff;		// sends 24 bit values

	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		m_Backlash.posI[ax] = (INT_POS_TYPE)floor(mp.BacklashPositive[ax] * m_InvdPos + 0.5);
		m_Backlash.negI[ax] = (INT_POS_TYPE)floor(mp.BacklashNegative[ax] * m_InvdPos + 0.5);
		ASSERT(fabs(m_Backlash.posI[ax] * m_dPos - mp.BacklashPositive[ax]) < 0.6*m_dPos);
		ASSERT(fabs(m_Backlash.negI[ax] * m_dPos - mp.BacklashNegative[ax]) < 0.6*m_dPos);
	}
}

void CControllerTracker::StoreSettings(CSettings& settings) const
{
	SControllerTracker& ct = settings.ContTrack;
	ct.wtPos = m_wtPos;
	ct.wtVel = m_wtVel;
	ct.wtAcc = m_wtAcc;
	ct.wtVA = m_wtVA;
	ct.wtPA = m_wtPA;
	ct.wtPV = m_wtPV;
	ct.bAllowAccStep = m_bAllowAccStepping;
}

void CControllerTracker::Init()
{
	LoadSettings(g_Settings);

	m_pPosConverter = &g_utPosConvert;

	CheckWeighting();

	m_SegAccelArray.SetSize(0, 256);
	m_ActualMotionIArray.SetSize(0, 256);
	m_RequestMotionIArray.SetSize(0, 256);
	m_bAllowAccStepNextSegment = false;

	m_CurrMotI.Zero();
	m_InitMotI.Zero();

	// testing and plotting
	m_iIndexPlotInit = 0;
	m_iTStepPlotInit = 0;
	m_iPrevPlotSeg = 0;
	m_iSegCount = 0;

	m_vtPosTipOfPathOrigin = 0;
}

void CControllerTracker::SetPosTipOfPathOrigin(CVector& vtPosTipOfPathOrigin)
{
	m_vtPosTipOfPathOrigin = vtPosTipOfPathOrigin;
}

void CControllerTracker::SetCurrentMotion(const SMotionStateBasic& ms)
{
	// adjust pos to actual machine pos for position correction
	CVector& vtPos = const_cast<CVector&>(ms.vtPos);
	CVector vtPosOrig = vtPos;
	vtPos += m_vtPosTipOfPathOrigin;			// get abs tip position
	m_pPosConverter->GetPosServoFromPosTip(vtPos);	// use servo position

	Real2Int(m_CurrMotI, ms);		// doesn't set iTime
	vtPos = vtPosOrig;			// reset ms.vtPos
	m_InitMotI = m_CurrMotI;
	if (m_bLogMotion)
		m_ActualMotionIArray.Add(m_CurrMotI);
}

void CControllerTracker::SetCurrentMotion(const SMotionState& ms)
{
	// adjust pos to actual machine pos for position correction
	CVector& vtPos = const_cast<CVector&>(ms.vtPos);
	CVector vtPosOrig = vtPos;
	vtPos += m_vtPosTipOfPathOrigin;			// get abs head machine position
	m_pPosConverter->GetPosServoFromPosTip(vtPos);	// use servo position
	
	Real2Int(m_CurrMotI, ms);		// doesn't set iTime
	vtPos = vtPosOrig;			// reset ms.vtPos
	m_InitMotI = m_CurrMotI;
	if (m_bLogMotion)
		m_ActualMotionIArray.Add(m_CurrMotI);
}

void CControllerTracker::SetCurrentMotionStopped()
{
	m_CurrMotI.ZeroVel();
	m_CurrMotI.ZeroAcc();
	m_InitMotI = m_CurrMotI;
//	m_ActualMotionIArray.SetAtGrow(0, m_CurrMotI);
}

void CControllerTracker::ClearMotionLog()
{
	if (m_bLogMotion)
		m_ActualMotionIArray.RemoveAll();
}

void CControllerTracker::SetPVWeighting(double PosWeight, double VelWeight)
{
	m_wtPos = PosWeight;		// 0.1
	m_wtVel = VelWeight;		// 0.45
	m_wtAcc = 1 - m_wtPos - m_wtVel;
}

void CControllerTracker::CheckWeighting()
{
	ASSERT(fabs(m_wtPos + m_wtVel + m_wtAcc - 1) <= 1e-10);
	ASSERT(m_wtPos >= 0.0 && m_wtPos <= 1.0);
	ASSERT(m_wtVel >= 0.0 && m_wtVel <= 1.0);
	ASSERT(m_wtAcc >= 0.0 && m_wtAcc <= 1.0);
}

void CControllerTracker::Int2Real(SMotionF& fMot, const SMotionI& iMot)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		fMot.acc[ax] = iMot.acc[ax] * m_dAcc;
		fMot.vel[ax] = iMot.vel[ax] * m_dVel;
		fMot.pos[ax] = iMot.pos[ax] * m_dPos;
	}
	fMot.t = iMot.iTime * m_dt;
}

void CControllerTracker::Int2Real(SMotionStateBasic& ms, const SMotionI& iMot)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		ms.vtAcc[ax] = iMot.acc[ax] * m_dAcc;
		ms.vtVel[ax] = iMot.vel[ax] * m_dVel;
		ms.vtPos[ax] = iMot.pos[ax] * m_dPos;
	}
	ms.t = iMot.iTime * m_dt;
}

void CControllerTracker::Real2Int(SMotionI& iMot, const SMotionF& fMot)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		iMot.acc[ax] = (INT_ACC_TYPE)floor(0.5 + fMot.acc[ax] * m_InvdAcc);
		iMot.vel[ax] = (INT_VEL_TYPE)floor(0.5 + fMot.vel[ax] * m_InvdVel);
		iMot.pos[ax] = (INT_POS_TYPE)floor(0.5 + fMot.pos[ax] * m_InvdPos);
	}
	iMot.iTime = NEARINT(fMot.t * m_Invdt);
}

void CControllerTracker::Real2Int(SMotionI& iMot, const SMotionStateBasic& ms)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		iMot.acc[ax] = (INT_ACC_TYPE)floor(0.5 + ms.vtAcc[ax] * m_InvdAcc);
		iMot.vel[ax] = (INT_VEL_TYPE)floor(0.5 + ms.vtVel[ax] * m_InvdVel);
		iMot.pos[ax] = (INT_POS_TYPE)floor(0.5 + ms.vtPos[ax] * m_InvdPos);
	}
}

void CControllerTracker::Real2Int(SMotionI& iMot, const SMotionState& ms)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		iMot.acc[ax] = (INT_ACC_TYPE)floor(0.5 + ms.vtAcc[ax] * m_InvdAcc);
		iMot.vel[ax] = (INT_VEL_TYPE)floor(0.5 + ms.vtVel[ax] * m_InvdVel);
		iMot.pos[ax] = (INT_POS_TYPE)floor(0.5 + ms.vtPos[ax] * m_InvdPos);
	}
}

void CControllerTracker::AllowStepNextSegment(const CVector* pvtAccInit /*= NULL*/)
{
	if (m_bAllowAccStepping)
		m_bAllowAccStepNextSegment = true;
	m_pvtAccInit = pvtAccInit;
}

bool CControllerTracker::MoveTo(const SMotionState& ms, int idT)
{
// Finds the best Accel Step/Ramp to get to next state from previous state
// Uses a weighted average of accelerations required to get to new pos, vel & acc


	// adjust pos to machine servo pos for position correction
	CVector& vtPos = const_cast<CVector&>(ms.vtPos);
	CVector vtPosOrig = vtPos;
	vtPos += m_vtPosTipOfPathOrigin;			// get abs head machine position
	// don't run g_PosConvert funcions in user thread!!
	m_pPosConverter->GetPosServoFromPosTip(vtPos);	// use servo position

	SMotionI ReqMotI;
	Real2Int(ReqMotI, ms);	// is required servo position
	vtPos = vtPosOrig;		// reset ms.vtPos

	double dTime = idT;
	double dTimeSq = idT * idT;


	SSegAccel segAcc0, segAcc;		// segAcc0 comes before segAcc if step required
	bool bSegAcc0Used = false;
	int iTimeEnd = m_CurrMotI.iTime + idT;
	segAcc.idTime = (short)idT;
	ASSERT(segAcc.idTime == idT);		// check conversion
	segAcc.iTimeEnd = iTimeEnd;
	ReqMotI.iTime = iTimeEnd;

	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		bool m_bApplyBacklashCorr = false;
		if (m_bApplyBacklashCorr)
		{
		// calc backlash position - simple method
		// could find when vel goes through zero and apply from there
		// don't use ReqMotI.vel - won't pickup changes when movement is only from 'tip to servo' conversion
		// look at sign change of ReqMotI.pos
			INT_POS_TYPE idReqPos = ReqMotI.pos[ax] - m_Backlash.posIPrev[ax];
			m_Backlash.posIPrev[ax] = ReqMotI.pos[ax];			// ReqMotI.pos gets modified!

			if (idReqPos > 0)
				m_Backlash.currDir[ax] = 1;
			else if (idReqPos < 0)
				m_Backlash.currDir[ax] = -1;
			ReqMotI.pos[ax] += (m_Backlash.currDir[ax] > 0) ? m_Backlash.posI[ax] : m_Backlash.negI[ax];
		}

		INT_VEL_TYPE idVel = ReqMotI.vel[ax] - m_CurrMotI.vel[ax];
		INT_POS_TYPE idPos = ReqMotI.pos[ax] - m_CurrMotI.pos[ax];
		double AccInit, AccFinal;
		double AiVA, AiPA, AiPV, AiA0A, AiA0V, AiA0P;		// used for step accel
		double AfVA, AfPA, AfPV, AfA0A, AfA0V, AfA0P;		// used for step accel
		double AfA, AfV, AfP;		// used for continuous accel
		if (idT == 0)	// check pos and vel continuous and set acc to requested
		{
			//ASSERT(idPos == 0);		// not if backlash changed!
			ASSERT(idVel == 0);
			segAcc.nSegType = PATHSEG_ACCEL;
			segAcc.iAcc[ax] = ReqMotI.acc[ax];
			ASSERT(segAcc.iAcc[ax] >= m_iAccValueMin && segAcc.iAcc[ax] <= m_iAccValueMax);
		}
		else if (m_bAllowAccStepNextSegment)			// If step required due to discontinuous Acc
		{
			// Set Final Acc to required and set Initial Acc such that vel is correct ???
			// note pos is 6*Pos and vel is 2*Vel intergral values
			ASSERT(m_pvtAccInit != NULL);
			double AiReq = floor(0.5 + (*m_pvtAccInit)[ax] * m_InvdAcc);	// take nearest integer value
			double AfReq = ReqMotI.acc[ax];
			double dVonT = idVel / dTime;
			double dPonT2 = idPos / dTimeSq;
			double VionT = m_CurrMotI.vel[ax] / dTime;

			AfVA = AfReq;							// first version used just AiPerr & AfPerr (A & V get to requested values)
			AiVA = dVonT - AfReq;		//
			AfPA = AfReq;
			AiPA = 0.5 * (dPonT2 - 3*VionT - AfReq);
			AfPV = -dPonT2 + 3*VionT + 2*dVonT;
			AiPV = dVonT - AfPV;

			AiA0A = AiReq;
			AfA0A = AfReq;
			AiA0V = AiReq;
			AfA0V = dVonT - AiReq;
			AiA0P = AiReq;
			AfA0P = dPonT2 - 3*VionT - 2*AiReq;

			double arWts[6];						// wtPos, wtVel, wtAcc respectivly
			GetStepAccWeights(idT, arWts);
			AccInit  = arWts[VA]*AiVA + arWts[PA]*AiPA + arWts[PV]*AiPV
							+ arWts[A0A]*AiA0A + arWts[A0V]*AiA0V + arWts[A0P]*AiA0P;
			AccFinal = arWts[VA]*AfVA + arWts[PA]*AfPA + arWts[PV]*AfPV
							+ arWts[A0A]*AfA0A + arWts[A0V]*AfA0V + arWts[A0P]*AfA0P;

//			segAcc.AccInit[ax] = NEARINT(AccInit);
			INT_ACC_TYPE idAcc = NEARINT((AccFinal - AccInit) / dTime);
			segAcc0.iAcc[ax] = NEARINT(AccFinal - idAcc * idT);	// AccInit
			segAcc.iAcc[ax] = idAcc;											// dAcc
			segAcc0.nSegType = PATHSEG_ACCEL;
			segAcc.nSegType = PATHSEG_dACCEL;
			segAcc0.idTime = 0;
			segAcc0.iTimeEnd = segAcc.iTimeEnd - idT;
			bSegAcc0Used = true;
			ASSERT(segAcc0.iAcc[ax] >= m_iAccValueMin && segAcc0.iAcc[ax] <= m_iAccValueMax);
			ASSERT(segAcc.iAcc[ax] >= m_iAccValueMin && segAcc.iAcc[ax] <= m_iAccValueMax);
		}
		else	// no step, acc continues from current value
		{
			// with requested motion rounded to intergral values:
			AccInit = m_CurrMotI.acc[ax];
			// note pos is 6*pos and vel is 2*vel intergral values
			AfA = ReqMotI.acc[ax];
			AfV = idVel / dTime - AccInit;				// make sure does floating division
			AfP = (idPos - 3 * idT * (INT_POS_TYPE)m_CurrMotI.vel[ax]) / dTimeSq
							- 2 * AccInit;			// make sure does floating division
			double arWts[3];						// wtPos, wtVel, wtAcc respectivly
			GetContAccWeights(idT, arWts);
			AccFinal = arWts[P] * AfP + arWts[V] * AfV + arWts[A] * AfA;
			segAcc.nSegType = PATHSEG_dACCEL;
			double fAccel = (AccFinal - AccInit) / dTime;
			VERIFY(NearIntCheck(segAcc.iAcc[ax], fAccel));
			ASSERT(segAcc.iAcc[ax] >= m_iAccValueMin && segAcc.iAcc[ax] <= m_iAccValueMax);


		}
		if (idT != 0)	// when check is valid!
		{
			// check where accel is too big!
//			if (fabs(AccInit) > 200 * m_InvdAcc)
//				TRACE2("ax:%i AccInit too big (%f) in CControllerTracker::MoveTo()\n", ax, AccInit * m_dAcc);
//			if (fabs(AccFinal) > 200 * m_InvdAcc)
//				TRACE2("ax:%i AccFinal too big (%f) in CControllerTracker::MoveTo()\n", ax, AccFinal * m_dAcc);
		}
	}	// for (int ax = 0; ax < NUM_AXIS; ax++)
	m_bAllowAccStepNextSegment = false;

	if (bSegAcc0Used)
	{
		segAcc0.iSegCount = m_iSegCount++;
		m_CurrMotI.UpdateMotion(segAcc0);	// update current motion of controller at end of step, given Init Accel and dAcc & idTime
	}
	segAcc.iSegCount = m_iSegCount++;
	m_CurrMotI.UpdateMotion(segAcc);			// update current motion of controller at end of step, given Init Accel and dAcc & idTime
	
	if (m_bStorePosError)
	{
		m_bStorePosError = false;
		for (int ax = 0; ax < NUM_AXIS; ax++)
			m_vtdPos[ax] = (m_CurrMotI.pos[ax] - ReqMotI.pos[ax]) * m_dPos;
	}

	// allow for backlash when direction changes - apply accel segs to adjust position
	// use ReqMotI pos and vel for calcs instead of m_CurrMot
/*	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		if (m_CurrMotI.vel[ax] < 0 && m_CurrBacklash[ax] > 0)
		{
			// start negative backlash
			// find time step when vel goes through zero
			m_Backlash.Tstep = 0
			m_CurrBacklash[ax] = -1;
		}
		else if (m_CurrMotI.vel[ax] > 0 && m_CurrBacklash[ax] < 0)
		{
			// start positive backlash
			m_CurrBacklash[ax] = 1;
		}
	}
*/


// Add Controller segment to multithread FIFO buffer
	int iBufferSpace = 2;
	if (m_pSegAccelFIFO)
	{
		if (bSegAcc0Used)
			m_pSegAccelFIFO->Add(segAcc0);	// if returns -1, not stored
		iBufferSpace = m_pSegAccelFIFO->Add(segAcc);			// if returns -1, not stored
		ASSERT(iBufferSpace >= 0);
	}



	if (m_bLogMotion)
	{
		// calc error
		SMotionI iErrMot;
		iErrMot = m_CurrMotI - ReqMotI;

	// store motions for testing
		if (bSegAcc0Used)
			m_SegAccelArray.Add(segAcc0);
		m_SegAccelArray.Add(segAcc);

//		if (idT != 0)
		{
			m_ActualMotionIArray.Add(m_CurrMotI);
			m_RequestMotionIArray.Add(ReqMotI);
		}
	}

	return (iBufferSpace >= 2);		// room two for more
}

bool CControllerTracker::StopAtPosAccelStep(const SMotionState& ms, int idT)
{
/* Formula for a step and ramp seg of t to move p02 starting at v0 & a0
	a1NoPVerr = (6*v0 + 4*dv) / dt - 6*dp / dt^2
	a0NoPVerr = 2*dv / dt - a1NoPVerr
*/
	// adjust pos to machine servo pos for position correction
	CVector& vtPos = const_cast<CVector&>(ms.vtPos);
	CVector vtPosOrig = vtPos;
	vtPos += m_vtPosTipOfPathOrigin;			// get abs head machine position
	// don't run g_PosConvert funcions in user thread!!
	m_pPosConverter->GetPosServoFromPosTip(vtPos);	// use servo position

	SMotionI ReqMotI;
	Real2Int(ReqMotI, ms);
	vtPos = vtPosOrig;		// reset ms.vtPos

	double dT = idT;
	double dTSq = idT * idT;

	SSegAccel segAcc0, segAcc;
	segAcc0.nSegType = PATHSEG_ACCEL;
	segAcc.nSegType = PATHSEG_dACCEL;
	segAcc0.idTime = 0;
	segAcc.idTime = (short)idT;
	segAcc0.iTimeEnd = m_CurrMotI.iTime;
	segAcc.iTimeEnd = segAcc0.iTimeEnd + idT;

	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		// set as pos types for range when multiplied
		INT_POS_TYPE iVel0 = m_CurrMotI.vel[ax];
		INT_POS_TYPE idPos = ReqMotI.pos[ax] - m_CurrMotI.pos[ax];

		double a1 = (iVel0*idT - idPos) / dTSq;
		double a0 = (-iVel0 / dT) - a1;

		// find closest idA
		double dA = (a1 - a0) / dT;
		int idA = NEARINT(dA);
		int iA01 = idA*idT;
		// set iA0 & iA1 so avg(iA0, iA1) == avg(a0, a1)
		int iA0 = NEARINT(0.5*(a0+a1 - iA01));		// = aMid - iA01/2

		segAcc0.iAcc[ax] = iA0;
		segAcc.iAcc[ax]  = idA;
	}

	segAcc0.iSegCount = m_iSegCount++;
	segAcc.iSegCount  = m_iSegCount++;
	m_CurrMotI.UpdateMotion(segAcc0);
	m_CurrMotI.UpdateMotion(segAcc);

	// Store Pos Error for checking
	for (ax = 0; ax < NUM_AXIS; ax++)
		m_vtdPos[ax] = (m_CurrMotI.pos[ax] - ReqMotI.pos[ax]) * m_dPos;

// Add Controller segment to multithread FIFO buffer
	int iBufferSpace = 2;
	if (m_pSegAccelFIFO)
	{
		m_pSegAccelFIFO->Add(segAcc0);	// if returns -1, not stored
		iBufferSpace = m_pSegAccelFIFO->Add(segAcc);			// if returns -1, not stored
		ASSERT(iBufferSpace >= 0);
	}
	return (iBufferSpace >= 2);		// room two for more
}

void CControllerTracker::GetContAccWeights(int iSteps, double* arWts)
{
	// modifies the 3 weightings for shorter segments than normal using continuous accel
	if (iSteps >= m_iStepsSegmentWt)
	{
		arWts[P] = m_wtPos;
		arWts[V] = m_wtVel;
		arWts[A] = m_wtAcc;
		return;
	}
	double t = (double)iSteps / m_iStepsSegmentWt;
	arWts[P] = m_wtPos * t*t;
	arWts[V] = m_wtVel * t;
	arWts[A] = m_wtAcc;
	double scale = 1.0 / (arWts[P] + arWts[V] + arWts[A]);
	arWts[P] *= scale;
	arWts[V] *= scale;
	arWts[A] *= scale;
}

void CControllerTracker::GetStepAccWeights(int iSteps, double* arWts)
{
	// set to earlier weightings
	arWts[VA] = 0.7;
	arWts[PA] = 0.2;
	arWts[PV] = 0.1;
	arWts[A0A] = 0;
	arWts[A0V] = 0;
	arWts[A0P] = 0;
//	return;
	
	// modifies the  6 weightings for shorter segments than normal using stepped accel
	// m_wtVA = 0.30; m_wtPA = 0.10; m_wtPV = 0.025;
	if (iSteps >= m_iStepsSegmentWt)
	{
		arWts[VA] = m_wtVA;
		arWts[PA] = m_wtPA;
		arWts[PV] = m_wtPV;
		arWts[A0A] = 1.0 - 2*m_wtVA - 2*m_wtPA - m_wtPV;
		arWts[A0V] = m_wtVA;
		arWts[A0P] = m_wtPA;
		return;
	}
	double t = (double)iSteps / m_iStepsSegmentWt;
	double tsq = t*t;
	arWts[PA] = m_wtPA * tsq;
	arWts[PV] = m_wtPV * tsq;
	arWts[A0P] = arWts[PA];

	double wtRem = 1 - arWts[PA] - arWts[PV] - arWts[A0P];	// wtRem is wtVA + wtA0V + wtA0A = 2*wtVA + wtA0A
	double w = 2 * m_wtVA;
	arWts[VA] = w * t * wtRem / (1.0 + w*t);
	arWts[A0V] = arWts[VA];
	arWts[A0A] = wtRem - arWts[VA] - arWts[A0V];
}

void CControllerTracker::SetMotionArray(CMotionStateBasicArray& msArr, CMotionIArray& motIArr)
{
	int numPts = motIArr.GetSize();
	msArr.SetSize(numPts);
	SMotionF fMot;
	SMotionStateBasic ms;
	for (int i = 0; i < numPts; i++)
	{
		Int2Real(fMot, motIArr[i]);
		SetMotionState(ms, fMot);
		msArr[i] = ms;
	}
}

void CControllerTracker::SetActualMotionArray(CMotionStateBasicArray& msArr)
{
	// sets msArr from m_SegAccelArray and verifies against m_ActualMotionIArray
	int numPts = m_SegAccelArray.GetSize();
	ASSERT(numPts > 0);
	msArr.SetSize(2 * numPts);		// PATHSEG_ACCEL with hold requires 2 points
	SMotionI iMot;
	SMotionF fMot;
	GetInitialMotion(iMot);
	int idxMSArr = 0;
	int idxAMArr = 0;
	Int2Real(fMot, iMot);
	SetMotionState(msArr[idxMSArr++], fMot);		// store first point

	for (int i = 0; i < numPts; i++)
	{
		// motion at start of segment is in iMot
		SSegAccel& segAcc = m_SegAccelArray[i];
		int nSegType = segAcc.nSegType;
		if (nSegType != PATHSEG_ACCEL && nSegType != PATHSEG_dACCEL)
			continue;
		if (nSegType == PATHSEG_ACCEL)			// add extra point
		{
			for (int ax = 0; ax < NUM_AXIS; ax++)
				iMot.acc[ax] = segAcc.iAcc[ax];
			Int2Real(fMot, iMot);
			SetMotionState(msArr[idxMSArr++], fMot);
		}
		if (segAcc.idTime != 0)
		{
			iMot.UpdateMotion(segAcc);
			Int2Real(fMot, iMot);
			SetMotionState(msArr[idxMSArr++], fMot);

			if (!(iMot == m_ActualMotionIArray[++idxAMArr]))	// check when (segAcc.idTime != 0)
			{
				TRACE("ERROR in segment calcs - CControllerTracker::SetActualMotionArray()\n");
				ASSERT(0);
			}
		}

	}
	msArr.SetSize(idxMSArr);		// truncate to actual used size
}



#include "..\Graph\Plot.h"		// only testing

void CControllerTracker::PlotMotions()
{
//	m_iIndexPlotInit;		next index in m_SegAccelArray to start plot from
//	m_iTStepPlotInit;		time step count of next start point
	const int numSegs = m_SegAccelArray.GetSize() - m_iIndexPlotInit;
	const int numPts = numSegs + 1;
	const int numPtsBez = 3*numSegs + 1;
	const int numPtsAcc = 2*numSegs;

	double* arTBez = new double[numPtsBez];		// enough for bezier segments
	double* arBezP = new double[numPtsBez];
	double* arBezV = new double[numPtsBez];
	double* arTAcc = new double[numPtsAcc];
	double* arAcc  = new double[numPtsAcc];
	SMotionF fMot, fMotInit;
	SMotionI iMot;

	int iTStep = 0;
	for (int array = 0; array < 2; array++)
	{
		CArray<SMotionI, SMotionI&>* pMotionArray;
		int numPtsAccel = numPtsAcc;
		if (array == 0)
			pMotionArray = &m_ActualMotionIArray;
		else	// if (array == 1)
		{
			pMotionArray = &m_RequestMotionIArray;
			numPtsAccel = numPts;
		}
		for (int ax = 0; ax < NUM_AXIS; ax++)
		{
			int idxAr = m_iIndexPlotInit;
			int idxBez = 3;									// end node of segment
			iMot = pMotionArray->ElementAt(idxAr);		// set first motion
			Int2Real(fMot, iMot);
			iTStep = m_iTStepPlotInit;
			for (int i = 0; i < numSegs; i++)
			{
				SSegAccel& segAcc = m_SegAccelArray.ElementAt(idxAr);
				if (array == 0)
				{
					iMot.UpdateMotion(segAcc);
					ASSERT(iMot == pMotionArray->ElementAt(idxAr + 1));
//					SMotionI& iStoredMot = pMotionArray->ElementAt(idxAr + 1);
//					ASSERT(iMot == iStoredMot);
				}
				else if (array == 1)
					iMot = pMotionArray->ElementAt(idxAr + 1);
				fMotInit = fMot;
				Int2Real(fMot, iMot);

				int idTime = segAcc.idTime;
				iTStep += idTime;			// initially on time of first point
				ASSERT(iTStep == segAcc.iTimeEnd);
				double tStep = iTStep * m_dt;
				double dTon3 = idTime * m_dt / 3.0;

				arTBez[idxBez] = tStep;
				arBezP[idxBez] = fMot.pos[ax];
				arBezV[idxBez] = fMot.vel[ax];
				if (i == 0)			// else
				{
					arTBez[idxBez-3] = (iTStep - idTime) * m_dt;	// previous time
					arBezP[idxBez-3] = fMotInit.pos[ax];
					arBezV[idxBez-3] = fMotInit.vel[ax];
				}
				arTBez[idxBez-2] = tStep - 2*dTon3;
				arTBez[idxBez-1] = tStep - dTon3;

				arBezP[idxBez-2] = fMotInit.pos[ax] + fMotInit.vel[ax] * dTon3;
				arBezV[idxBez-2] = fMotInit.vel[ax] + fMotInit.acc[ax] * dTon3;
				arBezP[idxBez-1] = fMot.pos[ax] - fMot.vel[ax] * dTon3;
				arBezV[idxBez-1] = fMot.vel[ax] - fMot.acc[ax] * dTon3;

				if (array == 0)
				{
					arTAcc[2*i  ] = (iTStep - idTime) * m_dt;		// start of segment time
					arTAcc[2*i+1] = tStep;
					arAcc[2*i  ] = fMotInit.acc[ax];
					arAcc[2*i+1] = fMot.acc[ax];
				}
				if (array == 1)
				{
					if (i == 0)
					{
						arTAcc[i] = (iTStep - idTime) * m_dt;
						arAcc[i] = fMotInit.acc[ax];
					}
					arTAcc[i+1] = tStep;
					arAcc[i+1] = fMot.acc[ax];
				}

				// do inc's
				idxAr++;
				idxBez += 3;
			}
			// plot points
			double scaleVel = 1.0;
			for (i = 0; i < numPtsBez; i++)
				arBezV[i] *= scaleVel;
			Plot(arTBez, arBezP, numPtsBez, GPS_BEZIER);
			Plot(arTBez, arBezV, numPtsBez, GPS_BEZIER);
			Plot(arTAcc, arAcc, numPtsAccel, GPS_LINE);
		}	// for (ax =...)
	}
	m_iIndexPlotInit += numPts-1;		// leave it pointing to final point
	m_iTStepPlotInit = iTStep;

	delete[] arTBez;
	delete[] arBezP;
	delete[] arBezV;
	delete[] arTAcc;
	delete[] arAcc;
}

