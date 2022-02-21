// PktControl.cpp: implementation of the CPktControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNCControl.h"
#include "PktControl.h"

#include "MachineCodes.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



/////////////////////////////////////////////////////////////
// class CErrorLog
/////////////////////////////////////////////////////////////

/*
CErrorLog::CErrorLog()
{
	m_nLastError = 0;
	m_nErrorCount = 0;
	m_nNewErrorCount = 0;
	const elem = sizeof(m_arErrorTypeCount) / sizeof(*m_arErrorTypeCount);
	for (int i = 0; i < elem; i++)
		m_arErrorTypeCount[i] = 0;
}

char* CErrorLog::m_arErrorDescription[] = {
	"Illegal error 0",
	"Received packet with checksum error",
	"Received packet ID not as expected",
	"Received reply without any request",
	"Received reply with parameter mismatch",

	"Illegal error number >= ERRORMAX"
};

void CErrorLog::LogError(int nError)
{
	if (nError >= ERRORMAX)
		nError = ERRORMAX;
	m_nLastError = nError;
	m_nErrorCount++;
	m_nNewErrorCount++;
	if (++m_arErrorTypeCount[nError] == 0)	// stop an overflow
		m_arErrorTypeCount[nError]--;
	TRACE3("***ERROR***[%i] %s (Total Errors:%i)\n",
			nError, m_arErrorDescription[nError], m_nErrorCount);
}
*/



//////////////////////////////////////////////////////////////////////
// class CPktControl
//////////////////////////////////////////////////////////////////////

// Construction/Destruction

CPktControl::CPktControl()
{
	m_TransmitQueue.SetBufferSize(400);
	m_TransmitHighPriorityQueue.SetBufferSize(20);
	m_ConfirmQueue.SetBufferSize(50);

//	m_bSendingPath = false;
	m_iPathDataLowTrigger = 20;
	m_iRequestPathData = 0;
	m_iPathSegTxCount = 0;
	m_iNumPathSegToTx = 0;

	m_iNextTxID = 0;
	m_iNextRxID = 0;
	m_bNextRxIDValid = false;

}

CPktControl::~CPktControl()
{
}



void CPktControl::SetPathBuffer(CSegAccelFIFO* pFIFO)
{
	m_PathData.SetBuffer(pFIFO);
	ResetPath();
	m_iRequestPathData = 0;
	if (PathDataLow())
		m_iRequestPathData++;
}

void CPktControl::ResetPath()
{
	ASSERT(m_PathData.GetBuffer());
	m_iPathSegTxCount = 0;
	m_PathData.ResetTrackers();
}

void CPktControl::SendPathSegsUpTo(int iMax)
{
	m_iNumPathSegToTx = iMax;
} 

void CPktControl::StopSendingPath()
{
	m_iNumPathSegToTx = 0;
}


int CPktControl::Queue(CPacket& pkt)	// returns remaining buffer space, -1 if not added
{
// Copies pkt into 'm_TransmitQueue' and sets the packet ID of the header and checksum byte
// Packets are transfered to 'TransmittedQueue' on transmission
// and remain in 'TransmittedQueue' until comfirmation is received or errors dealt with

	int iFreeSpace = m_TransmitQueue.Add(pkt);
	if (iFreeSpace < 0)
		TRACE1("TxQueue is full: limit %i packets\n", m_TransmitQueue.GetBufferSize());
	return iFreeSpace;	// not added if returns -1
}

int CPktControl::QueueHighPriority(CPacket& pkt)	// returns remaining buffer space, -1 if not added
{
// Copies pkt into 'm_TransmitQueue' and sets the packet ID of the header and checksum byte
// Packets are transfered to 'TransmittedQueue' on transmission
// and remain in 'TransmittedQueue' until comfirmation is received or errors dealt with

	int iFreeSpace = m_TransmitHighPriorityQueue.Add(pkt);
	if (iFreeSpace < 0)
		TRACE1("TxHighPriorityQueue is full: limit %i packets\n", m_TransmitHighPriorityQueue.GetBufferSize());
	return iFreeSpace;	// not added if returns -1
}

int CPktControl::QueuePathSeg(SSegAccel& sa)		// returns free space in queue
{
	if (SendingPath())
		return -1;			// can't send seperate path packet while sending from buffer
	CPacket pkt;
	sa.iSegCount = m_iPathSegTxCount++;		// keeps track of number of path segs transmitted
	pkt.SendPath(sa);
	return Queue(pkt);
}

int CPktControl::QueueSendParam(int iParam, int iValue)		// returns free space in queue
{
	CPacket pkt;
	pkt.SendParam(iParam, iValue);
	return Queue(pkt);
}

int CPktControl::QueueSendAxisParam(int iAxis, int iParam, int iValue)		// returns free space in queue
{
	CPacket pkt;
	pkt.SendParam(CAxPar(iAxis, iParam), iValue);
	return Queue(pkt);
}

int CPktControl::QueueReqParam(int iParam, int iValue)		// returns free space in queue
{
	CPacket pkt;
	pkt.ReqParam(iParam, iValue);
	return Queue(pkt);
}

int CPktControl::QueueReqAxisParam(int iAxis, int iParam, int iValue)		// returns free space in queue
{
	CPacket pkt;
	pkt.ReqParam(CAxPar(iAxis, iParam), iValue);
	return Queue(pkt);
}

int CPktControl::QueueReqAxisParamAllAxes(int iParam, int iValue)		// returns free space in queue
{
	CPacket pkt;
	int iFree;
	for (int ax = 1; ax <= NUM_AXIS; ax++)
	{
		pkt.ReqParam(CAxPar(ax, iParam), iValue);
		iFree = Queue(pkt);
	}
	return iFree;
}

const CPacket* CPktControl::GetNextPktToTx()
{
/* Move packet from TransmitQueue to ConfirmQueue and return data to transmit
	Packets remain in ConfirmQueue until comfirmation is received or errors dealt with
*/
	if (m_ConfirmQueue.GetFree() < 1)
	{
		LOGERROR("Can't transmit more: m_ConfirmQueue is full");
		return NULL;
	}
	CPacket* const pNextTxPkt = m_ConfirmQueue.GetNextToAdd();
	// try queues in order of priority
	if (!m_TransmitHighPriorityQueue.IsEmpty())
		*pNextTxPkt = m_TransmitHighPriorityQueue.Remove();
	else if (!m_TransmitQueue.IsEmpty())
		*pNextTxPkt = m_TransmitQueue.Remove();
	else if ((m_iNumPathSegToTx > 0) && !m_PathData.IsEmpty())
	{
		m_iNumPathSegToTx--;
		SSegAccel* pSA = m_PathData.GetNext();
		pSA->iSegCount = m_iPathSegTxCount++;	// keeps track of number of path segs transmitted
		pNextTxPkt->SendPath(*pSA);
		int iNumBuff = m_PathData.GetCount();
		if (iNumBuff > m_iPathDataLowTrigger)
			m_iRequestPathData = 0;
		else if (iNumBuff == m_iPathDataLowTrigger ||
					iNumBuff == m_iPathDataLowTrigger/2 ||
					iNumBuff == 0)
			m_iRequestPathData++;
	}
	else			// nothing to transmit
		return NULL;		// m_ConfirmQueue doesn't get changed!

	pNextTxPkt->SetIDandCS(m_iNextTxID++);
	m_ConfirmQueue.AddOne();
	return pNextTxPkt;		// packet is in confirm queue
}

CPacket* CPktControl::GetNextTxdPkt()
{
	if (m_ConfirmQueue.IsEmpty())
		return NULL;
	return &m_ConfirmQueue.GetNext();		// pointer valid till next is removed
}

bool CPktControl::RequestPathData()
{
	if (m_iRequestPathData == 0)
		return false;
	m_iRequestPathData--;
	return true;
}



// returns number of packets discarded
int CPktControl::DiscardUnconfirmed()
{
//	static int iDiscardCount = 0;
	if (m_ConfirmQueue.IsEmpty())
		return 0;
//	pTxdPkt->iStatus = PSTAT_RXCONFIRMATION;
	CPacket& pkt = m_ConfirmQueue.Remove();
	LOGERROR1("Discarding comfirm queue packet: %s", pkt.GetAsString());
//	TRACE1("Timed out on confirmation %i\n", ++iDiscardCount);
	return 1;
}


bool CPktControl::CheckReceived(CPacket& rxPkt)
{
/*	check checksum and expected received ID
	if packet is an error code packet take appropriate action
	if packet is a reply:
		check if is reply to first packet on transmitted queue
		if not check next packet on transmitted queue etc

		if origional found in queue, mark as confirmed - remove if tail of queue
		send message to user thread of confirmed (sent or received) value
	if packet is initiated
		OK
*/
//	if (rxPkt.param == 0x4c && rxPkt.axis == 0x03)
//		int a=2;		// check this packet

	if (!rxPkt.CheckCS())		//	checksum error
	{
		LOGERROR1("Rx packet checksum Pkt: %s", rxPkt.GetAsString());
		return false;
	}
	int iIDdiff = rxPkt.CompareID(m_iNextRxID);
	if (iIDdiff != 0)
		if (!m_bNextRxIDValid)
			m_iNextRxID += iIDdiff;		// set expected to received if not valid
		else
		{
			LOGERROR1("Rx packet ID - aligning expected with received - Pkt: %s", rxPkt.GetAsString());
			m_iNextRxID += iIDdiff;		// set expected to received if not valid
		}
	m_bNextRxIDValid = true;
	m_iNextRxID++;
	m_pTxPktForReply = NULL;


	// Is packet a reply or Initiated
	if (rxPkt.IsReply())
	{
		if (m_ConfirmQueue.IsEmpty())
		{
			LOGERROR1("Rx reply without request Pkt: %s", rxPkt.GetAsString());
			return false;
		}

		CPacket& txPkt = m_ConfirmQueue.GetNext();		// pointer valid till next is removed
		int iValid = IsValidReplyFor(rxPkt, txPkt);
		bool bErrorPkt = rxPkt.IsCommsErrorPacket();
		
		if (iValid == 0)		// not valid, try next on queue
		{
			LOGERROR1("Rx reply parameter mismatch Pkt: %s", rxPkt.GetAsString());	// search through queue to find origionator
			return false;
		}
		m_pTxPktForReply = &txPkt;
		if (iValid == 1)		// valid, just one reply packet
		{
			if (!bErrorPkt)
				txPkt.iStatus = PSTAT_RXCONFIRMATION;
			m_ConfirmQueue.Discard();	// if txPkt is the tail
		}
		else if (iValid == 2)		// all axes expected to reply
			txPkt.iStatus = PSTAT_RXPARTCONF1;
		else if (iValid == 3)		// predefined axes set expected to reply
			txPkt.iStatus = PSTAT_RXPARTCONF1;
		else
			ASSERT(0);
	}
	else	// not reply, was initiated
	{
	}
	return true;
}

// returns 0: for not valid, 1: for valid only one reply, 2: replies from all axes expected, 3:replies from predefined axes set
int CPktControl::IsValidReplyFor(const CPacket& rxPkt, const CPacket& txPkt)
{
	if (!rxPkt.IsReply() || !txPkt.IsInitiated())
		return 0;		// not a reply to an initiated

	bool bSendReqMatch = (rxPkt.IsSend() == txPkt.IsSend());		// check if same operation

	switch (rxPkt.GetSizeType())
	{
	case P0SIZE_BYTE:
		if (txPkt.IsSend())
			return 1;			// if a NOP operation no further checks
		return 0;

	case P0SIZE_DATA:
		if (rxPkt.GetDataType() == P1DATA_COMMSERROR)
			if (rxPkt[3] == txPkt.header)		// error packet contains tx header!
				switch (rxPkt.GetErrorNumber())
				{
				case Error_RxPacketID:
				case Error_ParamNumBad:
					return 1;		// remove pkt from confirm queue - this is only reply to it
				default:
					return 0;
				}
		if (!bSendReqMatch)
			return 0;
		switch (txPkt.GetSizeType())
		{
		case P0SIZE_PATH:
			return 1;
		case P0SIZE_DATA:
			if (rxPkt.GetDataType() != txPkt.GetDataType())
				return 0;
			if (rxPkt.GetDataType() == P1DATA_PARAM)	// parameter packet
			{
				if (rxPkt.GetParam() != txPkt.GetParam())		// params should be equal
					return 0;
				if (rxPkt.GetAxis() == txPkt.GetAxis())	// axes should be equal unless tx axis is 'all axis' or 'predefined' (f,e)
					return 1;
				if (txPkt.GetAxis() == P1AXIS_ALL)
					return 2;
				else if (txPkt.GetAxis() == P1AXIS_SET)
					return 3;
				else
					return 0;
			}
			else		// not parameter packet
				return 1;
		default:
			return 0;
		}
		return 0;

	case P0SIZE_PATH:
		if (!bSendReqMatch)
			return 0;
		return 1;

	case P0SIZE_STREAM:
		if (!bSendReqMatch)
			return 0;
		return 1;

	default:
		return 0;
	}
	return 0;
}

void CPktControl::SetNextTxID(int id)
{
	m_iNextTxID = id;
}







