// ParamDoc.cpp : implementation file
//

#include "stdafx.h"

#include <fstream.h>

#include "CNCControl.h"
//#include "StrUtils.h"
#include "ParamDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CParamDoc

IMPLEMENT_DYNCREATE(CParamDoc, CDocument)

CParamDoc::CParamDoc()
{
	m_nNumAxis = NUM_AXIS;
	m_pCNCComms = NULL;
	m_pMachineState = NULL;
	m_pMachineBuffer = NULL;

	SetTitle("Machine Parameters");

	// Set error text strings - error code set in "MachineCodes.h"
	for (int idxError = 0; idxError < Error_Max; idxError++)
		m_arErrors[idxError] = "Text not set";

	m_arErrors[0] = "";

	m_arErrors[Error_PosOverflow]						= "Error_PosOverflow";

	m_arErrors[Error_PCRxBuffFull]					= "Error_PCRxBuffFull";
	m_arErrors[Error_RxBuffTooMany]					= "Error_RxBuffTooMany";
	m_arErrors[Error_RxPacketChecksum]				= "Error_RxPacketChecksum";
	m_arErrors[Error_RxPacketID]						= "Error_RxPacketID";
	m_arErrors[Error_TxBuffOvf]						= "Error_TxBuffOvf";
	m_arErrors[Error_TxBuffWontFit]					= "Error_TxBuffWontFit";

	m_arErrors[Error_PathBuffSizeSetNotEmpty]		= "Error_PathBuffSizeSetNotEmpty";
	m_arErrors[Error_PathBuffRequGtNum]				= "Error_PathBuffRequGtNum";
	m_arErrors[Error_PathTooManyObjsZeroTime]		= "Error_PathTooManyObjsZeroTime";
	m_arErrors[Error_PathGetObjTimeLow]				= "Error_PathGetObjTimeLow";
	m_arErrors[Error_PathBufferFull]					= "Error_PathBufferFull";
	m_arErrors[Error_PathObjCountMismatch]			= "Error_PathObjCountMismatch";
	m_arErrors[Error_PathObjChecksumFromBuff]		= "Error_PathObjChecksumFromBuff";

	m_arErrors[Error_SampleBuffEmpty]				= "Error_SampleBuffEmpty";

	m_arErrors[Error_InterruptSendSampleData]		= "Error_InterruptSendSampleData";
	m_arErrors[Error_InterruptSetGetRAM]			= "Error_InterruptSetGetRAM";
	m_arErrors[Error_InterruptSetPathData]			= "Error_InterruptSetPathData";
	m_arErrors[Error_InterruptGetPathData]			= "Error_InterruptGetPathData";
	m_arErrors[Error_InterruptSendMotionUpdate]	= "Error_InterruptSendMotionUpdate";

	m_arErrors[Error_ParamNumBad]						= "Error_ParamNumBad";
	m_arErrors[Error_ParamValueBad]					= "Error_ParamValueBad";
	m_arErrors[Error_ParamSetReadOnly]				= "Error_ParamSetReadOnly";
	m_arErrors[Error_ParamLocked]						= "Error_ParamLocked";
	m_arErrors[Error_NotEnoughMemory]				= "Error_NotEnoughMemory";

	m_arErrors[Error_HandTxBuffOvf]					= "Error_HandTxBuffOvf";
	m_arErrors[Error_HandTxBuffWontFit]				= "Error_HandTxBuffWontFit";
	m_arErrors[Error_HandRxBuffFull]					= "Error_HandRxBuffFull";
	m_arErrors[Error_HandRxBuffTooMany]				= "Error_HandRxBuffTooMany";
	m_arErrors[Error_HandRxBuffChecksum]			= "Error_HandRxBuffChecksum";

	m_arErrors[Error_ServoSimCurrentLimit]			= "Error_ServoSimCurrentLimit";
	m_arErrors[Error_ServoAxisStuck]					= "Error_ServoAxisStuck";
	m_arErrors[Error_AxisPosErrorLimit]				= "Error_AxisPosErrorLimit";
	m_arErrors[Error_AxisPastPosLimit]				= "Error_AxisPastPosLimit";
	m_arErrors[Error_ProbeRequestTimedOut]			= "Error_ProbeRequestTimedOut";
	
	m_arErrors[Error_LocalMachineShutdown]			= "Error_LocalMachineShutdown";
	m_arErrors[Error_RemoteMachineShutdown]		= "Error_RemoteMachineShutdown";
	m_arErrors[Error_LocatorBlockedDistance]		= "Error_LocatorBlockedDistance";
	m_arErrors[Error_ServoDriveCurrentLimit]		= "Error_ServoDriveCurrentLimit";

	m_arErrors[Error_InterruptI2CCommand]			= "Error_InterruptI2CCommand";

	m_arErrors[Error_Max]								= "Invalid Error Code";



}

BOOL CParamDoc::OnNewDocument()		// not called anyway
{
	if (!CDocument::OnNewDocument())
		return FALSE;
//	SetModifiedFlag(false);			// to disable ID_FILE_SAVE
	return TRUE;
}

CParamDoc::~CParamDoc()
{
	int i;
	for (i = 0; i <= m_nMaxGeneralPar; i++)
		delete m_GeneralPar[i].name;
	for (i = 0; i <= m_nMaxAxisPar; i++)
		delete m_AxisPar[i][0].name;
}


BEGIN_MESSAGE_MAP(CParamDoc, CDocument)
	//{{AFX_MSG_MAP(CParamDoc)
	ON_COMMAND(ID_MACHINE_SENDALLPARAMETERS, OnSendAllParam)
	ON_COMMAND(ID_MACHINE_REQUESTALLPARAMETERS, OnRequestAllParam)
//	ON_COMMAND(ID_PATH_SENDNEXTPATHDATA, OnPathSendNextData)
//	ON_COMMAND(ID_PATH_SENDALLPATHDATA, OnPathSendAllData)
	ON_COMMAND(ID_PATH_REQUESTNEXTPATHDATA, OnPathRequestNextData)
	ON_COMMAND(ID_PATH_REQUESTALLPATHDATA, OnPathRequestAllData)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParamDoc diagnostics

#ifdef _DEBUG
void CParamDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CParamDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CParamDoc serialization

void CParamDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing Code here
	}
	else
	{
		// TODO: add loading Code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CParamDoc commands


char* CParamDoc::GetErrorDescription(int nCode)
{
	if (nCode < 0 || nCode > Error_Max)
		nCode = Error_Max;
	return m_arErrors[nCode];
}



void CParamDoc::SetAllParamsState(int iState)
{
	for (int i = 0; i <= m_nMaxGeneralPar; i++)
		m_GeneralPar[i].state = iState;
	for (i = 0; i <= m_nMaxAxisPar; i++)
		for (int ax = 0; ax < m_nNumAxis; ax++)
			m_AxisPar[i][ax].state = iState;
}

void CParamDoc::SetAllParamsValue(int iValue)
{
	for (int i = 0; i <= m_nMaxGeneralPar; i++)
		m_GeneralPar[i].value = iValue;
	for (i = 0; i <= m_nMaxAxisPar; i++)
		for (int ax = 0; ax < m_nNumAxis; ax++)
			m_AxisPar[i][ax].value = iValue;
}


CParam* CParamDoc::GetParam(int axis, int code)
{
	if (axis == 0)
	{
		if (code <= m_nMaxGeneralPar)
			return &m_GeneralPar[code];
		LOGERROR1("Invalid General Param %i", code);
	}
	else if (axis >= 1 && axis <= m_nNumAxis)
	{
		if (code <= m_nMaxAxisPar)
			return &m_AxisPar[code][axis - 1];
		LOGERROR1("Invalid Axis Param %i", code);
	}
	else
		LOGERROR1("Invalid Axis %i", axis);
	return NULL;
}

int CParamDoc::FindParamCode(const char* szParamName, int axis)
{
	int i, code = -1;
	if (axis == 0)
	{
		for (i = 0; i <= m_nMaxGeneralPar; i++)
			if (m_GeneralPar[i].name && strcmp(m_GeneralPar[i].name, szParamName) == 0)
			{
				code = m_GeneralPar[i].code;
				break;
			}
	}
	else if (axis >= 1 && axis <= m_nNumAxis)
	{
		for (i = 0; i <= m_nMaxAxisPar; i++)
			if (m_AxisPar[i][axis].name && strcmp(m_AxisPar[i][axis].name, szParamName) == 0)
			{
				code = m_AxisPar[i][axis].code;
				break;
			}
	}
	if (code != -1 && code != i)
		TRACE2("NOTE: In CParamDoc::FindParamCode() index is %i and code is %i\n", i, code);
	return code;
}

bool CParamDoc::SendParam(CAxPar axpar)
{
	CParam& sp = *GetParam(axpar);
	if (sp.state & PSAB_UNKNOWN && !(sp.state & PSAB_MODIFIED))
	{
		TRACE0("No valid value to send\n");
		int beep = MB_ICONHAND;
		BOOL res = ::MessageBeep(beep);
		int err = GetLastError();
		return false;
	}
/*	if (!(sp.state & PSAB_READY))
	{
		TRACE0("Parameter not in READY state for send\n");
		return false;
	}
*/
	sp.state &= ~PSAB_READY;
	sp.state |= PSAB_SEND | PSAB_QUEUED | PSAB_MONITOR;	// new value queued for transmission, waiting confirmation
	SendMonitorMessage(WMU_SENDPARAM, axpar, sp.sendValue);
	UpdateAllViews(NULL, HINT_AXPAR | axpar);
	return true;
}

bool CParamDoc::RequestParam(CAxPar axpar)
{
	CParam& sp = *GetParam(axpar);
/*	if (!(sp.state & PSAB_READY))
	{
		TRACE0("Parameter not in READY state for request\n");
		return false;
	}
*/
	sp.state &= ~PSAB_READY;
	sp.state |= PSAB_REQUEST | PSAB_QUEUED | PSAB_MONITOR;	// request is queued for transmission, waiting confirmation
	SendMonitorMessage(WMU_REQUESTPARAM, axpar, 0);
	UpdateAllViews(NULL, HINT_AXPAR | axpar);
	return true;
}

void CParamDoc::SetCNCComm(CCNCComms* pCNCComms)
{
	m_pCNCComms = pCNCComms;
}

bool CParamDoc::SetParam(CAxPar axpar, int val)
{
	CParam& sp = *GetParam(axpar);
/*	if (!(sp.state & PSAB_READY))
	{
		TRACE0("Parameter not in READY state for set\n");
		return false;
	}
*/
	sp.sendValue = val;
	sp.state |= PSAB_MODIFIED;		// new value set in 'sendValue', waiting to be queued for tx
	SendParam(axpar);
	return true;
}


bool CParamDoc::SetRxParamValue(CAxPar axpar, int val)	// accessed by monitor thread to set recieved value
{
	CParam* pParam;
	if (axpar.IsAllAxis())		// set all axis
		for (int ax = 1; ax <= m_nNumAxis; ax++)
		{
			pParam = GetParam(ax, axpar.Param());
			if (!pParam)
				return false;
			pParam->value = val;
			pParam->sendValue = val;
			pParam->state = PSAB_READY;
		}
	else
	{
		pParam = GetParam(axpar);
		if (!pParam)
			return false;
		if (pParam->style & PSY_UNSIGNED)
			val &= 0xffff;			// take low word
		pParam->value = val;
		pParam->sendValue = val;		// can compare with sent value
		pParam->state = PSAB_READY;
	}
	return true;
}

bool CParamDoc::SetRxConfirmParamValue(CAxPar axpar)	// accessed by monitor thread to set recieved value
{
	CParam* pParam;
	if (axpar.IsAllAxis())		// set all axis
		for (int ax = 1; ax <= m_nNumAxis; ax++)
		{
			pParam = GetParam(ax, axpar.Param());
			if (!pParam)
				return false;
			pParam->value = pParam->sendValue;
			pParam->state = PSAB_READY;
		}
	else
	{
		pParam = GetParam(axpar);
		if (!pParam)
			return false;
		pParam->value = pParam->sendValue;		// can compare with sent value
		pParam->state = PSAB_READY;
	}
	return true;
}


bool CParamDoc::GetParamValue(CAxPar axpar, int* piVal)
{
	CParam* pParam = GetParam(axpar);
	if (pParam == NULL)
		return false;
	*piVal = pParam->value;
	return true;
}

bool CParamDoc::SetExParamValue(CAxPar axpar, __int64 lVal)
{
	CParam* pParam = GetParam(axpar);
	int style = pParam->style;
	int totalBytes = (style & PSY_TOTALBYTESMASK) >> PSY_TOTALBYTESSHIFT;
	int wordNum = (style & PSY_WORDNUMMASK) >> PSY_WORDNUMSHIFT;
	if (wordNum != 0)
	{
		LOGERROR("Param not first word");		// or maybe backtrack to first word?
		return false;
	}
	pParam->value = (WORD)lVal;
	int remainingBytes = totalBytes - 2;
	while (remainingBytes > 0)
	{
		lVal >>= 16;
		wordNum++;
		axpar.NextParam();
		pParam = GetParam(axpar);
		style = pParam->style;
		if (totalBytes != (style & PSY_TOTALBYTESMASK) >> PSY_TOTALBYTESSHIFT)
		{
			LOGERROR("Param total bytes mismatch");
			return false;
		}
		if (wordNum != (style & PSY_WORDNUMMASK) >> PSY_WORDNUMSHIFT)
		{
			LOGERROR("Param word num mismatch");
			return false;
		}
		pParam->value = (WORD)lVal;
		remainingBytes -= 2;
	}
//	if (remainingBytes == -1)
//		pParam->value &= 0x00ff;		// last word has only one byte
	return true;
}

bool CParamDoc::GetExParamValue(CAxPar axpar, __int64* plVal)
{
	CParam* pParam = GetParam(axpar);
	int style = pParam->style;
	int style0 = style;
	int totalBytes = (style & PSY_TOTALBYTESMASK) >> PSY_TOTALBYTESSHIFT;
	int wordNum = (style & PSY_WORDNUMMASK) >> PSY_WORDNUMSHIFT;
	if (wordNum != 0)
	{
		LOGERROR("Param not first word");		// or maybe backtrack to first word?
		return 0;
	}
	if (pParam->state & PSAB_UNKNOWN)
		return 0;
	*plVal = 0;
	WORD* pwVal = (WORD*)plVal;
	*pwVal = (WORD)pParam->value;

	int remainingBytes = totalBytes - 2;
	while (remainingBytes > 0)
	{
		wordNum++;
		axpar.NextParam();
		pParam = GetParam(axpar);
		style = pParam->style;
		if (totalBytes != (style & PSY_TOTALBYTESMASK) >> PSY_TOTALBYTESSHIFT)
		{
			LOGERROR("Param total bytes mismatch");
			return 0;
		}
		if (wordNum != (style & PSY_WORDNUMMASK) >> PSY_WORDNUMSHIFT)
		{
			LOGERROR("Param word num mismatch");
			return 0;
		}
		if (pParam->state & PSAB_UNKNOWN)
			return 0;
		pwVal++;			// point to next word in __int64
		*pwVal = (WORD)pParam->value;
		remainingBytes -= 2;
	}
	if ((short)*pwVal < -1 && !(style0 & PSY_UNSIGNED))
		while (++wordNum <= 3)		// if value is neg. and style is signed set remaining bytes to ff
			*(++pwVal) = (WORD)-1;
	
	return 1;
}

void CParamDoc::OnSendAllParam() 
{
	SendMonitorMessage(WMU_SENDALLPARAM);
}

void CParamDoc::OnRequestAllParam() 
{
	SendMonitorMessage(WMU_REQUESTALLPARAM);
}



void CParamDoc::OnPathSendNextData() 
{
//	SendMonitorMessage(WMU_SENDNEXTPATH);
}

void CParamDoc::OnPathSendAllData() 
{
//	SendMonitorMessage(WMU_SENDALLPATHDATA);
}

void CParamDoc::OnPathRequestNextData() 
{
	SendMonitorMessage(WMU_REQUESTNEXTPATHDATA);
}

void CParamDoc::OnPathRequestAllData() 
{
	SendMonitorMessage(WMU_REQUESTALLPATHDATA);
}

void CParamDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsModified());
}



void CParamDoc::UpdateMachineStateStatus(CMachineState* pMachineState)
{
	m_pMachineState = pMachineState;
	UpdateAllViews(NULL, HINT_MACHSTATE);
}

void CParamDoc::UpdateMachineBufferStatus(SContBufferSpan* pMachineBuffer)
{
	m_pMachineBuffer = pMachineBuffer;
	UpdateAllViews(NULL, HINT_MACHBUFF);
}






void CParamDoc::OnCloseDocument() 
{
	// TODO: Add your specialized code here and/or call the base class
	CDocument::OnCloseDocument();
}

BOOL CParamDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDocument::CanCloseFrame(pFrame);
}
