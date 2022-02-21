// Controller.cpp: implementation of the CController class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "CNCControl.h"

#include "ParamDoc.h"
#include "Controller.h"
#include "ThreadMessages.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CString& GetCommErrorDesc(DWORD nErrors)
{
	static CString szErrors;
	szErrors.Format("Comm Error: 0x%02x", nErrors);
	if (nErrors & CE_RXOVER)
		szErrors += ", Receive Queue overflow";
	if (nErrors & CE_OVERRUN)
		szErrors += ", Receive Overrun Error";
	if (nErrors & CE_RXPARITY)
		szErrors += ", Receive Parity Error";
	if (nErrors & CE_FRAME)
		szErrors += ", Receive Framing error";
	if (nErrors & CE_BREAK)
		szErrors += ", Break Detected";
	if (nErrors & CE_TXFULL)
		szErrors += ", TX Queue is full";
	return szErrors;
}



UINT MonitorComms(LPVOID pParam)
{
	CCtrlThreadInfo* pInfo = (CCtrlThreadInfo*)pParam;
	if (pInfo == NULL ||
		!pInfo->IsKindOf(RUNTIME_CLASS(CCtrlThreadInfo)))
	return (UINT)-1;    // illegal parameter

	CController cncCtrl;
	// Set info passed from user thread
	cncCtrl.SetComm(pInfo->m_hComm);
	cncCtrl.m_pUserThread = pInfo->m_pUserThread;
	cncCtrl.m_pParamDoc = pInfo->m_pParamDoc;
	cncCtrl.m_pTransmittedData = pInfo->m_pTransmittedData;
	cncCtrl.m_pReceivedData = pInfo->m_pReceivedData;

	delete pInfo;		// Was created by User thread

	cncCtrl.RunMonitor();

	return 0;
}

IMPLEMENT_DYNAMIC(CCtrlThreadInfo, CObject)



//////////////////////////////////////////////////////////////////////
// class CController
//////////////////////////////////////////////////////////////////////

// Construction/Destruction

CController::CController()
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_pTransmittedData = NULL;
	m_pReceivedData = NULL;
	m_pUserThread = NULL;
	m_pParamDoc = NULL;

	m_bLogCommsData = false;

	// tx/rx tracking
	m_iTxPktCount = 0;
	m_iRxPktCount = 0;

//	m_PktControl.m_pMonitorErrors = &m_MonitorErrors;
	m_bStepPackets = false;


	m_sizeTxFileBuffer = 500;		// ~0.25sec worth of transmitting
	m_iBytesToSendTxFile = m_sizeTxFileBuffer;
	m_pTxFileBuffer = NULL;
	m_iPacketsToSendTxFile = 5;

	// values for simulating response
	m_bSimulateMachineResponse = false;






}

CController::~CController()
{
	delete[] m_pTxFileBuffer;
}

//////////////////////////////////////////////////////////////////////


bool CController::SetComm(HANDLE hComm)
{
	if (m_hComm == INVALID_HANDLE_VALUE)
		m_hComm = hComm;
	else
	{
		ASSERT(0);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

/* 
Monitor function waits for one of the following events:
	- A received byte on serial port
	- Transmit queue empties
	- A user request from the user thread
	- A Timeout to check status ~2 sec

Action on event:
	- A received byte on serial port
			Got full packet? N -> go wait for event again
			Interperate packet and either:	
				queue more path packets if ready
				notify user of set/request result
				notify user of machine state
			
	- Transmit queue empties
			Transmit up to X packets if any waiting in packet queue
	
	- A user request from the user thread
			Get/Send Param -> queue packet (in user queue?)
			Start Sending Path Data
			Stop Path

	- A Timeout to check status ~2 sec
			Check that there are no Rx bytes and event status is correct
*/
int CController::RunMonitor()
{
	m_pTransmittedData->SetBufferSize(500);
	m_pReceivedData->SetBufferSize(500);
	if (m_pTxFileBuffer == NULL)
		m_pTxFileBuffer = new BYTE[m_sizeTxFileBuffer];

	m_MachTrack.m_pPktControl = &m_PktControl;
	m_MachTrack.m_pParamDoc = m_pParamDoc;
	m_MachTrack.m_pUserThread = m_pUserThread;
	
	m_olWrite.hEvent = CreateEvent(NULL, false, false, NULL);	// create set for test
	m_olRead.hEvent  = CreateEvent(NULL, false, false, NULL);
	const int numEvents = 2;
	HANDLE events[numEvents];
	events[0] = m_olRead.hEvent;
	events[1] = m_olWrite.hEvent;

//	UINT hReplyTimer = SetTimer(NULL, 2, 1000, NULL);		// 50ms timer
//	if (hReplyTimer == 0)
//		TRACE0("Timer Failed\n");

	m_bMonitorActive = true;
	m_bTxPending = false;

	InitReadFile();			// sets up event on received data

/*	__int64 iFreq, iCount, iCount0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&iFreq);
	double freq = iFreq * 1e-3;
	QueryPerformanceCounter((LARGE_INTEGER*)&iCount);
	double count1 = (iCount - iCount0) / freq;
*/
	DWORD iTimeOut = 500;		// Time-out in milliseconds
	while(m_bMonitorActive)
	{
		DWORD waitRes = MsgWaitForMultipleObjects(
			numEvents, events, false, iTimeOut, QS_ALLINPUT);	// QS_POSTMESSAGE also works
		switch (waitRes)
		{
		case WAIT_OBJECT_0 + 0:		// RxByte:
			GetReceivedData();
			break;
		case WAIT_OBJECT_0 + 1:		// TxEmpty:
			m_bTxPending = false;
			TxQueuedData();
			break;
		case WAIT_OBJECT_0 + 2:		// Thread Message:
			while (HandleMessage());	// may be more than one message
			break;
		case WAIT_TIMEOUT:
			DoTimeOutCheck();
			break;
		case -1:
			TRACE0("Error with thread wait\n");
		default:
			ASSERT(0);
		}
		if (!m_bMonitorActive)
			break;

		// regular checks
		if (m_bSimulateMachineResponse)
			while (GetSimulatedMachineResponse())
				HandleRxPacket();

		DoConfirmCheck();
		m_MachTrack.CheckPathStage();
		m_MachTrack.CheckInitContact();

		if (!m_bTxPending)
			TxQueuedData();	// check if any data to send if none being sent


		// set machine state details
		int iTick = GetTickCount();
		if (unsigned(iTick - m_iTimeMachineStateUpdate) >= 400)
		{
			m_iTimeMachineStateUpdate = iTick;
			NotifyUserThread(CN_UPDATE_MACHINESTATEINFO, (int)&m_MachTrack.GetMachState());	// notify of new machine state
		}
	}	// while(m_bMonitorActive)


	CloseHandle(m_olRead.hEvent);
	CloseHandle(m_olWrite.hEvent);
	TRACE("CController::RunMonitor() returning\n");
	return 0;
}

bool CController::SetPathBuffer(void* pBuffer)		// returns true if buffer ok
{
	m_PktControl.SetPathBuffer((CSegAccelFIFO*) pBuffer);
	return true;
}

bool CController::TxQueuedData()
{
// Send up to X queued packets if packet queue not empty
// write <=0.5sec of bytes to comms file as nothing else can be sent immediatly until tx complete

	ASSERT(!m_bTxPending);
	int iNumPkts = 0;
	int iNumBytes = 0;
	int iPacketsToSendTxFile = m_iPacketsToSendTxFile;
	if (m_bStepPackets)
	{
		iPacketsToSendTxFile = m_iStepPacketsCount;
		m_iStepPacketsCount = 0;
	}

	const CPacket* pPkt;
	for (;;)
	{
		if (iNumPkts >= iPacketsToSendTxFile)
			break;
		if (iNumBytes > m_iBytesToSendTxFile - PKTSIZE_MAX)
			break;
		pPkt = m_PktControl.GetNextPktToTx();
		if (pPkt == NULL)
			break;
		memcpy(m_pTxFileBuffer + iNumBytes, pPkt->pktData, pPkt->iSize);
		iNumBytes += pPkt->iSize;
		iNumPkts++;
		if (m_bLogCommsData)
		{
			m_pTransmittedData->Add(pPkt->pktData, pPkt->iSize);
			NotifyUserThread(CN_UPDATE_TXLOG, pPkt->iSize);
		}
	}
	if (iNumBytes != 0)
	{
		TxBytes(m_pTxFileBuffer, iNumBytes);
		m_iTimeTxLastPkt = GetTickCount();
	}
	m_iTxPktCount += iNumPkts;

//	static int cnt;
//	TRACE2("Transmitting more data(%i) %i bytes\n", ++cnt, iNumBytes);
//	if (iNumBytes == 0)
//		iNumBytes = 0;
	if (m_PktControl.RequestPathData())
		NotifyUserThread(CN_PATHBUFFER_LOW, 0);	// send message to fill path seg buffer, 0 means normal

	return true;
}

/*
bool CController::TxNextQueuedData()
{
	if (m_bTxPending)
		return false;
// Send only 1 queued packet if packet queue not empty
	const CPacket* pPkt = m_PktControl.GetNextPktToTx();
	if (pPkt == NULL)
		return false;
	TxBytes(pPkt->pktData, pPkt->iSize);
	return true;
}
*/

bool CController::TxBytes(const BYTE* pData, int nBytes)
{
	if (nBytes == 0)		// WriteFile() was ending thread if nBytes == 0 !!
		return true;
	ASSERT(nBytes != 0);
	ASSERT(!m_bTxPending);
	DWORD nErrors;
	COMSTAT comStat;
	VERIFY(ClearCommError(m_hComm, &nErrors, &comStat));
	if (nErrors)
		LOGERROR(GetCommErrorDesc(nErrors));

	DWORD nBytesWritten;
	if (WriteFile(m_hComm, pData, nBytes, &nBytesWritten, &m_olWrite))	// event set when this data is transmitted
	{
		ASSERT((int)nBytesWritten == nBytes);
		m_bTxPending = false;
	}
	else
	{
		ASSERT(nBytesWritten == 0);
		ASSERT(GetLastError() == ERROR_IO_PENDING);
		m_bTxPending = true;
	}
	VERIFY(ClearCommError(m_hComm, &nErrors, &comStat));
	if (nErrors)
		LOGERROR(GetCommErrorDesc(nErrors));

	return true;
}

void CController::SendBytes(const BYTE* pData, int nBytes)	// from direct user request
{
	TxBytes(pData, nBytes);
}

bool CController::GetReceivedData()
{
/*
		Got full packet? N -> go wait for event
			Interperate packet and either:	
				queue more path packets if ready
				notify user of set/request result
				notify user of machine state
*/
// function is called after received data event (except first time)

	// data has been stored to m_arRxData[m_iRxDataSize] by overlapped read
	// except very first execution of this function which sets up overlap
	// check if received a full packet and if so, interperate
	// m_iRxDataSize now has size received - last byte in m_arRxData[m_iRxDataSize-1]

	bool bLooped = false;
	for (;;)
	{
		// windows XP behaves differently to window 98
		DWORD nBytesRead;
		BOOL bRes = GetOverlappedResult(m_hComm, &m_olRead, &nBytesRead, false);
		DWORD nLastError = GetLastError();
		if (bRes)	// win XP always true
		{
			//ASSERT(nLastError == ERROR_SUCCESS);
			if (nBytesRead == m_iPktBytesToRead)
				;					// win 98 goes here always if bRes == true
			else
				ASSERT(nBytesRead < m_iPktBytesToRead);
		}
		else		// windows 98 goes here
		{
			ASSERT(nLastError == ERROR_IO_INCOMPLETE);
			//ASSERT(nLastError == ERROR_IO_PENDING || nLastError == ERROR_OPERATION_ABORTED);
			ASSERT(nBytesRead < m_iPktBytesToRead);
			ASSERT(nBytesRead == 0);
			nBytesRead = m_iPktBytesToRead;
		}

		if (nBytesRead > 0)
		{
			m_iRxDataSize += nBytesRead;
			if (m_iExpRxPktSize <= 0)		// if m_iExpRxPktSize not set yet
			{
				if (m_iRxDataSize >= 1 && m_iExpRxPktSize != -2)		// got first byte - header so calc expected pkt size
				{
					m_iExpRxPktSize = CPacket::GetPktLenFromHeader(m_arRxData[0]);
					m_iPktBytesToRead = m_iExpRxPktSize - m_iRxDataSize;
					if ((m_arRxData[0] & P0SIZE_MASK) == P0SIZE_STREAM)
					{
						m_iExpRxPktSize = -2;		// get from 2nd byte
						m_iPktBytesToRead = 2 - m_iRxDataSize;
					}
				}
				if (m_iRxDataSize >= 2 && m_iExpRxPktSize == -2)		// check if it's a variable
				{
					m_iExpRxPktSize = CPacket::GetStreamPktLenFromByte2(m_arRxData[1]);		// length of variable packet
					m_iPktBytesToRead = m_iExpRxPktSize - m_iRxDataSize;
				}
			}
			else
				m_iPktBytesToRead = m_iExpRxPktSize - m_iRxDataSize;

			if (m_iExpRxPktSize > 0 && m_iRxDataSize >= m_iExpRxPktSize)
			{
				ASSERT(m_iRxDataSize == m_iExpRxPktSize);
				m_RxPkt.SetFromData(m_arRxData, m_iExpRxPktSize);
				int iDest = 0;
				for (int iSrc = m_iExpRxPktSize; iSrc < m_iRxDataSize; iSrc++)
					m_arRxData[iDest++] = m_arRxData[iSrc];		// move any remaining data to start
				m_iRxDataSize = iDest;
				m_iExpRxPktSize = -1;		// not set yet
				m_iPktBytesToRead = 1 - m_iRxDataSize;		// need to read just 1 byte of next packet

				if (m_bSimulateMachineResponse)
				{
					if (SimulateMachineResponse())
						while (GetSimulatedMachineResponse())
							HandleRxPacket();
				}
				if (!m_bSimulateMachineResponse)		// SimulateMachineResponse() can reset this back to false
					HandleRxPacket();
			}
		}

		DWORD nErrors;
		COMSTAT comStat;
		VERIFY(ClearCommError(m_hComm, &nErrors, &comStat));
		if (nErrors)
			LOGERROR(GetCommErrorDesc(nErrors));

		// read event was set to get here
		// ReadFile resets event if no data or sets event if data read
		bRes = ReadFile(m_hComm, m_arRxData + m_iRxDataSize, m_iPktBytesToRead, &nBytesRead, &m_olRead);
		nLastError = GetLastError();
		if (bRes)
		{
			if (nBytesRead == m_iPktBytesToRead)		// windows 98
				;	//ASSERT(nLastError == ERROR_IO_INCOMPLETE);
			else													// windows XP
			{
				//ASSERT(nLastError == ERROR_SUCCESS);
				ASSERT(nBytesRead < m_iPktBytesToRead);
				bRes = false;
			}
		}
		else
		{
			ASSERT(nLastError == ERROR_IO_PENDING);
			ASSERT(nBytesRead < m_iPktBytesToRead);
			ASSERT(nBytesRead == 0);
		}
		VERIFY(ClearCommError(m_hComm, &nErrors, &comStat));
		if (nErrors)
			LOGERROR(GetCommErrorDesc(nErrors));
		if (!bRes)
			break;
		bLooped = true;
	}	// for (;;)

	// will have emptied rx queue
	return true;
}

void CController::InitReadFile()
{
	// should do a ReadFile before GetOverlappedResult !!
	m_iRxDataSize = 0;
	m_iPktBytesToRead = 1;
	m_iExpRxPktSize = -1;			// not set yet

	DWORD nBytesRead;
	BOOL bRes = ReadFile(m_hComm, m_arRxData + m_iRxDataSize, m_iPktBytesToRead, &nBytesRead, &m_olRead);
	DWORD nLastError = GetLastError();
	if (bRes)
	{
		ASSERT(nLastError == ERROR_SUCCESS);
		if (nBytesRead == m_iPktBytesToRead)
			;		// windows 98
		else
		{
			ASSERT(nBytesRead < m_iPktBytesToRead);	// windows XP
			bRes = false;
		}
	}
	else
	{
		ASSERT(nLastError == ERROR_IO_PENDING);
		ASSERT(nBytesRead < (DWORD)m_iPktBytesToRead);
		ASSERT(nBytesRead == 0);
	}

	DWORD nErrors;
	COMSTAT comStat;
	VERIFY(ClearCommError(m_hComm, &nErrors, &comStat));
	if (nErrors)
		LOGERROR(GetCommErrorDesc(nErrors));
	if (bRes)
		GetReceivedData();		// read more if some read already
}

bool CController::SimulateMachineResponse()
{
	// rx input is connected to tx output so they must match!
	// take appropriate action on received packet to simulate machine response
	ASSERT(m_RxPkt.CheckCS());
	CPacket* pTxPkt = m_PktControl.GetNextTxdPkt();

	// check if it looks like a reflected packet
	if ((pTxPkt != NULL) && (m_RxPkt == *pTxPkt) && (m_RxPkt.IsInitiated()))
	{
		m_MachineSim.SendPacket(m_RxPkt);
		return true;
	}
	// packet not what was expected for reflection, check if simulate is intended
	int res = AfxMessageBox("Received Packet was not reflected although\nSimulate Machine Response is selected.\nContinue Simulating?", MB_YESNO | MB_DEFBUTTON2);
	m_bSimulateMachineResponse = (res == IDYES);
	if (m_bSimulateMachineResponse)
		m_MachineSim.SendPacket(m_RxPkt);

	NotifyUserThread(CN_SETSIMULATESTATE, m_bSimulateMachineResponse);	// notify a change
	return m_bSimulateMachineResponse;
}

bool CController::GetSimulatedMachineResponse()
{
	return m_MachineSim.GetReturnedPacket(m_RxPkt);
}

bool CController::HandleRxPacket()
{
/*	if packet is a reply:
		check if reply first packet on transmitted queue
		if not check next packet on transmitted queue

		if origional found in queue, mark as confirmed - remove if tail of queue
		send message to user thread of confirmed (sent or received) value
*/

	m_iRxPktCount++;
	m_MachTrack.CountRxPacket();
	CPacket& rxPkt = m_RxPkt;
	if (m_bLogCommsData)
	{
		int iSize = rxPkt.GetSize();
		if (m_pReceivedData->Add(rxPkt.GetData(), iSize) >= 0)
			NotifyUserThread(CN_UPDATE_RXLOG, iSize);
		else
			LOGERROR("Rx data log full");
	}
	if (!m_PktControl.CheckReceived(rxPkt))		// sets GetTxPktForReply()
		return false;

	// take appropriate action on received packet
	if (rxPkt.IsReply())
	{
		switch (rxPkt.GetSizeType())
		{
		case P0SIZE_BYTE:
			if (rxPkt.IsSend())
			{
				if (m_PktControl.GetTxPktForReply() == NULL)		// ignore for now, remove this later
					return true;
				m_MachTrack.ConfirmSentValueFrom(*m_PktControl.GetTxPktForReply());
			}
			break;
		case P0SIZE_DATA:
			m_MachTrack.HandleDataPacket(rxPkt);
			break;
		case P0SIZE_PATH:
			m_MachTrack.HandlePathPacket(rxPkt);
			break;
		case P0SIZE_STREAM:
			m_MachTrack.HandleStreamPacket(rxPkt);
			break;
		}		
	}
	else	// was initiated
	{
		switch (rxPkt.GetSizeType())
		{
		case P0SIZE_BYTE:
			break;
		case P0SIZE_DATA:
			m_MachTrack.HandleDataPacket(rxPkt);
			break;
		case P0SIZE_PATH:
			m_MachTrack.HandlePathPacket(rxPkt);
			break;
		case P0SIZE_STREAM:
			m_MachTrack.HandleStreamPacket(rxPkt);
			break;
		}
	}
	return true;
}







//////////////////////////////////////////////////

bool CController::HandleMessage()	// returns true if there was a message
{
	MSG msg;
	if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		return false;

	CPacket pkt;
	pkt.iSize = 0;		// is in constructor anyway!
	int wp = msg.wParam;
	long lp = msg.lParam;
	WORD lpL = LOWORD(lp);
	WORD lpH = HIWORD(lp);
	switch (msg.message)		// most common first
	{
	case WMU_SENDPATH:
		m_MachTrack.PathSend();
		break;
	case WMU_STOPPATH:
		m_MachTrack.PathStop();
		break;
	case WMU_SENDMOREPATH:		// send more cause data is available
		break;
	case WMU_RESETPATH:
		m_MachTrack.PathReset();
		break;

	case WMU_CHANGEVELOCITY: 
		m_MachTrack.ChangeVelocity((SAxisDataXcng*)wp);
		break;
	case WMU_SETVELOCITY: 
		m_MachTrack.SetVelocity((SAxisDataXcng*)wp);
		break;
	case WMU_STOPMOVEMENT: 
		m_MachTrack.StopMovement();
		break;

	case WMU_SETPARAMETERS:
		m_MachTrack.SetStandardParameters();
		break;


	case WMU_REQUESTNEXTPATHDATA:
		RequestNextPathData();
		break;
	case WMU_REQUESTALLPATHDATA:
		RequestAllPathData();
		break;
	case WMU_SETPATHBUFFER:
		SetPathBuffer((void*)lp);
		break;

	case WM_TIMER:
		ASSERT(0);		// no WM_TIMER !
		break;
	
	case WMU_REQUEST:
		DoUserRequest(wp, lp);
		break;
	case WMU_SENDPARAM:
		pkt.SendParam(wp, lp);		// (axpar, val)
		m_iStepPacketsCount = 1;
		break;
	case WMU_REQUESTPARAM:
		pkt.ReqParam(wp);						// (axpar)
		m_iStepPacketsCount = 1;
		break;
	case WMU_SENDALLPARAM:
		SendAllGeneralParam();
		SendAllAxisParam();
		break;
	case WMU_REQUESTALLPARAM:
		RequestAllGeneralParam();
		RequestAllAxisParam();
		break;

	case WMU_REQUESTMEM:
		if (CheckSetRAMLocation(wp))
			pkt.ReqMem(wp, lpL);				// (iAddr, iBytes)
		break;
	case WMU_SENDMEM:
		if (CheckSetRAMLocation(wp))
			pkt.SendMem(wp, lpL, lpH);		// (iAddr, iBytes, iValue)
		break;

	case WMU_SENDPKTCONT:
		SendPacketsContinuous();
		break;
	case WMU_SENDPKTSTEP:
		SendPacketsStep(wp);
		break;

	case WMU_REQUESTREG:
		pkt.ReqReg(wp, lpL);					// (iReg, iBytes)
		break;
	case WMU_SENDREG:
		pkt.SendReg(wp, lpL, lpH);			// (iReg, iBytes, iValue)
		break;
	case WMU_SENDBYTES:
		SendBytes((BYTE*)wp, lp);
		break;

	case WMU_INITCONTACT:
		m_MachTrack.InitContact();
		break;

	case WM_QUIT:
		m_bMonitorActive = false;
		break;
	default:
		TRACE1("*** Error: Unhandled message sent to monitor: %i\n", msg.message);
		return true;
	}

	if (pkt.iSize != 0)
		QueuePacket(pkt);

	return true;
}

void CController::DoUserRequest(int iRequest, long lData)
{
	switch (iRequest)
	{
	case REQ_SetSimulateResponse:
		m_bSimulateMachineResponse = (lData != 0);
		break;
	case REQ_SetLogCommsData:
		m_bLogCommsData = (lData != 0);
		break;
	case REQ_SetLogProbeSamples:
		m_MachTrack.m_ProbeSampler.LogSamples((char*)lData);
		break;
	case REQ_RecalibrateProbe:
		m_MachTrack.m_ProbeSampler.Recalibrate();
		break;
	case REQ_UpdateProbeSettings:
		m_MachTrack.m_ProbeSampler.LoadSettings();
		break;
	default:
		LOGERROR1("*** Error: Unhandled request sent to monitor: %i\n", iRequest);
	}
}

void CController::NotifyUserThread(int iNotification, int iData)
{
	m_pUserThread->PostThreadMessage(WMU_COMMNOTIFY, iNotification, iData);
}

bool CController::DoConfirmCheck()
{
	// If nothing was transmitted during last interval
	// and 'ConfirmQueue' is still full then empty 'ConfirmQueue' (or resend)
	// Ignores anything currently being rx as CNC may tx regularly

	if (!m_bTxPending)
	{
		if (GetTickCount() - m_iTimeTxLastPkt >= 500)		// been 500ms since start of last tx
			if (m_PktControl.DiscardUnconfirmed() > 0)
				TxQueuedData();
	}


	return true;
}

bool CController::DoTimeOutCheck()
{
	if (m_bTxPending)
	{
		DWORD nBytesWritten;
		if (GetOverlappedResult(m_hComm, &m_olWrite, &nBytesWritten, false))
		{
			ASSERT(0);		// end of tx should have triggered wait
			m_bTxPending = false;
		}
		else
			ASSERT(GetLastError() == ERROR_IO_INCOMPLETE);
	}
	if (m_MachTrack.ProcessingPath() && m_PktControl.PathDataLow())
		NotifyUserThread(CN_PATHBUFFER_LOW, 1);	// 1 means due to time out


	return true;
}






//----------------------------------------------
// Sending packet group functions

bool CController::RequestAllGeneralParam()
{
	CPacket pkt;
	int maxPar = m_pParamDoc->m_nMaxGeneralPar;
	for (int i = 0; i <= maxPar; i++)
		if (m_pParamDoc->m_GeneralPar[i].name)	// check valid param
		{
			pkt.ReqParam(CAxPar(0, i));
			QueuePacket(pkt);
		}
	return 0;
}

bool CController::RequestAllAxisParam()
{
	CPacket pkt;
	int maxPar = m_pParamDoc->m_nMaxAxisPar;
	int numAxis = m_pParamDoc->m_nNumAxis;
	for (int i = 0; i <= maxPar; i++)
		if (m_pParamDoc->m_AxisPar[i][0].name)	// check valid param
			for (int ax = 1; ax <= numAxis; ax++)		// request all axis of param
			{
				pkt.ReqParam(CAxPar(ax, i));
				QueuePacket(pkt);
			}
	return 0;
}

bool CController::SendAllGeneralParam()
{
	CPacket pkt;
	int maxPar = m_pParamDoc->m_nMaxGeneralPar;
	for (int i = 0; i <= maxPar; i++)
	{
		CParam& param = m_pParamDoc->m_GeneralPar[i];
		if (param.name)			// check valid param
			if (param.state & PSAB_MODIFIED || ~param.state & PSAB_UNKNOWN)	// can send if modified or known
			{
				pkt.SendParam(CAxPar(0, i), param.sendValue);
				QueuePacket(pkt);
			}
	}
	return 0;
}

bool CController::SendAllAxisParam()
{
	CPacket pkt;
	int maxPar = m_pParamDoc->m_nMaxAxisPar;
	int numAxis = m_pParamDoc->m_nNumAxis;
	for (int i = 0; i <= maxPar; i++)
		for (int ax = 1; ax <= numAxis; ax++)
		{
			CParam& param = m_pParamDoc->m_AxisPar[i][ax-1];
			if (!param.name)			// check valid param
				ax = numAxis;			// stop loop
			else
				if (param.state & PSAB_MODIFIED || ~param.state & PSAB_UNKNOWN)	// can send if modified or known
				{
					pkt.SendParam(CAxPar(ax, i), param.sendValue);
					QueuePacket(pkt);
				}
		}
	return 0;
}



void CController::SendPacketsContinuous()
{
	m_bStepPackets = false;
}

void CController::SendPacketsStep(int iNum)
{
	// sends one packet at a time
	m_bStepPackets = true;
	m_iStepPacketsCount = iNum;
}

bool CController::RequestNextPathData()
{
	static int iNextObjNum = 0;
	CPacket pkt;
	pkt.ReqPath(iNextObjNum, POB_NEXTTOUSE);		// (iObjNum, nBase)
	QueuePacket(pkt);
	return true;
}

bool CController::RequestAllPathData()
{
	ASSERT(0);
	return false;
}


///////////////////////////
// memory functions
///////////////////////////

bool CController::CheckSetRAMLocation(int iAddr)
{
	// Check RAM Location parameter and set if not equal to address
	int iRAMLCode = m_pParamDoc->FindParamCode("RAMLocationW0", 0);
	if (iRAMLCode == -1)
	{
		TRACE0("Error: Can't find parameter 'RAMLocationW0' to set RAM location");
		return false;
	}
	CAxPar axparL(0, iRAMLCode);
	CAxPar axparH(0, iRAMLCode + 1);
	__int64 i64Value;
	int res = m_pParamDoc->GetExParamValue(axparL, &i64Value);
	if (!res || i64Value != iAddr)
	{
		CPacket pkt;
		pkt.SendParam(axparL, LOWORD(iAddr));		// (axpar, val)
		QueuePacket(pkt);
		pkt.SendParam(axparH, HIWORD(iAddr));		// (axpar, val)
		QueuePacket(pkt);
	}
	return true;
}

////////////////////////////

