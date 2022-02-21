// MachineState.cpp: implementation of the CMachineState class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MachineState.h"

#include "MachineCodes.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

double CMachineState::m_zAxisPosTipAtBaseTop;

CMachineState::CMachineState()
{
	memset(this, 0, sizeof(CMachineState));
}

CMachineState::~CMachineState()
{

}

/////////////////////////////////
// CMachineState
/////////////////////////////////

void CMachineState::OnUpdatePosServo()		// calc PosTip from PosServo
{
	// only run g_PosConvert functions in monitor thread !!!
	CVector vtPosServo;
	GetPosServo(vtPosServo);
	g_mtPosConvert.GetPosTipFromPosServo(vtPosServo, vtPosTip);	// get absolute tip pos
}

void CMachineState::OnUpdatePosTrack()		// calc PosTip Track from PosTrack - equivalant to PosServo
{
	// only run g_PosConvert functions in monitor thread !!!
	CVector vtPosTrack;
	GetPosTrack(vtPosTrack);
	g_mtPosConvert.GetPosTipFromPosServo(vtPosTrack, vtPosTipTrack);	// get absolute tip pos
}

void CMachineState::OnUpdatePosProbe()		// calc PosProbeTip from PosServo and Probe extension
{
	// only run g_PosConvert functions in monitor thread !!!
	CVector vtPosServo;
	GetPosServo(vtPosServo);
	g_mtPosConvert.GetPosProbeTipFromPosServo(vtPosServo, ProbeZExt, vtPosProbeTip);	// get absolute tip pos
}

void CMachineState::GetPosTipRelBase(CVector& vtPos)		// tip height relative to base
{
	vtPos = vtPosTip;
	vtPos.z -= m_zAxisPosTipAtBaseTop;
}

void CMachineState::GetPosTip(CVector& vtPos)				// tip height from absolute position
{
	vtPos = vtPosTip;
}

void CMachineState::GetPosTipTrackRelBase(CVector& vtPos)		// tip track height relative to base
{
	vtPos = vtPosTipTrack;
	vtPos.z -= m_zAxisPosTipAtBaseTop;
}

void CMachineState::GetPosTipTrack(CVector& vtPos)				// tip track height from absolute position
{
	vtPos = vtPosTipTrack;
}

void CMachineState::GetPosHead(CVector& vtPos)				// servo position corrected at tip
{
	vtPos = vtPosTip;
//	vtPos.z += g_utPosConvert.GetZAxisHead2TipDist();		// get head from tip - just used to set tool dimensions
}

void CMachineState::GetPosProbeRelBase(CVector& vtPos)	// tip height relative to base
{
	vtPos = vtPosProbeTip;
	vtPos.z -= m_zAxisPosTipAtBaseTop;
}

void CMachineState::GetPosProbe(CVector& vtPos)				// tip height from absolute position
{
	vtPos = vtPosProbeTip;
}

void CMachineState::GetPosServo(CVector& vtPosServo)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
		vtPosServo[ax] = iPosServo[ax] * dPos;
}

void CMachineState::GetPosTrack(CVector& vtPosTrack)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
		vtPosTrack[ax] = iPosTrack[ax] * dPosTrack;
}

void CMachineState::GetPosError(CVector& vtPosError)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
		vtPosError[ax] = iPosError[ax] * dPos;
}


void CMachineState::GetVel(CVector& vtVel)
{
	// could apply correction and use VelTip!
	for (int ax = 0; ax < NUM_AXIS; ax++)
		vtVel[ax] = iVelServo[ax] * dVel;
}

void CMachineState::GetVelServo(CVector& vtVelServo)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
		vtVelServo[ax] = iVelServo[ax] * dVel;
}

void CMachineState::GetVelTrack(CVector& vtVelTrack)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
		vtVelTrack[ax] = iVelTrack[ax] * dVelTrack;
}

void CMachineState::GetVelError(CVector& vtVelError)
{
	for (int ax = 0; ax < NUM_AXIS; ax++)
		vtVelError[ax] = iVelError[ax] * dVel;
}

///////////////////////////////////////////

bool CMachineState::UsingPosFeedback()
{
	int iVal = GetParamValue(GP_ControlCode);
	//int nFeedFoward = iVal & 0x0f;
	int nFeedBack = (iVal >> 4) & 0x0f;
	return (nFeedBack >= 1 && nFeedBack <= 2);
}
