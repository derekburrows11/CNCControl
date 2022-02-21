// MachineSim.h: interface for the CMachineSim class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MACHINESIM_H__A0489240_BD21_11D7_86C3_E7E33F909E24__INCLUDED_)
#define AFX_MACHINESIM_H__A0489240_BD21_11D7_86C3_E7E33F909E24__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Packet.h"


typedef TFIFO<CPacket> CPacketFIFO;


struct SServoState
{
	double v, i, w, p;		// voltage, current, speed, pos
};

class CServoModel
{
	double R, L, E, T, Mr, F0, F1;		// resistance, inductance etc. See Formula Motor Control.txt
	double c0, c1, c2;					// precalced coefficents
	double a, b;				// two poles
	double c;					// initial rate of change for current
	double Ai, Bi, Aw, Bw;	// proportion of a&b exp's for w and i eg: [Aw.exp(-at) + Bw.exp(-bt)].Wss
	double dT;					// step time (~1ms)

	double Fa, Fb, Fc, Fd, Gb, Gc;
	double WaW1, WaI1, WaU1, IaW1, IaI1, IaU1;
	double WbW1, WbW2, WbU1, WbU2;
	double IcW0, IcW1, IcU1;


	SServoState m_arState[5];	// store last 5 states
	SServoState* m_pState[5];	// pointers to last 5 states
	SServoState *pS0, *pS1, *pS2, *pS3, *pS4;	// pointers to last 5 states

	void SetCoefficents();
	void FindStepVoltageFor(SServoState nextState);
	void CalcCurrent2S1V(SServoState nextState);

};

class CMachineServoModel
{
//	SServoState m_State[5][NUM_AXIS];	// store 5 last states
//	SServoParam m_Param[NUM_AXIS];
	double m_dt;

	void ApplySteadyVolts(double* arVolts);





};


// Machine Simulator

class CMachineSim  
{
public:
	CMachineSim();
	virtual ~CMachineSim();

	// the following 2 functions all call DoTimeCheck()
	void SendPacket(const CPacket& sentPkt);
	bool GetReturnedPacket(CPacket& returnPkt);
	//

private:
// Tx/Rx packets
	CPacket m_RxPkt;
	CPacket m_ResponsePkt;
	int m_iNextRxID;
	int m_iNextTxID;
	CPacketFIFO m_TransmitQueue;


	// path segments
	int m_iNumAxis;
	CSegAccelFIFO m_SegAccelFIFO;
	SSegAccel m_SegAccCurr;
	int m_iSegCountCurr;
	SMotionI m_MotICurr;			// intergral value of machine state
	SMotionI m_MotIEndBuff;		// intergral value of machine state at end of buffer
	int m_iTimeEndBuffer;
	int m_iSegCountEndBuffer;
	int m_iBuffFreeLastNotify;

	// path driving
	bool m_bDrivingPath;
	DWORD m_iTickPathStart;		// time in ms from GetTickCount()
	DWORD m_iTickRealTimeZero;
	int m_iTickLastCheck;
	int m_iTimePathLastSendPos;

// parameter storage
	int m_iReqParamValue;
	short m_arAxParam[0x0400];		// indexed by CAxPar


// functions
	void DoTimeCheck();

	void Reset();
	void ResetPath();
	void StartPath();
	void StopPath();

	void HandlePathPacket();
	void HandleDataPacket();

	void DoPathIncUpdate();

//	bool QueuePacket(CPacket& pkt) { return m_TransmitQueue.Add(pkt) >= 0; }
};

#endif // !defined(AFX_MACHINESIM_H__A0489240_BD21_11D7_86C3_E7E33F909E24__INCLUDED_)
