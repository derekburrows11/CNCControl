// ServoPath.h: interface for the CServoPath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVOPATH_H__2ABD1D86_E794_11D4_8C1E_FE9D690EF12C__INCLUDED_)
#define AFX_SERVOPATH_H__2ABD1D86_E794_11D4_8C1E_FE9D690EF12C__INCLUDED_

#include "Param.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// to be made obsolete
class CServoPath  
{
public:
	CServoPath();
	virtual ~CServoPath();

	bool ResetPath();
	bool Finished();
	CAccelData* GetNextData();
	int GetNumPathObjs();

protected:
	CAccelData* m_arAccelData;
	int m_nSizeAccelData;
	int m_nNextData;
};







#endif // !defined(AFX_SERVOPATH_H__2ABD1D86_E794_11D4_8C1E_FE9D690EF12C__INCLUDED_)
