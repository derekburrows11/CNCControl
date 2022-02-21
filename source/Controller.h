// Controller.h: interface for the CController class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTROLLER_H__803AC6E5_9E8D_11D4_8C1E_DF2EB3D90C2F__INCLUDED_)
#define AFX_CONTROLLER_H__803AC6E5_9E8D_11D4_8C1E_DF2EB3D90C2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "PktControl.h"	// Added by ClassView
#include "MachineSim.h"
#include "MachineTracker.h"

class CParamDoc;

#define MAX_PACKET_SIZE  PKTSIZE_MAX

UINT MonitorComms(LPVOID pParam);


typedef TFIFO<BYTE> CByteFIFO;		// replaces CThreadDataExchange

class CCtrlThreadInfo : public CObject
{
	DECLARE_DYNAMIC(CCtrlThreadInfo)
public:
	HANDLE m_hComm;
	CWinThread* m_pUserThread;

	CParamDoc* m_pParamDoc;
	CByteFIFO* m_pTransmittedData;
	CByteFIFO* m_pReceivedData;
};






class CController  
{
public:
	CController();
	virtual ~CController();

// functions
	bool SetComm(HANDLE hComm);
	bool SetPathBuffer(void* pBuffer);

	int RunMonitor();

// data
	CParamDoc* m_pParamDoc;
	CWinThread* m_pUserThread;
	CByteFIFO* m_pTransmittedData;
	CByteFIFO* m_pReceivedData;


protected:
// data

	// Byte Tx queue (serial comms file)
	BYTE* m_pTxFileBuffer;
	int m_sizeTxFileBuffer;
	int m_iBytesToSendTxFile;
	int m_iPacketsToSendTxFile;

	bool m_bStepPackets;
	int m_iStepPacketsCount;


	bool m_bTxPending;
	CPktControl m_PktControl;
	bool m_bMonitorActive;
	HANDLE m_hComm;
	OVERLAPPED m_olWrite, m_olRead;
	bool m_bLogCommsData;

	CPacket m_RxPkt;
	char m_arRxData[MAX_PACKET_SIZE];
	int m_iRxDataSize;
	int m_iExpRxPktSize;
	DWORD m_iPktBytesToRead;

	// values for simulating machine response
	bool m_bSimulateMachineResponse;
	CMachineSim m_MachineSim;

	// packet counters
	int m_iTxPktCount;
	int m_iRxPktCount;


	// Machine State tracking
	CMachineTracker m_MachTrack;


	// times
	int m_iTimeTxLastPkt;
	int m_iTimeMachineStateUpdate;



protected:
// functions
	// machine send/request functions

	// queueing data
	int QueuePacket(CPacket& pkt) { return m_PktControl.Queue(pkt); }	// returns free space in queue
	int QueueHPPacket(CPacket& pkt) { return m_PktControl.QueueHighPriority(pkt); }	// returns free space in queue
//	int GetQueueFreeSpace() { return m_PktControl.GetQueueFreeSpace(); }		// returns free space in queue
	void SendPacketsContinuous();
	void SendPacketsStep(int iNum);

	bool SendAllGeneralParam();
	bool SendAllAxisParam();
	bool RequestAllGeneralParam();
	bool RequestAllAxisParam();
	

	bool RequestNextPathData();
	bool RequestAllPathData();

	bool CheckSetRAMLocation(int iAddr);




	bool TxQueuedData();
	bool TxBytes(const BYTE* pData, int nBytes);
	void SendBytes(const BYTE* pData, int nBytes);	// from direct user request
	void InitReadFile();
	bool GetReceivedData();

	// machine simulating functions
	bool SimulateMachineResponse();
	bool GetSimulatedMachineResponse();

	// general handling functions
	bool HandleMessage();
	bool HandleRxPacket();

	// checks
	bool DoTimeOutCheck();
	bool DoConfirmCheck();

	void DoUserRequest(int iRequest, long lData);
	void NotifyUserThread(int iNotification, int iData = 0);



};

#endif // !defined(AFX_CONTROLLER_H__803AC6E5_9E8D_11D4_8C1E_DF2EB3D90C2F__INCLUDED_)
