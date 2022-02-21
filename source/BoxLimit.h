// BoxLimit.h: interface for the CBoxLimit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOXLIMIT_H__AFC27410_77A6_481E_9F53_38258CDB712C__INCLUDED_)
#define AFX_BOXLIMIT_H__AFC27410_77A6_481E_9F53_38258CDB712C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "Vector.h"


class CBoxLimit
{
public:
	CBoxLimit();
	virtual ~CBoxLimit();

	bool SolveMaxMa();
	bool SolveMaxMaOrig();		// origional version - not used now

public:
	// input data
	bool bSet;
	CVector vtA, vtB, vtBoxMin, vtBoxMax;
	bool bMaxMbBound;			// if max Ma can be from different bounds, selects one with max Mb

	// solved data
	int axEdgeDir;						// edge direction used -> vtBound[axEdge] == 0
	TVector<char> vtBound;			// each axis is [-1, 0, 1] due to [Min, None, Max] bound
	TVector<char> arvtBound[3];	// vtBound for each direction
	CVector vtBoundValue;			// each axis is [BoxMin, BoxMax] instead of [-1, 1] or edgeVal instead of 0
//	CVector vtAperpB;					// vtA component perpendicular to vtB and in the plane of vtA & vtB
	double Ma, Mb;
	double arMa[3];					// value of Ma for each axis view for bound change locating
	double arEdgeVal[3];				// value of edgeVal for each axis view for bound change locating
};

#endif // !defined(AFX_BOXLIMIT_H__AFC27410_77A6_481E_9F53_38258CDB712C__INCLUDED_)
