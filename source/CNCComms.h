// CNCComms.h: interface for the CCNCComms class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CNCCOMMS_H__D1605DD2_9521_11D4_8C1E_FAFCA34A0C2F__INCLUDED_)
#define AFX_CNCCOMMS_H__D1605DD2_9521_11D4_8C1E_FAFCA34A0C2F__INCLUDED_

#include "CommsDataDlg.h"	// Added by ClassView
#include "Controller.h"
#include "ThreadMessages.h"


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CCNCComms  
{
public:
	CCNCComms();
	virtual ~CCNCComms();
	void Init();
	bool UpdateTxLog(int iNumBytes);
	bool UpdateRxLog(int iNumBytes);

	bool OpenComms();
	bool CloseComms();
	bool IsCommOpen();
	HANDLE GetCommHandle();
	bool ConfigPort();
	bool SelectPort();
	bool SetCommPort(int iPort);
	int GetCommPort() { return m_iPort; }
	char* GetCommName() { return m_szCommName; }

	void LogCommsData(bool bLog);
	bool IsLoggingCommsData();
	void ShowDataLog(bool bShow);
	bool IsDataLogVisible();

	bool SendMonitorRequest(int iRequest, long lData = 0) { return SendMonitorMessage(WMU_REQUEST, iRequest, lData); }
	bool SendMonitorMessage(int iMsg, int wParam = 0, long lParam = 0);


protected:
	// data
	int m_iPort;
	HANDLE m_hComm;

	bool ActivateMonitorThread();
	bool TerminateMonitorThread();

	bool m_bLogCommsData;
	bool m_bShowDataLog;

	// functions
	void SetDefaultCommDCB();
	DCB m_CommDCB;
	char m_szCommName[6];
//	CSerial m_SerialDirect;
	CCommsDataDlg* m_pCommsLog;
	CByteFIFO* m_pTransmittedData;
	CByteFIFO* m_pReceivedData;
	CWinThread* m_pMonitorThread;
	

};

#endif // !defined(AFX_CNCCOMMS_H__D1605DD2_9521_11D4_8C1E_FAFCA34A0C2F__INCLUDED_)
