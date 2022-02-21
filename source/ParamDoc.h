#if !defined(AFX_PARAMDOC_H__6864A0C2_7D83_11D4_8C1E_CFCA79DD0A2F__INCLUDED_)
#define AFX_PARAMDOC_H__6864A0C2_7D83_11D4_8C1E_CFCA79DD0A2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// ParamDoc.h : header file
//

#include "Param.h"
#include "MachineCodes.h"
#include "CNCComms.h"
#include "MachineState.h"

struct SContBufferSpan;

enum		// Parameter status bits
{
	PSAB_UNKNOWN = 0x01,			// value unknown
	PSAB_READY = 0x02,			// ready for operation

	PSAB_SEND = 0x04,				// sending		\_ one of
	PSAB_REQUEST = 0x08,			// requesting	/
		
	PSAB_MODIFIED = 0x10,		// 'sendValue' is has been modified, reset after value is sent

	PSAB_MONITOR = 0x0100,		// being handled be monitor thread
	PSAB_QUEUED = 0x0200,		// queued
	PSAB_TRANSMITTED = 0x0400,	// transmitted, waiting reply
	PSAB_CONFIRMED = 0x0800,	// confirmation received
	PSAB_OK = 0x1000,				// conf ok		\_ one of
	PSAB_ERROR = 0x2000,			// conf error	/
	PSAB_MONITORDONE = 0x4000,	// monitor thread finished

	PSAB_SENTDIFF = 0x10000,	// value different to sent value
};

enum
{
/*	PSA_UNKNOWN = 1,	// unknown value
	PSA_READY,			// not currently sending or requesting

	PSA_NVSET,			// new value set in 'sendValue', waiting to be queued for tx
	PSA_NVQUEWAITCONF,// new value queued for transmission, waiting confirmation
	PSA_NVTXWAITCONF,	// new value queued for transmission, waiting confirmation
	PSA_NVSENTOK,		// new value transmitted and confirmed OK
	PSA_NVCONFERROR,	// new value, confirmation error

	PSA_REQWAITREPLY,	// value requested, waiting for reply
	PSA_REQRECVOK,		// value requested and received OK
	PSA_REQRECVERROR,	// value requested and received error
*/
};

enum		// Parameter style bits
{
	PSY_FORMATMASK = 0x0003,
	PSY_NOFORMAT = 0x0000,
	PSY_DEC = 0x0001,
	PSY_HEX = 0x0002,
	PSY_BIN = 0x0003,

	PSY_UNSIGNED = 0x0004,
	PSY_READONLY = 0x0008,
	PSY_HASMAX = 0x0010,
	PSY_HASMIN = 0x0020,
	PSY_WORDNUMMASK = 0x00c0,
	PSY_WORDNUMSHIFT = 6,
	PSY_TOTALBYTESMASK = 0x0700,
	PSY_TOTALBYTESSHIFT = 8,

	PSY_CODEMASK = 0x0fff,	// used for list item data
	PSY_GENERAL = 0x1000,	// used for list item data
	PSY_AXIS = 0x2000,		// used for list item data
	PSY_UNIQUEAXIS = 0x4000 | PSY_AXIS,	// DO NOT USE for list item data
};


/////////////////////////////////////////////////////////////////////////////
// class CParamEx
/////////////////////////////////////////////////////////////////////////////
// inherited from CParam with extra data for loading from file

class CParamEx : public CParam		// extra parameter info for loading from file
{
public:
	int iFileSection;
	int iFileLine;

	int iTotalBytes;
	int iWordNum;


};


/////////////////////////////////////////////////////////////////////////////
// class CParamDoc
/////////////////////////////////////////////////////////////////////////////

#define HINT_MASK			0xffff0000
#define HINT_AXPAR		0x00010000
#define HINT_MACHSTATE	0x00020000
#define HINT_MACHBUFF	0x00030000



class CParamDoc : public CDocument
{
public:
	CParamDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CParamDoc)

// Attributes
protected:
	CCNCComms* m_pCNCComms;
	char* m_arErrors[Error_Max+1];

public:
	int m_nNumAxis;
	CParam m_GeneralPar[100];
	CParam m_AxisPar[200][NUM_AXIS];
	int m_nMaxGeneralPar;
	int m_nMaxAxisPar;

	CMachineState* m_pMachineState;
	SContBufferSpan* m_pMachineBuffer;


// Operations
public:

	void UpdateMachineStateStatus(CMachineState* pMachineState);
	void UpdateMachineBufferStatus(SContBufferSpan* pMachineBuffer);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParamDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual void OnCloseDocument();
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CParamDoc();
	void SetCNCComm(CCNCComms* pCNCComms);
	void SetAllParamsState(int iState);
	void SetAllParamsValue(int iValue);
	CParam* GetParam(int axis, int code);
	CParam* GetParam(CAxPar axpar) { return GetParam(axpar.Axis(), axpar.Param()); }
	int FindParamCode(const char* szParamName, int axis);
	bool SetAxisParam(int iAxis, int iParam, int iValue) { return SetParam(CAxPar(iAxis, iParam), iValue); }
	bool SetParam(CAxPar axpar, int val);
	bool RequestParam(CAxPar axpar);
	bool SendParam(CAxPar axpar);

	bool SetRxParamValue(CAxPar axpar, int iVal);	// used by monitor thread
	bool SetRxConfirmParamValue(CAxPar axpar);		// used by monitor thread
	bool GetParamValue(CAxPar axpar, int* iVal);		// used by monitor thread
	bool SetExParamValue(CAxPar axpar, __int64 lVal);
	bool GetExParamValue(CAxPar axpar, __int64* lVal);
	char* GetErrorDescription(int nCode);

	bool SendMonitorMessage(int iMsg, int wParam = 0, long lParam = 0) { return m_pCNCComms->SendMonitorMessage(iMsg, wParam, lParam); }
	bool SendMonitorRequest(int iRequest, long lData = 0) { return m_pCNCComms->SendMonitorMessage(WMU_REQUEST, iRequest, lData); }



#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	//{{AFX_MSG(CParamDoc)
	afx_msg void OnSendAllParam();
	afx_msg void OnRequestAllParam();
	afx_msg void OnPathSendNextData();
	afx_msg void OnPathSendAllData();
	afx_msg void OnPathRequestNextData();
	afx_msg void OnPathRequestAllData();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARAMDOC_H__6864A0C2_7D83_11D4_8C1E_CFCA79DD0A2F__INCLUDED_)
