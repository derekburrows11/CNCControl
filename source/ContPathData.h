// ContPathData.h: interface for the CContPathData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTPATHDATA_H__B88CB2C2_9B4B_11D7_86C3_FE0435E97625__INCLUDED_)
#define AFX_CONTPATHDATA_H__B88CB2C2_9B4B_11D7_86C3_FE0435E97625__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "Param.h"
#include "ControllerTracker.h"

class CContPathData  
{
public:
	CContPathData();
	virtual ~CContPathData();

	void SetBuffer(CSegAccelFIFO* pBuffer);
	CSegAccelFIFO* GetBuffer() { return m_pSegAccelFIFO; }
	bool GetNext(SSegAccel& sa);
	SSegAccel* GetNext();
	int GetCount() { return (m_pSegAccelFIFO == NULL) ? 0 : m_pSegAccelFIFO->GetCount(); }
	bool IsEmpty() { return (m_pSegAccelFIFO == NULL) ? true : m_pSegAccelFIFO->IsEmpty(); }
//	bool IsFinished() { return IsEmpty(); }

	void ResetTrackers();
	int GetCurrSegNum() { return m_iCurrSegNum; }		// count of last GetNext()
	int GetCurrSegEndTime() { return m_iCurrSegEndTime; }	// end time of last GetNext()


protected:
	CSegAccelFIFO* m_pSegAccelFIFO;	// buffer to receive calculated segments

	int m_numAccelAxes;

	// tracking values
	int m_iCurrSegNum;
	int m_iCurrSegEndTime;
	SMotionI m_MotICurr;
};

#endif // !defined(AFX_CONTPATHDATA_H__B88CB2C2_9B4B_11D7_86C3_FE0435E97625__INCLUDED_)
