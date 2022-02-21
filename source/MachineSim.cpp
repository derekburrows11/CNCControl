// MachineSim.cpp: implementation of the CMachineSim class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"
#include "MachineSim.h"

#include "MachineCodes.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMachineSim::CMachineSim()
{
	m_iNumAxis = 3;
	m_iTickRealTimeZero = GetTickCount();
	for (int i = sizeof(m_arAxParam)/sizeof(*m_arAxParam); --i >= 0;)
		m_arAxParam[i] = 0;
	Reset();
}

CMachineSim::~CMachineSim()
{
}

void CMachineSim::Reset()
{
	m_iReqParamValue = 1;
	m_iNextRxID = -1;	// not set yet
	m_iNextTxID = 3;	// start different to recieved ID
	m_TransmitQueue.RemoveAll();
	m_TransmitQueue.SetBufferSize(20);

	ResetPath();

	m_bDrivingPath = false;
	m_iTickPathStart = (DWORD)-1;
	m_iTickLastCheck = 0;

	m_MotICurr.Zero();
	m_MotIEndBuff.Zero();
}

void CMachineSim::ResetPath()
{
	m_SegAccelFIFO.RemoveAll();
	m_SegAccelFIFO.SetBufferSize(500);
	m_iSegCountCurr = 0;
	m_iTimeEndBuffer = 0;
	m_iSegCountEndBuffer = -1;		// zero based count
	m_iBuffFreeLastNotify = 0;

	m_MotICurr.Zero();
	m_MotIEndBuff.Zero();
}


void CMachineSim::StartPath()
{
	DoTimeCheck();

	ASSERT(!m_bDrivingPath);
	if (m_bDrivingPath)
		return;
	int numSegs = m_SegAccelFIFO.GetCount();
	if (numSegs == 0)
	{
		// send error message - no segs
		return ;
	}
	m_MotICurr.ZeroVel();
	m_MotICurr.ZeroAcc();
	m_MotICurr.iTime = 0;
	m_SegAccCurr = m_SegAccelFIFO.Remove();
	m_iSegCountCurr++;

	m_bDrivingPath = true;
	m_iTickPathStart = GetTickCount();
	m_iTimePathLastSendPos = -5000;		// -5sec so will send pos next check!
	DoTimeCheck();
}

void CMachineSim::StopPath()
{
	DoTimeCheck();

	if (!m_bDrivingPath)
	{
		// send error message - not driving
		return;
	}
	m_bDrivingPath = false;
}

void CMachineSim::DoTimeCheck()
{
// Update real time
	int iTick = GetTickCount();
	if (iTick == m_iTickLastCheck)
		return;
	m_iTickLastCheck = iTick;

	int iTime100Sec = (iTick - m_iTickRealTimeZero) / 10;
	int iTimeSec = iTime100Sec / 100;
	iTime100Sec %= 100;
	CAxPar axPar0(0, GP_RealTimeSecW0);
	CAxPar axPar1(0, GP_RealTimeSecW1);
	m_arAxParam[axPar0] = MAKEWORD(iTime100Sec, iTimeSec);	// low byte is 100'ths sec, high byte is sec
	m_arAxParam[axPar1] = (short)(iTimeSec >> 8);

	if (!m_bDrivingPath)
		return;

// Check state of path and path buffer
	int iTimePath = iTick - m_iTickPathStart;	// time in ms
	int segsBefore = m_SegAccelFIFO.GetCount();
	while (m_MotICurr.iTime + m_SegAccCurr.idTime <= iTimePath)
	{
		m_MotICurr.UpdateMotion(m_SegAccCurr);		// will set .iTime to end of segment
		if (m_SegAccelFIFO.Remove(m_SegAccCurr))
			m_iSegCountCurr++;
		else
		{				// end of path segments - m_SegAccCurr not valid anymore!
			m_bDrivingPath = false;		// send end of data packet
			break;
		}
	}
	int segsAfter = m_SegAccelFIFO.GetCount();

	// calc pos, vel & acc and store
	SMotionI motI = m_MotICurr;
	int iTimeSeg = iTimePath - motI.iTime;		// not used!
	if (m_bDrivingPath)
		motI.UpdateMotionAtPathTime(m_SegAccCurr, iTimePath);
	for (int ax = 1; ax <= m_iNumAxis; ax++)
	{
		int iOffset = CAxPar(ax, 0);
		m_arAxParam[iOffset + AP_PosTrackW0] = *((short*)&motI.pos[ax] + 0);	// compiles same as without +0 or just casting as (short)
		m_arAxParam[iOffset + AP_PosTrackW1] = *((short*)&motI.pos[ax] + 1);
		m_arAxParam[iOffset + AP_PosTrackW2] = *((short*)&motI.pos[ax] + 2);
		m_arAxParam[iOffset + AP_VelTrackW0] = *((short*)&motI.vel[ax] + 0);
		m_arAxParam[iOffset + AP_VelTrackW1] = *((short*)&motI.vel[ax] + 1);
		m_arAxParam[iOffset + AP_AccTrackW0] = *((short*)&motI.acc[ax] + 0);
		m_arAxParam[iOffset + AP_AccTrackW1] = *((short*)&motI.acc[ax] + 1);
	}

	CPacket pkt;
	int iBuffFree = m_SegAccelFIFO.GetFree();
	if (abs(iBuffFree - m_iBuffFreeLastNotify) >= 10)	// notify if changed 10 or more
	{
		m_iBuffFreeLastNotify = iBuffFree;
		if (m_TransmitQueue.GetFree() >= 2)
		{
			pkt.SendParam(GP_PathBuffEndSegNum, m_iSegCountEndBuffer);
			m_TransmitQueue.Add(pkt);
			pkt.SendParam(GP_PathBuffFreeObjs, iBuffFree);
			m_TransmitQueue.Add(pkt);
		}
		else
			ASSERT(0);
	}

	// send current position and velocity regularly
	if (iTimePath - m_iTimePathLastSendPos >= 200 || !m_bDrivingPath)	// >= 200ms
	{
		m_iTimePathLastSendPos = iTimePath;
		int arPos[NUM_AXIS];
		int arVel[NUM_AXIS];
		for (ax = 0; ax < m_iNumAxis; ax++)
		{
			arPos[ax] = *(int*)((char*)&motI.pos[ax] + 2);	// select part of __int64!
			arVel[ax] = *(int*)((char*)&motI.vel[ax] + 1);
		}
		pkt.SendPathPos();
		pkt.SetPathValues(arPos);
		m_TransmitQueue.Add(pkt);
		pkt.SendPathVel();
		pkt.SetPathValues(arVel);
		m_TransmitQueue.Add(pkt);
	}

	if (!m_bDrivingPath)		// send finished path packet
	{
		LOGEVENT("DoTimeCheck() - sending finished path packet");
		pkt.SendParam(GP_PathProcess, 0x10);	// 0x10 finished normally
		m_TransmitQueue.Add(pkt);
		pkt.SendParam(GP_PathCurrSegNum, m_iSegCountCurr);
		m_TransmitQueue.Add(pkt);
	}
}

void CMachineSim::DoPathIncUpdate() // not finished
{
	// inc path time by 1ms
	// calc path motion for next step
	// read actual motors positions
	// calc and apply motor voltages for next step
#if 0
	int iTimeSeg = iTimePath - motI.iTime;		// not used!
	motI.UpdateMotionAtPathTime(m_SegAccCurr, iTimePath);

	SMotionI motI0;	// current motion
	SMotionI motI1;	// next motion
	int dt = motI1.iTime - motI0.iTime;

/*
	for Feed Foward voltage
	use required vel and acc to find required w & i
	w = 2pi/2.5mm * vel
	dw/dt = 2pi/2.5mm * acc
	dw/dt*Mr = T*i - F1*w - F0*sign(w)
	T*i = dw/dt*Mr + F1*w + F0*sign(w)

	w1 = [w0

*/

	m_ServoModel.ApplySteadyVolts(volts);

#endif

}

void CMachineServoModel::ApplySteadyVolts(double* arVolts)
{
	arVolts[0] = 0;	// just a filler
}

void CServoModel::SetCoefficents()
{
/*
	F =
	[ -Fa   Fb ]
	[ -Fc  -Fd ]

	G =
	[  0   -Gb ]
	[  Gc   0  ]
*/
	Fa = F1/Mr;
	Fb = T/Mr;
	Fc = E/L;
	Fd = R/L;
	Gb = F0/Mr;
	Gc = 1/L;

	double b24ac = (Fa-Fd)*(Fa-Fd) - 4*Fb*Fc;
	ASSERT(b24ac >= 0);
	b24ac = sqrt(b24ac);
	a = (Fa+Fd - b24ac) * 0.5;		// roots are -(Fa+Fd), a & b are pos values
	b = (Fa+Fd + b24ac) * 0.5;
	//	NOTE: b > a
/*
	c = di(0)/dt / Iss
	c	= Fb.Fc + Fa.Fd  =  T.E + F1.R	if Gb == 0 (non-linear term)
		  -------------     ----------
		     Fa               F1.L
*/
	c = (Fb*Fc + Fa*Fd) / Fa;

// w0 = WaW1.w1 + WaI1.i1 + WaU1.u1					...1
// i0 = IaW1.w1 + IaI1.i1 + IaU1.u1					...2
	WaW1 = 0;
	WaI1 = 0;
	WaU1 = 0;
	IaW1 = 0;
	IaI1 = 0;
	IaU1 = 0;


// w0 = WbW1.w1 + WbW2.w2 + WbU1.u1 + WbU2.u2	...3
// i0 = IbI1.i1 + IbI2.i2 + IbU1.u1 + IbU2.u2	...4
// w0 = (WaW1 + IaI1).w1 + (WaI1.IaW1 - IaI1.WaW1).w2 + WaU1.u1 + (WaI1.IaU1 - IaI1.WaU1).u2
	WbW1 = WaW1 + IaI1;
	WbW2 = WaI1*IaW1 - IaI1*WaW1;
	WbU1 = WaU1;
	WbU2 = WaI1*IaU1 - IaI1*WaU1;

// for current from previous two speeds and previous voltage
// i0 = { IaI1.w0 + (WaI1.IaW1 - IaI1.WaW1).w1 + (WaI1.IaU1 - IaI1.WaU1).u1 } / WaI1
	IcW0 = IaI1 / WaI1;
	IcW1 = (WaI1*IaW1 - IaI1*WaW1) / WaI1;
	IcU1 = (WaI1*IaU1 - IaI1*WaU1) / WaI1;
}

void CServoModel::CalcCurrent2S1V(SServoState /*nextState*/)
{
	// Estimate current from previous two speeds and previous voltage
	pS0->i = IcW0*pS0->w + IcW1*pS1->w + IcU1*pS1->v;
}

void CServoModel::FindStepVoltageFor(SServoState /*nextState*/)
{
	// to go into "SetConstants"
	double expat = exp(-a*dT);
	double expbt = exp(-b*dT);
	double onemexpat = 1 - expat;
	double onemexpbt = 1 - expbt;

	Aw = b / (b-a);		// as initial dw/dt = 0 (no zeros)
	Bw = a / (a-b);

	double c = 0;	// V / (Iss * L);
	Ai = (b-c) / (b-a);
	Bi = (a-c) / (a-b);

	SServoState& s0 = *pS0;
	// calc seperate a&b exp components
	double wa = ((F1/Mr - b) * s0.w - (T/Mr) * s0.i) / (a-b);
	double wb = ((F1/Mr - a) * s0.w - (T/Mr) * s0.i) / (b-a);

	// w1 is next w
	double w1 = (F1/Mr*(expat-expbt) + (a*expbt-b*expat)) * s0.w
					- T/Mr*(expat-expbt)*s0.i;		// due to current state
//	w1 /= (a-b);
//	w1 += (Ai*onemexpat + Bi*onemexpbt) * Wss/v * v;	// due to next input

//	double i1 = 
}


void CMachineSim::SendPacket(const CPacket& sentPkt)
{
	DoTimeCheck();

	// don't change responsePkt till end - it may be rxPkt!
	m_RxPkt = sentPkt;
	if (!m_ResponsePkt.GetReplyFor(sentPkt))		// leave as is if doesn't get reply
	{
		m_ResponsePkt = sentPkt;
		return;
	}

	switch (sentPkt.GetSizeType())
	{
	case P0SIZE_PATH:		HandlePathPacket(); break;
	case P0SIZE_DATA:		HandleDataPacket(); break;
	case P0SIZE_BYTE:		break;
	case P0SIZE_STREAM:	break;
	}

	if (m_ResponsePkt.GetSize() != 0)
	{
		ASSERT(m_TransmitQueue.GetFree() >= 1);
		m_TransmitQueue.Add(m_ResponsePkt);
	}
}

bool CMachineSim::GetReturnedPacket(CPacket& returnPkt)
{
	DoTimeCheck();
	if (m_TransmitQueue.IsEmpty())
		return false;
	returnPkt = m_TransmitQueue.Remove();
	returnPkt.SetIDandCS(m_iNextTxID++);
	return true;
}

void CMachineSim::HandlePathPacket()
{
	if (m_RxPkt.IsSend())
	{
		if (m_SegAccelFIFO.GetFree() >= 1)		// sent an accel segment
		{
			SSegAccel* pSA = m_SegAccelFIFO.GetNextToAdd();
			m_RxPkt.GetSegAccel(*pSA);
			// keep track of end of buffer counts
			m_iTimeEndBuffer += pSA->idTime;
			m_iSegCountEndBuffer++;
			pSA->iTimeEnd = m_iTimeEndBuffer;
			ASSERT(pSA->iSegCount == (m_iSegCountEndBuffer & 0x0f));	// packet only contains low 4 bits of seg count
			pSA->iSegCount = m_iSegCountEndBuffer;
			m_SegAccelFIFO.AddOne();
			m_MotIEndBuff.UpdateMotion(*pSA);
		}
		else		// reply with buffer full error msg
		{
			m_ResponsePkt.ReturnError(Error_PathBufferFull, *(short*)&m_RxPkt.inst0);
		}
	}
	else if (m_RxPkt.IsRequest())	// will be!
		ASSERT(0);		

}

// see "CNC.par" for parameter details

void CMachineSim::HandleDataPacket()
{
	if (m_RxPkt.GetDataType() == P1DATA_PARAM)
	{
		CAxPar axPar = m_RxPkt.GetAxPar();
		if (m_RxPkt.IsSend())
		{
			short iVal = m_RxPkt.GetValue();
			if (axPar < sizeof(m_arAxParam) / sizeof(*m_arAxParam))
				m_arAxParam[axPar] = iVal;	// set corresponding array value
			if (axPar.IsGeneralParam())
				switch (axPar.param)
				{
				case GP_PathBuffMaxObjs:
					if (m_SegAccelFIFO.IsEmpty())
						m_SegAccelFIFO.SetBufferSize(iVal);
					break;
				case GP_PathBuffNumObjs:
					break;
				case GP_PathProcess:
					if (iVal != 0)
						StartPath();
					else
						StopPath();
					break;
				case GP_PathReset:
					ResetPath();
					break;
				}
			else		// is axis param
			{
				if (axPar.IsAllAxis())
					for (int ax = 1; ax <= m_iNumAxis; ax++)
						m_arAxParam[CAxPar(ax, axPar.param)] = iVal;
			}
		}
		else if (m_RxPkt.IsRequest())	// will be!
		{
			int iVal = 0;
			if (axPar < sizeof(m_arAxParam) / sizeof(*m_arAxParam))
				iVal = m_arAxParam[axPar];
			if (axPar.IsGeneralParam())
				switch (axPar.param)
				{
				case GP_PathBuffMaxObjs:
					iVal = m_SegAccelFIFO.GetMaxSize(); break;
				case GP_PathBuffNumObjs:
					iVal = m_SegAccelFIFO.GetCount(); break;
				case GP_PathBuffFreeObjs:
					iVal = m_SegAccelFIFO.GetFree(); break;
				}
			else		// is axis param
				switch (axPar.param)
				{
				case 0: break;
				}
			m_ResponsePkt.SetValue(iVal);
		}
	}
}



