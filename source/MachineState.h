// MachineState.h: interface for the CMachineState class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MACHINESTATE_H__61C390C6_704B_42C6_BEE4_D78F5F087422__INCLUDED_)
#define AFX_MACHINESTATE_H__61C390C6_704B_42C6_BEE4_D78F5F087422__INCLUDED_


#include "PathDataObjects.h"
#include "PosConverter.h"
#include "Param.h"
#include "CNCControl.h"


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CMachineState
{
public:
	CMachineState();
	virtual ~CMachineState();

public:
	short m_arParam[(NUM_AXIS+1) * 0x100];			// received values
	short m_arParamSent[(NUM_AXIS+1) * 0x100];	// sent values
	char m_arStateParam[(NUM_AXIS+1) * 0x100];	// param state below

	enum	// State of Parameter value
	{
		ST_NOTSET = 0,
		ST_REQUESTED = 1,
		ST_SENT,
		ST_RECEIVED,			// all received >= this
		ST_RECEIVEDREQU,
		ST_RECEIVEDSENT,
	};


	double dPos;
	double dVel;
	double dAcc;
	double dPosTrack;
	double dVelTrack;
	double dAccTrack;
	double dTime;

	static double m_zAxisPosTipAtBaseTop;		// z pos offset to get tip height from axis pos

	SContBufferSpan PathBuff;

	// full values
	int iTime;
	int iPosServo[NUM_AXIS];
	int iVelServo[NUM_AXIS];
	__int64 iPosTrack[NUM_AXIS];
	int iVelTrack[NUM_AXIS];
	int iPosError[NUM_AXIS];
	int iVelError[NUM_AXIS];

	// pos corrected values
	CVector vtPosTip;			// true tip position after pos correction
	CVector vtPosTipTrack;			// true tip position after pos correction

	// probe values
//	int iProbeZ;
	double ProbeZ;				// actual reading
	double ProbeZExt;			// extension distance from base
	bool m_bProbeReadError;
	CVector vtPosProbeTip;

public:

	// functions for reporting parameter values and request/send states
	void SetRequested(CAxPar axpar) { m_arStateParam[axpar] = ST_REQUESTED; }
	void SetRequestedAllAxis(int iParam)
		{ for (int ax = 1; ax <= NUM_AXIS; ax++) m_arStateParam[CAxPar(ax, iParam)] = ST_REQUESTED; }

	void SetSent(CAxPar axpar, int iVal) { m_arStateParam[axpar] = ST_SENT; m_arParamSent[axpar] = (short)iVal; }
	
	void SetReceivedRequ(CAxPar axpar, int iVal) { m_arStateParam[axpar] = ST_RECEIVEDREQU; m_arParam[axpar] = (short)iVal; }
	void SetReceivedSent(CAxPar axpar, int iVal) { m_arStateParam[axpar] = ST_RECEIVEDSENT; m_arParam[axpar] = (short)iVal; }
	void SetReceivedSent(CAxPar axpar) { m_arStateParam[axpar] = ST_RECEIVEDSENT; m_arParam[axpar] = m_arParamSent[axpar]; }

	bool IsReceived(CAxPar axpar) { return m_arStateParam[axpar] >= ST_RECEIVED; }
	bool IsReceivedRequ(CAxPar axpar) { return m_arStateParam[axpar] == ST_RECEIVEDREQU; }
	bool IsReceivedSent(CAxPar axpar) { return m_arStateParam[axpar] == ST_RECEIVEDSENT; }

	short GetParamValue(CAxPar axpar) { return m_arParam[axpar]; }
	short GetParamValue(int ax, int par) { return m_arParam[CAxPar(ax, par)]; }
	short& ParamValueSent(CAxPar axpar) { return m_arParamSent[axpar]; }
	short& ParamValueSent(int ax, int par) { return m_arParamSent[CAxPar(ax, par)]; }


	void SetZAxisPosTipAtBaseTop(double zAxisPos) { m_zAxisPosTipAtBaseTop = zAxisPos; }

	void OnUpdatePosServo();
	void OnUpdatePosProbe();
	void OnUpdatePosTrack();

	void GetPosTipRelBase(CVector& vtPos);		// tip pos relative to base
	void GetPosTip(CVector& vtPos);				// tip pos
	void GetPosTipTrackRelBase(CVector& vtPos);
	void GetPosTipTrack(CVector& vtPos);
	void GetPosHead(CVector& vtPos);				// servo position corrected at tip
	void GetPosProbeRelBase(CVector& vtPos);	// tip pos relative to base
	void GetPosProbe(CVector& vtPos);			// tip pos
	void GetPosServo(CVector& vtPosServo);		// servo pos
	void GetPosTrack(CVector& vtPosTrack);		// servo track pos
	void GetPosError(CVector& vtPosError);
	double GetPosServo(int ax) { return iPosServo[ax] * dPos; }
	double GetPosTrack(int ax) { return iPosTrack[ax] * dPosTrack; }
	double GetPosError(int ax) { return iPosError[ax] * dPos; }

	void GetVel(CVector& vtVel);
	void GetVelServo(CVector& vtVelServo);
	void GetVelTrack(CVector& vtVelTrack);
	void GetVelError(CVector& vtVelError);
	double GetVelServo(int ax) { return iVelServo[ax] * dVel; }
	double GetVelTrack(int ax) { return iVelTrack[ax] * dVelTrack; }
	double GetVelError(int ax) { return iVelError[ax] * dVel; }

	double GetProbeReadingZ() { return ProbeZ; }

	bool UsingPosFeedback();

};



#endif // !defined(AFX_MACHINESTATE_H__61C390C6_704B_42C6_BEE4_D78F5F087422__INCLUDED_)
