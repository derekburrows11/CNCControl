// ServoPath.cpp: implementation of the CServoPath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"
#include "ServoPath.h"

#include "AccelData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// following just for testing
CAccelData arTestPath[] = {
	{ 0,	ACCSEG_RAMP,	200,	23,	-3,	0		},
	{ 0,	ACCSEG_RAMP,	150,	15,	67,	30		},
	{ 0,	ACCSEG_RAMP,	120,	15,	60,	-30	},
	{ 0,	ACCSEG_RAMP,	100,	10,	7,		30		},
	{ 0,	ACCSEG_RAMP,	100,	10,	-40,	-30	},
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServoPath::CServoPath()
{
	m_arAccelData = arTestPath;
	m_nSizeAccelData = sizeof(arTestPath) / sizeof(arTestPath[0]);
	for (int i = 0; i < m_nSizeAccelData; i++)
		m_arAccelData[i].nStepCount = i;

	ResetPath();
}

CServoPath::~CServoPath()
{

}

CAccelData* CServoPath::GetNextData()
{
	if (m_nNextData < m_nSizeAccelData)
		return &m_arAccelData[m_nNextData++];
	else
		return NULL;
}

int CServoPath::GetNumPathObjs()
{
	return m_nSizeAccelData;
}

bool CServoPath::Finished()
{
	return (m_nNextData >= m_nSizeAccelData);
}

bool CServoPath::ResetPath()
{
	m_nNextData = 0;
	return true;
}
