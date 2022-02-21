// CNCComms.cpp: implementation of the CCNCComms class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNCControl.h"

//#include <conio.h>


#include "CNCControlApp.h"
#include "PortSelDlg.h"
#include "CNCComms.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif






//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCNCComms::CCNCComms()
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_iPort = -1;
	m_pCommsLog = NULL;

	m_pMonitorThread = NULL;
	m_pTransmittedData = NULL;
	m_pReceivedData = NULL;

	m_bLogCommsData = false;


}


CCNCComms::~CCNCComms()
{
	CloseComms();
	delete m_pCommsLog;
}

void CCNCComms::Init()
{
	SetCommPort(g_Settings.MachComms.iCommPort);
	g_Settings.MachComms.iCommPort = m_iPort;

	m_bLogCommsData = false;
	m_bShowDataLog = false;

	COMMCONFIG commConfig;
	DWORD sizeCC = sizeof(commConfig);
	VERIFY(GetDefaultCommConfig(m_szCommName, &commConfig, &sizeCC));
	m_CommDCB = commConfig.dcb;	// Set DCB to driver default

	SetDefaultCommDCB();		// Set default DCB for machine (or set from INI file)
	commConfig.dcb = m_CommDCB;
	VERIFY(SetDefaultCommConfig(m_szCommName, &commConfig, sizeCC));	// Doesn't set 'Restore Default' defaults

}

//////////////////////////////////////////////////////////////////////
// Open / Close Comms

void CCNCComms::LogCommsData(bool bLog)
{
	m_bLogCommsData = bLog;
	if (IsCommOpen())
		SendMonitorRequest(REQ_SetLogCommsData, bLog);
}

bool CCNCComms::IsLoggingCommsData()
{
	return m_bLogCommsData;
}

void CCNCComms::ShowDataLog(bool bShow)
{
	if (m_pCommsLog)
		m_pCommsLog->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	else if (bShow)
	{
		m_pCommsLog = new CCommsDataDlg;
		m_pCommsLog->ShowWindow(SW_SHOW);
	}
	m_bShowDataLog = bShow;
	if (bShow)
		AfxGetMainWnd()->SetFocus();
}

bool CCNCComms::IsDataLogVisible()
{
	if (m_pCommsLog)
		return m_pCommsLog->IsWindowVisible() != 0;
	return false;
}

bool CCNCComms::OpenComms()
{
	if (m_hComm != INVALID_HANDLE_VALUE)	// Already open
		return false;
	m_hComm = CreateFile(
		m_szCommName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,		// or 0 if not overlapped
		NULL );
	if (m_hComm == INVALID_HANDLE_VALUE)
		return false;



	DWORD mask = 0;
	DWORD dwInQueue = 1000;
	DWORD dwOutQueue = 1000;
//	COMMPROP cp;
	COMMTIMEOUTS cto1, cto2;
	DWORD nErrors = (DWORD)-1;
	COMSTAT cs;

//	a = GetCommState(m_hComm, &dcb);
//	b = GetCommMask(m_hComm, &mask);
//	c = GetCommProperties(m_hComm, &cp);
	VERIFY(GetCommTimeouts(m_hComm, &cto1));

	VERIFY(SetCommState(m_hComm, &m_CommDCB));
	VERIFY(SetCommMask(m_hComm, mask));
	VERIFY(SetupComm(m_hComm, dwInQueue, dwOutQueue));		// set queues
//	VERIFY(SetCommTimeouts(m_hComm, &cto));

	VERIFY(ClearCommError(m_hComm, &nErrors, &cs));
	ASSERT(nErrors == 0);

	VERIFY(GetCommTimeouts(m_hComm, &cto2));

/*
	// see if can control loop back test
	ASSERT(m_iPort >= 1 && m_iPort <=4);
	short commPortAddrs[4] = { 0x03f8, 0x02f8, 0x03e8, 0x02e8 };
	short iPortAddr = commPortAddrs[m_iPort - 1];

	#define MCR 4
	short iPortMCR = (short)(iPortAddr + MCR);
	int val = _inp(iPortMCR);
//	_outp(iPortMCR, val | 0x10);			// MCR 3fc, set 0x10 (bit 4) for loop back test
	int val2 = _inp(iPortMCR);
*/


	if (!ActivateMonitorThread())
		return false;

// Activate data log window
	LogCommsData(m_bLogCommsData);
	ShowDataLog(m_bShowDataLog);

// Send initial contact sequence
	SendMonitorMessage(WMU_INITCONTACT);

	return true;
}

bool CCNCComms::ActivateMonitorThread()
{
// Create a thread to monitor serial port and send/receive data
	// collect info for new thread and start it
	CCtrlThreadInfo* pInfo = new CCtrlThreadInfo;	// deleted by new thread
	pInfo->m_hComm = GetCommHandle();
	pInfo->m_pUserThread = AfxGetThread();
	ASSERT(AfxGetApp()->IsKindOf(RUNTIME_CLASS(CCNCControlApp)));
	pInfo->m_pParamDoc = ((CCNCControlApp*)AfxGetApp())->m_pParamDoc;

	m_pTransmittedData = new CByteFIFO;
	m_pReceivedData = new CByteFIFO;
	pInfo->m_pTransmittedData = m_pTransmittedData;
	pInfo->m_pReceivedData = m_pReceivedData;

	m_pMonitorThread = AfxBeginThread(MonitorComms, pInfo);
	ASSERT(m_pMonitorThread != NULL);
	VERIFY(m_pMonitorThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL));
	return true;
}

bool CCNCComms::TerminateMonitorThread()
{
	if (!m_pMonitorThread)
		return false;
	HANDLE hMonitorThread = m_pMonitorThread->m_hThread;		// thread object was once deleted immediatly after WM_QUIT!
	VERIFY(m_pMonitorThread->PostThreadMessage(WM_QUIT, 0, 0));
	DWORD iWaitRes = WaitForSingleObject(hMonitorThread, INFINITE);	// wait for monitor thread to end
	DWORD iLastError = 0;
	if (iWaitRes == WAIT_FAILED)			// WAIT_FAILED = -1
		iLastError = GetLastError();		// 6 = ERROR_INVALID_HANDLE
	ASSERT(iWaitRes == WAIT_OBJECT_0 || iLastError == ERROR_INVALID_HANDLE);
	m_pMonitorThread = NULL;
	delete m_pTransmittedData;
	delete m_pReceivedData;
	m_pTransmittedData = NULL;
	m_pReceivedData = NULL;
	return true;
}


bool CCNCComms::SendMonitorMessage(int iMsg, int wParam, long lParam)
{
	if (!m_pMonitorThread)
	{
		LOGERROR("Comms monitor thread not opened");
		return false;
	}
	if (m_pMonitorThread->PostThreadMessage(iMsg, wParam, lParam))
		return true;
	TRACE0("Error posting message to monitor thread\n");
	return false;
}

bool CCNCComms::CloseComms()
{
	TerminateMonitorThread();

	// Close Data Log window
	if (m_pCommsLog)
	{
		m_pCommsLog->DestroyWindow();
		delete m_pCommsLog;
		m_pCommsLog = NULL;
	}
	
	BOOL res = CloseHandle(m_hComm);
	m_hComm = INVALID_HANDLE_VALUE;
	return res != 0;
}

bool CCNCComms::IsCommOpen()
{
	return (m_hComm != INVALID_HANDLE_VALUE);
}

//////////////////////////////////////////////////////////////////////
// Configuration

bool CCNCComms::SelectPort()
{
	CPortSelDlg selDlg;
	selDlg.m_iPort = m_iPort - 1;
	if (selDlg.DoModal() == IDOK)
	{
		int iNewPort = selDlg.m_iPort + 1;
		if (m_hComm == INVALID_HANDLE_VALUE || iNewPort == m_iPort)
			SetCommPort(iNewPort);
		else
			AfxMessageBox("Close communications before changing port");
	}
	return true;
}

bool CCNCComms::ConfigPort()
{
	COMMCONFIG commConfig;
	DWORD sizeCC = sizeof(commConfig);

	if (m_hComm == INVALID_HANDLE_VALUE)
		commConfig.dcb = m_CommDCB;
	else
		VERIFY(GetCommConfig(m_hComm, &commConfig, &sizeCC));
	if (CommConfigDialog(m_szCommName, *AfxGetMainWnd(), &commConfig))
	{
		m_CommDCB = commConfig.dcb;
		SetCommConfig(m_hComm, &commConfig, sizeCC);
		return true;
	}
	return false;
}




//////////////////////////////////////////////////////////////////////
// Implementation






HANDLE CCNCComms::GetCommHandle()
{
	return m_hComm;
}

bool CCNCComms::UpdateTxLog(int iNumBytes)
{
	if (!m_pCommsLog)
		return false;
	BYTE* pData;
	bool bMore;
	int len;
	do
	{
		bMore = m_pTransmittedData->GetNextBlock(pData, len);
		if (len >= iNumBytes)
		{
			len = iNumBytes;
			bMore = false;			// ignore anymore for now
		}
//		else	// (len < iNumBytes)
//			ASSERT(bMore);
		if (len == 0)
			break;
		iNumBytes -= len;		// iNumBytes is now bytes left
		m_pCommsLog->AddToTx(pData, len, !bMore);	// NewLine if no more
		m_pTransmittedData->Discard(len);
	} while (bMore);
	return true;
}

bool CCNCComms::UpdateRxLog(int iNumBytes)
{
	if (!m_pCommsLog)
		return false;
	BYTE* pData;
	bool bMore;
	int len;
	do
	{
		bMore = m_pReceivedData->GetNextBlock(pData, len);
		if (len >= iNumBytes)
		{
			len = iNumBytes;
			bMore = false;			// ignore anymore for now
		}
//		else	// (len < iNumBytes)
//			ASSERT(bMore);
		if (len == 0)
			break;
		iNumBytes -= len;		// iNumBytes is nowbytes left
		m_pCommsLog->AddToRx(pData, len, !bMore);	// NewLine if no more
		m_pReceivedData->Discard(len);
	} while (bMore);
	return true;
}

bool CCNCComms::SetCommPort(int iPort)
{
	if (iPort < 1 || iPort > 4)
		iPort = 1;
	m_iPort = iPort;
	strcpy(m_szCommName, "COM#");
	m_szCommName[3] = (char)(m_iPort + '0');
	return true;
}

void CCNCComms::SetDefaultCommDCB()
{
		// Set default DCB for machine
	m_CommDCB.BaudRate = CBR_19200;
	m_CommDCB.ByteSize = 8;
	m_CommDCB.Parity = NOPARITY;
	m_CommDCB.StopBits = ONESTOPBIT;		// 1 stop bit

	m_CommDCB.fOutX = 0;
	m_CommDCB.fInX = 0;
	m_CommDCB.fOutxCtsFlow = 0;
	m_CommDCB.fRtsControl = 0;


	m_CommDCB.fBinary = true;
	m_CommDCB.fParity = true;
	m_CommDCB.fAbortOnError = true;

}
