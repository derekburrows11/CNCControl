// ContPathData.cpp: implementation of the CContPathData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"
#include "ContPathData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CContPathData::CContPathData()
{
	m_pSegAccelFIFO = NULL;
	m_numAccelAxes = NUM_AXIS;

	ResetTrackers();
}

CContPathData::~CContPathData()
{
}

///////////////

void CContPathData::SetBuffer(CSegAccelFIFO* pBuffer)
{
	m_pSegAccelFIFO = pBuffer;
}

void CContPathData::ResetTrackers()
{
	m_iCurrSegNum = -1;			// count of last GetNext()
	m_iCurrSegEndTime = 0;		// must be reset at start of path seg
	m_MotICurr.Zero();
}

bool CContPathData::GetNext(SSegAccel& sa)	// returns true if got seg accel
{
	ASSERT(m_pSegAccelFIFO != NULL);
	if (m_pSegAccelFIFO->IsEmpty())
		return false;
	sa = m_pSegAccelFIFO->Remove();
	m_iCurrSegNum++;
	ASSERT(sa.iSegCount == m_iCurrSegNum);
	sa.iSegCount = m_iCurrSegNum;
	m_iCurrSegEndTime += sa.idTime;
	ASSERT(sa.iTimeEnd == m_iCurrSegEndTime);
	m_MotICurr.UpdateMotion(sa);
	return true;
}

SSegAccel* CContPathData::GetNext()		// returns NULL if none available
{
	ASSERT(m_pSegAccelFIFO != NULL);
	if (m_pSegAccelFIFO->IsEmpty())
		return NULL;
	SSegAccel* pSA = &m_pSegAccelFIFO->Remove();	// pointer garenteed until next is removed
	m_iCurrSegNum++;
	ASSERT(pSA->iSegCount == m_iCurrSegNum);
	pSA->iSegCount = m_iCurrSegNum;
	m_iCurrSegEndTime += pSA->idTime;
	ASSERT(pSA->iTimeEnd == m_iCurrSegEndTime);
	m_MotICurr.UpdateMotion(*pSA);
	return pSA;
}


