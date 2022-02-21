// PktControl.h: interface for the CPktControl class.
//			and CPacket class
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PKTCONTROL_H__7DEB7081_A8D1_11D4_8C1E_91D8F02F0E2F__INCLUDED_)
#define AFX_PKTCONTROL_H__7DEB7081_A8D1_11D4_8C1E_91D8F02F0E2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "Packet.h"
#include "FIFOBuffer.h"
#include "ContPathData.h"


typedef TFIFO<CPacket> CPacketFIFO;

/*
enum
{
	ERROR_NOTUSED = 0,
	ERROR_RXPKTCHECKSUM = 1,		// in class C??
	ERROR_RXPKTID,
	ERROR_RXREPLYWITHOUTREQU,
	ERROR_RXREPLYPARAMMISMATCH,

	ERROR_ParamNotFirstWord,		// in class CParamData
	ERROR_ParamTotalBytesMisMatch,
	ERROR_ParamWordNumMisMatch,

	ERROR_ParamBytesRedefined,		// in class CParamLoader


	ERRORMAX
};

class CErrorLog
{
	int m_nLastError;
	int m_nErrorCount;
	int m_nNewErrorCount;
	char m_arErrorTypeCount[ERRORMAX+1];
	static char* m_arErrorDescription[ERRORMAX+1];

public:
	CErrorLog();
	void LogError(int nError);
};
*/


class CPktControl
{
public:
	CPktControl();
	virtual ~CPktControl();

	void SetPathBuffer(CSegAccelFIFO* pFIFO);
	void ResetPath();
	void StopSendingPath();
//	void SendPath(bool bSend) { m_bSendingPath = bSend; }		// allows tx of path segs
	bool SendingPath() { return m_iNumPathSegToTx > 0; }
	void SendPathSegsUpTo(int iMax);
	bool PathDataLow() { return m_PathData.GetCount() <= m_iPathDataLowTrigger; }
	bool RequestPathData();
	int GetPathSegNum() { return m_PathData.GetCurrSegNum(); }		// count of last GetNext()
	int GetPathSegEndTime() { return m_PathData.GetCurrSegEndTime(); }	// end time of last GetNext()
	bool IsPathFinished() { return m_PathData.IsEmpty(); }


	void SetNextTxID(int id);
	int DiscardUnconfirmed();

	int QueueHighPriority(CPacket& pPkt);			// returns remaining space in queue, -1 if not added
	int Queue(CPacket& pPkt);							// returns remaining space in queue, -1 if not added
	int QueuePathSeg(SSegAccel& sa);					// returns free space in queue
	int QueueSendParam(int iParam, int iValue);	// returns free space in queue
	int QueueSendAxisParam(int iAxis, int iParam, int iValue);	// returns free space in queue
	int QueueReqParam(int iParam, int iValue = 0);	// returns free space in queue
	int QueueReqAxisParam(int iAxis, int iParam, int iValue = 0);	// returns free space in queue
	int QueueReqAxisParamAllAxes(int iParam, int iValue = 0);	// returns free space in queue

	
	int GetQueueFreeSpace() { return m_TransmitQueue.GetFree(); }
	const CPacket* GetNextPktToTx();
//	int GetNumToTx() { return m_TransmitQueue.GetCount(); }
	bool CheckReceived(CPacket& rxPkt);
	CPacket* GetTxPktForReply() { return m_pTxPktForReply; }
	CPacket* GetNextTxdPkt();

protected:
	int IsValidReplyFor(const CPacket& rxPkt, const CPacket& txPkt);

public:
//	CErrorLog* m_pMonitorErrors;

protected:
	// Queues to send - in order of priority
	CPacketFIFO m_TransmitHighPriorityQueue;
	CPacketFIFO m_TransmitQueue;
	CContPathData m_PathData;		// accesses path buffer from user thread
//	bool m_bSendingPath;				// only send path data if true
	int m_iPathDataLowTrigger;
	int m_iRequestPathData;
	int m_iPathSegTxCount;		// keeps track of number of path segs transmitted
	int m_iNumPathSegToTx;		// must get set repetedly to allow path segs to tx

	CPacketFIFO m_ConfirmQueue;	// transmitted packets waiting to receive confirmation

	CPacket* m_pTxPktForReply;

	int m_iNextTxID;
	int m_iNextRxID;
	bool m_bNextRxIDValid;
//	int m_iRxBadPacket;		// keeps track of bad rx packets


};

#endif // !defined(AFX_PKTCONTROL_H__7DEB7081_A8D1_11D4_8C1E_91D8F02F0E2F__INCLUDED_)
