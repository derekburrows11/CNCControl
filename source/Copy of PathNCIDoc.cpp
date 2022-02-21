// PathNCIDoc.cpp : implementation file
//

#include "stdafx.h"

#include "PolyFunc.h"

#include "ReadNCIFile.h"
#include "PathNCIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif






// Node relative properties
struct SSegRelProps;
struct SNodeRelProps
{
	NODEREF nr;
	CPathNode* pNode;
	CVector vtDirCng;
	double magDirCng;
	int nMagDirCng;		// DIR_CHANGE_NONE, DIR_CHANGE_SMALL or DIR_CHANGE_LARGE
	CVector vtCross;
	double magCross;
	double magCurve;				// curvature (radians / mm)
	CVector vtCurve;				// vtCross direction with mag of magCurve
	void SetDirChange(const SSegRelProps& segPrev, const SSegRelProps& segNext);
};

// Segment relative properties
struct SSegRelProps
{
	CVector vt;
	CVector vtUnit;
	double mag;
	int nMag;			// MAG_SHORT or MAG_LONG
	int nType;
	int num;
	bool bValid;
	CVector vtCrossCng;
	double magCrossCng;

	void SetSegment(const CVector& vtSeg);
	void SetCrossChange(const SNodeRelProps& ndPrev, const SNodeRelProps& ndNext);
};

void SSegRelProps::SetSegment(const CVector& vtSeg)
{
	vt = vtSeg;
	mag = vtSeg.Mag();
	vtUnit = vtSeg / mag;
}

void SSegRelProps::SetCrossChange(const SNodeRelProps& ndPrev, const SNodeRelProps& ndNext)
{
	vtCrossCng.Cross(ndPrev.vtCross, ndNext.vtCross);	// could take dir change of vtCrossUnit
	magCrossCng = vtCrossCng.Mag();
}

void SNodeRelProps::SetDirChange(const SSegRelProps& segPrev, const SSegRelProps& segNext)
{
	vtDirCng = segNext.vtUnit - segPrev.vtUnit;
	magDirCng = vtDirCng.Mag();						// is radians for small angles
	vtCross.Cross(segPrev.vt, segNext.vt);			// curve plane at this node
	magCross = vtCross.Mag();
	magCurve = 2 * magDirCng / (segPrev.mag + segNext.mag);	// curvature at this node
	vtCurve = vtCross * (magCurve / magCross);
}













/////////////////////////////////////////////////////////////////////////////
// CPathNCIDoc

IMPLEMENT_DYNCREATE(CPathNCIDoc, CPathDoc)

CPathNCIDoc::CPathNCIDoc()
{
}

BOOL CPathNCIDoc::OnNewDocument()
{
	if (!CPathDoc::OnNewDocument())
		return FALSE;
	return TRUE;
}

CPathNCIDoc::~CPathNCIDoc()
{
}


BEGIN_MESSAGE_MAP(CPathNCIDoc, CPathDoc)
	//{{AFX_MSG_MAP(CPathNCIDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPathNCIDoc diagnostics

#ifdef _DEBUG
void CPathNCIDoc::AssertValid() const
{
	CPathDoc::AssertValid();
}

void CPathNCIDoc::Dump(CDumpContext& dc) const
{
	CPathDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPathNCIDoc serialization

#include "PolySegDblFit.h"

void CPathNCIDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		CReadNCIFile nciFile;
		nciFile.InterperateFile(this, ar.GetFile());
//		nciFile.InterperateFile(this, ar);

		FindNodeBounds();
		SmoothCurves();

		m_NodeArray.RemoveAll();
		m_NodeArray.SetSize(0);
		m_CurveInfoArray.SetSize(0);		// no longer relevent
		m_NodeArray.Copy(m_FittedBezierArray);		// set to fitted array
		FindNodeBounds();

  }
}

/////////////////////////////////////////////////////////////////////////////
// CPathNCIDoc commands

/*
	Converting short segment curves to beziers
	Formula:
		given two short connected segments lengths 'L1, L2' with small change in direction 'theta'
		curve = theta * 2 / (L1 + L2)
		radius = (L1 + L2) / (2 * theta)
		max error b/w chord and arc = L^2 * curve / 8


*/


void CPathNCIDoc::SmoothCurves()
{
	if (m_iNumSegments < 2)
		return;

	// method
	bool bUsingLengths   = 0;	// one or the other!
	bool bUsingArcErrors = 1;

	// For all methods
	// 1 rad = 57.3 deg,  0.1 rad ~= 6 deg
	double angChangeSmall = 5 * deg2rad;			// was 3 deg
	double angChangeNone = 0.06e-3 * deg2rad;		// 0.06e-3 deg

	// Only for bUsingArcErrors method
	double errArcSmall = 0.100;										// mm was 0.050, 0.016
	double errArcSmallDirSet = errArcSmall * 22.0 / 16.0;		// mm

	// Only for bUsingLengths method
	double magSegShort = 2;							// mm maybe 3mm?

	

	enum
	{
		MAG_SHORT,
		MAG_LONG,
		DIR_CHANGE_NONE,
		DIR_CHANGE_SMALL,
		DIR_CHANGE_LARGE,
		ERROR_NONE,
		ERROR_SMALL,
		ERROR_LARGE,
		ST_NOTSET,
		ST_STRAIGHT,
		ST_CURVED,

	};

	bool bInCurve = false;
	bool bGotActualCurve = false;		// set when a bend is encounted

	m_CurveInfoArray.SetSize(0,512);
	m_FittedBezierArray.SetSize(0,1024);
	m_nrLastAddedToFittedArray = -1;		// start value


	SCurveInfo curve;
	curve.bStartDirSet = false;
	curve.bEndDirSet = false;
	curve.numSegs = 0;
	int iNumStraightSegs = 0;		// not in straight

	NODEREF nrLastDirChange;
	bool bEndCurveDirSetAtLastDirChange;
	CVector vtEndCurveDirAtLastDirChange;

	NODEREF nrStartStraight;
	CVector vtStartStraightDir;

	SNodeRelProps ndPrev, ndCurr, ndNext;
	SSegRelProps segPrev, segNext;

	segNext.vt = 0.0;
	segNext.mag = -1;
	segNext.num = 0;
	segNext.bValid = false;
	segNext.nType = ST_NOTSET;


	ndNext.pNode = GetFirstNode(ndNext.nr);
	if (ndNext.pNode == NULL)
		return;			// end of list
	ASSERT(ndNext.pNode->type & PNTF_END);		// first will be end node

	// Set initial condition - set segPrev.vt so change from segPrev.vt to segNext.vt is large
	ndCurr.nr = ndNext.nr;					// preload next node without changing nrNext
	ndCurr.pNode = GetNextNode(ndCurr.nr);		// setup so segPrev.vt is opposite segNext.vt - therefor change is large
	if (ndCurr.pNode == NULL)
		return;			// end of list

	bool bAtEndOfNodes = false;
	while (!bAtEndOfNodes)
	{
		ndPrev = ndCurr;
		ndCurr = ndNext;
		segPrev = segNext;

		segNext.nType = ST_NOTSET;

		ndNext.pNode = GetNextNode(ndNext.nr);		// only returns PNTF_NODE types, use GetNodeAbs(nr) if required
		CPathNode*& pNextNode = ndNext.pNode;
		if (pNextNode == NULL)		// end of list		- curve must be finalised
		{
			bAtEndOfNodes = true;
			pNextNode = ndPrev.pNode;	// setup so segPrev.vt is opposite segNext.vt - therefor change is large
			ASSERT(ndCurr.pNode->type & PNTF_END);		// last will be end node
		}

		ASSERT(pNextNode->type & (PNTF_END | PNTF_CONTROL));	// must be end or control
		segNext.bValid = (pNextNode->type & PNTF_END) && (ndCurr.pNode->type & PNTF_END);
		if (pNextNode->type & PNTF_END)
			segNext.num++;

		if (!segPrev.bValid && !segNext.bValid)		// neither segments are valid
		{
			ASSERT(!bInCurve);
			continue;
		}
		if (!segNext.bValid && !bInCurve)				// next seg not valid but not in curve so end dir won't be needed
			continue;

		if (!segPrev.bValid)				// (ndPrev is control) if prev seg was not calculated, do it now - may be needed for start curve dir
		{
			segPrev.SetSegment(*ndCurr.pNode - *ndPrev.pNode);		// don't do on first pass!
			segPrev.nMag = (segPrev.mag <= magSegShort) ? MAG_SHORT : MAG_LONG;
		}
		segNext.SetSegment(*pNextNode - *ndCurr.pNode);
		segNext.nMag = (segNext.mag <= magSegShort) ? MAG_SHORT : MAG_LONG;

		if (segNext.mag == 0)		// then load the next node into pNextNode and retry
		{
			ASSERT(0);
			continue;		// skip past null segments
		}

		
		// got Prev and Next vectors, now check direction change

		ndCurr.SetDirChange(segPrev, segNext);

		// 1 rad = 57.3 deg,  0.1 rad ~= 6 deg,  0.0175 rad ~= 1 deg
		if (ndCurr.magDirCng > angChangeSmall)
			ndCurr.nMagDirCng = DIR_CHANGE_LARGE;
		else
			ndCurr.nMagDirCng = (ndCurr.magDirCng <= angChangeNone) ? DIR_CHANGE_NONE : DIR_CHANGE_SMALL;


		double errArcPrev, errArcNext;
		int iErrArcPrev, iErrArcNext;
		double errArcPrevIfDirSet, errArcNextIfDirSet;
		int iErrArcPrevIfDirSet, iErrArcNextIfDirSet;

		if (ndCurr.magDirCng > 0.8)		// above ~ 45 deg
			iErrArcPrev = iErrArcNext = iErrArcPrevIfDirSet = iErrArcNextIfDirSet = ERROR_LARGE;
		else if (ndCurr.nMagDirCng == DIR_CHANGE_NONE)
			iErrArcPrev = iErrArcNext = iErrArcPrevIfDirSet = iErrArcNextIfDirSet = ERROR_NONE;
		else
		{
		//	double radius = (segPrev.mag + segNext.mag) * 0.5 / ndCurr.magDirCng;	// ndCurr.magDirCng = angle (if small)
		//	derrPrev = (segPrev.mag*segPrev.mag) * 0.125 / radius;
		//	double curve = 2 * ndCurr.magDirCng / (segPrev.mag + segNext.mag);
			double errCoeff = 0.25 * ndCurr.magDirCng / (segPrev.mag + segNext.mag);	// = curve/8
			errArcPrev = segPrev.mag*segPrev.mag * errCoeff;
			errArcNext = segNext.mag*segNext.mag * errCoeff;
			//	curve = 2 * ndCurr.magDirCng / magSeg;							// if dir is set from other segment
			errArcPrevIfDirSet = 0.25 * ndCurr.magDirCng * segPrev.mag;	// if dir is set from other segment
			errArcNextIfDirSet = 0.25 * ndCurr.magDirCng * segNext.mag;
			iErrArcPrev = (errArcPrev <= errArcSmall) ? ERROR_SMALL : ERROR_LARGE;
			iErrArcNext = (errArcNext <= errArcSmall) ? ERROR_SMALL : ERROR_LARGE;
			iErrArcPrevIfDirSet = (errArcPrevIfDirSet <= errArcSmallDirSet) ? ERROR_SMALL : ERROR_LARGE;
			iErrArcNextIfDirSet = (errArcNextIfDirSet <= errArcSmallDirSet) ? ERROR_SMALL : ERROR_LARGE;
		}


		// set allowable small dir changes for arc errors
		if (iErrArcPrev == ERROR_SMALL && iErrArcNext == ERROR_SMALL)
			ndCurr.nMagDirCng = DIR_CHANGE_SMALL;
//		else if (iErrArcPrevIfDirSet == ERROR_SMALL || iErrArcNextIfDirSet == ERROR_SMALL)
//			ndCurr.nMagDirCng = DIR_CHANGE_SMALL;


		int nMagDirChange = ndCurr.nMagDirCng;		// DIR_CHANGE_NONE, DIR_CHANGE_SMALL or DIR_CHANGE_LARGE

/*
	Using segment lengths and direction change:
	prevSeg | nextSeg | Dir Match | Result
		l			s			no				start curve cusp
		l			s			close			start curve smooth
		l			s			same			append to prev long seg
		s			l			no				end curve cusp
		s			l			close			end curve smooth
		s			l			same			end curve at prev node, append to next long seg
		l			l			no				cusp, straight segments
		l			l			close			cusp, or add smoothing segment
		l			l			same			append segments (or leave)
		s			s			no				cusp - start or end curve
		s			s			close			continue curve
		s			s			same			continue curve or append
*/

if (bUsingLengths)
{
		bool bCouldStartCurve = false;
		bool bCouldEndCurve = false;

		// check start curve conditions
		if (!bInCurve)
		{
			curve.numSegs = 0;
			if (segPrev.nMag == MAG_LONG && segNext.nMag == MAG_SHORT)
				switch (nMagDirChange)
				{
				case DIR_CHANGE_LARGE:		// start curve at this node, cusp
					bCouldStartCurve = true;
					curve.nrStart = ndCurr.nr;
					curve.bStartDirSet = false;
					break;
				case DIR_CHANGE_SMALL:		// start curve at this node, match end direction
					bCouldStartCurve = true;
					curve.nrStart = ndCurr.nr;
					curve.bStartDirSet = true;
					curve.vtStartDir = segPrev.vt;
					break;
				case DIR_CHANGE_NONE:		// start curve at later node where direction changes
					break;
				}
		}

		// check end curve conditions
		if (bInCurve)
		if ((segPrev.nMag == MAG_SHORT && segNext.nMag == MAG_LONG) || !segNext.bValid)		// curve must be finalised if !bSegNextValid
		{
			switch (nMagDirChange)
			{
			case DIR_CHANGE_LARGE:		// end curve at this node, cusp
				bCouldEndCurve = true;
				curve.nrEnd = ndCurr.nr;
				curve.bEndDirSet = false;
				break;
			case DIR_CHANGE_SMALL:		// end curve at this node, match end direction
				bCouldEndCurve = true;
				curve.nrEnd = ndCurr.nr;
				curve.bEndDirSet = true;
				curve.vtEndDir = segNext.vt;
				break;
			case DIR_CHANGE_NONE:		// end curve at earlier node where direction changes
				bCouldEndCurve = true;
				curve.nrEnd = nrLastDirChange;
				curve.bEndDirSet = bEndCurveDirSetAtLastDirChange;
				if (curve.bEndDirSet)
					curve.vtEndDir = vtEndCurveDirAtLastDirChange;	// valid if bEndDirSetAtLastDirChange == true
				break;
			}
			FindCurve(curve);
			bInCurve = false;
		}
		else if (segPrev.nMag == MAG_SHORT && segNext.nMag == MAG_SHORT)
			if (nMagDirChange == DIR_CHANGE_LARGE)
			{
				bCouldEndCurve = true;
				curve.nrEnd = ndCurr.nr;
				curve.bEndDirSet = false;

				FindCurve(curve);
				bInCurve = false;

				bCouldStartCurve = true;
			}



	// store info for possible use later at end curve
	if (bInCurve && (nMagDirChange != DIR_CHANGE_NONE))
	{
		nrLastDirChange = ndCurr.nr;
		bEndCurveDirSetAtLastDirChange = (nMagDirChange == DIR_CHANGE_SMALL);
		if (bEndCurveDirSetAtLastDirChange)
			vtEndCurveDirAtLastDirChange = segNext.vt;
	}



	if (bInCurve)
		curve.numSegs++;		// add segNext.vt to count after all checks

}	// if (bUsingLengths)


/*
	Using arc errors:
	iErrArcPrev		iErrArcPrevIfDirSet
			iErrArcNext		iErrArcNextIfDirSet
		s		s			x		x				continue, or start curve from start of prev seg

		s		l			s		x				end curve at mid node - smooth, end dir = next seg
		s		l			l		s				[not likely] end curve at mid node - smooth, end dir = prev seg
		s		l			l		l				end curve at mid node - cusp

		l		s			s		x				[not likely] end curve at mid node - smooth, end dir = next seg
		l		s			l		x				end curve at mid node - smooth, end dir = ~prev seg

		l		l			x		x				end curve at mid node - cusp
*/

if (bUsingArcErrors)	// using arc errors
{

	bool bArcError = (iErrArcPrev == ERROR_LARGE) || (iErrArcNext == ERROR_LARGE);
	// check end curve conditions
	if (bInCurve)
	{
		if (nMagDirChange == DIR_CHANGE_NONE)		// keep track of consecutive straight sections
			if (iNumStraightSegs == 0)			// if not in straight
			{
				nrStartStraight = ndPrev.nr;		// check doesn't get here first time!!
				vtStartStraightDir = segPrev.vt;
				iNumStraightSegs = 2;
			}
			else
				iNumStraightSegs++;
		else									// changing direction
			iNumStraightSegs = 0;		// not in straight


		if (bArcError || !segNext.bValid || (iNumStraightSegs >= 3))		// end curve signals
		{
			curve.nrEnd = ndCurr.nr;
			curve.bEndDirSet = false;
			if (iErrArcPrev <= ERROR_SMALL || iErrArcNext <= ERROR_SMALL)		// possible smooth join
				switch (nMagDirChange)
				{
				case DIR_CHANGE_LARGE:		// end curve at this node, cusp
//					ASSERT(0);					// should not really get here if one arc error is small!
					break;
				case DIR_CHANGE_SMALL:		// end curve at this node, match end direction
					curve.bEndDirSet = true;
					if (iErrArcPrevIfDirSet <= ERROR_SMALL)
					{
						curve.vtEndDir = segNext.vt;
						bGotActualCurve = true;
						segNext.nType = ST_STRAIGHT;
					}
					else if (iErrArcNextIfDirSet <= ERROR_SMALL)
					{
						curve.vtEndDir = segPrev.vt;		// not usual - probably will not have enough segments in curve section!
						//ASSERT(!bGotActualCurve);			// if curve tightens up bGotActualCurve will be set
					}
					else
						ASSERT(0);
					break;
				case DIR_CHANGE_NONE:		// end curve at start of straight segments
					curve.bEndDirSet = true;				// dir will be cont if all in a curve
					if (iNumStraightSegs >= 3)
					{
						curve.nrEnd = nrStartStraight;		// valid even if iNumStraightSegs <= 2
						curve.vtEndDir = vtStartStraightDir;
						curve.numSegs -= iNumStraightSegs-1;		// curve.numSegs is inc later but iNumStraightSegs is inc previously
					}
					else		// must have got here with !bSegNextValid
						curve.vtEndDir = segNext.vt;
					break;
				}
			// found end of curve segments, fit curve
			if (bGotActualCurve)
			{
				ASSERT(curve.numSegs > 1 || curve.bStartDirSet || curve.bEndDirSet)	;	// check if curve applicable
				FindCurve(curve);
			}
			bInCurve = false;
		}
		else		// curve not ended
			if (nMagDirChange != DIR_CHANGE_NONE)
				bGotActualCurve = true;
	}


	// check start curve conditions
	if (!bInCurve && segNext.bValid)
	{
		curve.numSegs = 0;	
		iNumStraightSegs = 0;		// not in straight
		bInCurve = true;
		bGotActualCurve = false;	// set when a bend is encounted
		curve.nrStart = ndCurr.nr;
		switch (nMagDirChange)
		{
		case DIR_CHANGE_LARGE:			// start curve at this node, cusp
			ASSERT(bArcError);			// should not really get here if !bArcError
			curve.bStartDirSet = false;
			break;
		case DIR_CHANGE_SMALL:			// start curve at this node, match end direction
			if (segNext.nType == ST_STRAIGHT)		// next seg has been used as a straight reference already
				bInCurve = false;
			else if (iErrArcNextIfDirSet <= ERROR_SMALL)
			{
				curve.bStartDirSet = true;
				curve.vtStartDir = segPrev.vt;
				bGotActualCurve = true;		// needed for single short seg between longer straight ones with small angles
			}
			else if (iErrArcPrevIfDirSet <= ERROR_SMALL)
			{
				//curve.bStartDirSet = true;			// don't set start dir to segNext.vt!
				//curve.vtStartDir = segNext.vt;
			}
			else			// large segments - don't curve!
				bInCurve = false;
			break;
		case DIR_CHANGE_NONE:			// start curve at later node where direction changes
			if (segPrev.bValid)			// unless prev seg was invalid (bezier curve) then allow a start
				bInCurve = false;
			else
			{
				curve.bStartDirSet = true;
				curve.vtStartDir = segPrev.vt;
			}
			break;
		}
	}


	if (bInCurve)
		curve.numSegs++;		// add segNext.vt to count after all checks

}	// if (bUsingArcErrors)



	}	// while (!bAtEndOfNodes)

	// add remaining nodes to m_FittedBezierArray
	NODEREF nr = m_nrLastAddedToFittedArray;
	NODEREF nrLast;
	GetLastPathNode(nrLast);
	while (++nr <= nrLast)
		m_FittedBezierArray.Add(*GetPathNode(nr));
	m_nrLastAddedToFittedArray = nrLast;


/*	CPathNode nd;
	nd.type = PNT_BREAKPATH;
	nd.x = 0;					// noderef for end
	m_FittedBezierArray.Add(nd);
*/

}	// CPathNCIDoc::SmoothCurves()

/*

for curves

length:
< 3
adjacent segs within 1:3 length ratio, norm 1:2

max arc error:
0.014
often <0.005

max dir change:
0.04
often ~0.032		(2deg = 0.035rad)

for straight
max dir change:
< 5e-7



*/


void CPathNCIDoc::FindCurve(SCurveInfo& ci)
{
	ASSERT(ci.numSegs > 1 || ci.bStartDirSet || ci.bEndDirSet)	;	// check if curve applicable
// store list of curve info
	m_CurveInfoArray.Add(ci);


	// Add all nodes from origional array to m_FittedBezierArray up to and including ci.nrStart
	NODEREF nr = m_nrLastAddedToFittedArray;		// starts at -1
	while (++nr <= ci.nrStart)
		m_FittedBezierArray.Add(*GetPathNode(nr));

	// Add any command nodes directly after start node and check for none later in curve section - or curve should be broken(not handled yet!)
	int iSeg = 0;
	for (; nr <= ci.nrEnd; nr++)
		if (GetPathNode(nr)->IsCommand())
			if (iSeg == 0)			// OK to add if just after first node!
				m_FittedBezierArray.Add(*GetPathNode(nr));
			else
			{
				TRACE("Have to insert command node in smoothed path!!! in CPathNCIDoc::FindCurve()\n");
				ASSERT(0);		// end current fit and add this node
			}
		else			// is point node of a segment
			iSeg++;
	ASSERT(iSeg > 0);
	ASSERT(nr == ci.nrEnd + 1);
	m_nrLastAddedToFittedArray = ci.nrEnd;			// will be after fitted curve added



	// find number of constraints
	if (ci.bStartDirSet) ci.bStartDirSet = 1;		// make sure true is 1
	if (ci.bEndDirSet) ci.bEndDirSet = 1;
	int numEndDirsSet = ci.bStartDirSet + ci.bEndDirSet;
	int numPoints = ci.numSegs + 1;
	ASSERT(numPoints + numEndDirsSet >= 3);

	// check curvature of ajoining segments if direction is set to it
	bool bStartStraight, bEndStraight;
	nr = ci.nrStart;
	CPathNode* pNode = GetPrevNode(nr);
	// if prev seg is part of a fitted curve then must be disjoin anyway, curving if not end to end segment
	bStartStraight = ci.bStartDirSet && pNode && (pNode->type & PNTF_END);
	nr = ci.nrEnd;
	pNode = GetNextNode(nr);
	// if next seg is part of a fitted curve then must be disjoin anyway, curving if not end to end segment
	bEndStraight = ci.bEndDirSet && pNode && (pNode->type & PNTF_END);

	// set nr location of this curve in an PNT_BREAKPATH node - don't do
	CPathNode ndCi, ndCf;
//	ndCi.type = PNT_BREAKPATH;
//	ndCi.x = ci.nrStart;					// get noderef of start
//	m_FittedBezierArray.Add(ndCi);

	// set first node pointers
	CPathNode *pndInit, *pnd1, *pnd2, *pndFinal;
	nr = ci.nrStart;
	pndInit = GetNode(nr);		// these are PNT_ENDNODE
	ASSERT(pndInit->type & PNTF_END);

	if (numPoints == 3)
		pnd1 = GetNextNode(nr);
	if (numPoints <= 3)
	{
		pndFinal = GetNextNode(nr);
		ASSERT(nr == ci.nrEnd);
	}

	// select appropriate fit algorithm
	bool bStore1Seg = true;

	if (numPoints >= 5)	// fit cubic - 4 points with 1 or 2 end dir's, or 5 or more points
	{
		// OK for 3D
		bStore1Seg = false;
		FitCubicToCurve(ci);
	}
	else if (numPoints == 4)
	{
		if (numEndDirsSet >= 1)
		{
			// OK for 3D
			bStore1Seg = false;
			FitCubicToCurve(ci);
		}
		else		// fit exact cubic
		{
			// OK for 3D
			pnd1 = GetNextNode(nr);
			pnd2 = GetNextNode(nr);
			pndFinal = GetNextNode(nr);
			ASSERT(nr == ci.nrEnd);
			FitBezierTo4Points(*pndInit, *pnd1, *pnd2, *pndFinal, ndCi, ndCf);
			// just check it, not been used yet!
		}
	}
	else if (numPoints == 2 && numEndDirsSet == 1)			// fit exact arc
	{
		// OK for 3D
		if (ci.bStartDirSet)
			if (bStartStraight)
				FitBezierToNoCurveArcDir(*pndInit, ci.vtStartDir, *pndFinal, ndCi, ndCf);	// all CVector base
			else 
				FitBezierToArcDir(*pndInit, ci.vtStartDir, *pndFinal, ndCi, ndCf);	// all CVector base
		else if (ci.bEndDirSet)
		{
			CVector vtEndDir = -ci.vtEndDir;
			if (bEndStraight)
				FitBezierToNoCurveArcDir(*pndFinal, vtEndDir, *pndInit, ndCf, ndCi);	// all CVector base
			else
				FitBezierToArcDir(*pndFinal, vtEndDir, *pndInit, ndCf, ndCi);	// all CVector base
		}
	}
	else if (numPoints == 3 && numEndDirsSet == 0)	// fit exact arc
	{
		// OK for 3D
		FitBezierToArc3Point(*pndInit, *pnd1, *pndFinal, ndCi, ndCf);	// all CVector base
	}
	else if ((numPoints == 2 || numPoints == 3) && numEndDirsSet == 2)
	{	// if 3 points ignore mid point and do as for (numPoints == 2 && numEndDirsSet == 2) or 	// fit cubic: dir of control nodes seg is avg of end directions, translate to fit mid point
		// set bezier as cubic with ends and dir's set, ok for small dir changes
		// calculate as 1D poly using vtChord as base axis, control nodes perpendicular from thirds along vtChord
		// OK for 3D
		CVector& vtInitDir = ci.vtStartDir;
		CVector& vtFinalDir = ci.vtEndDir;
		CVector vtChord = *pndFinal - *pndInit;
		double magChordSqOn3 = vtChord.MagSq() / 3;
		double CiVectScale = magChordSqOn3 / dot(vtChord, vtInitDir);
		ndCi = *pndInit + vtInitDir * CiVectScale;
		double CfVectScale = magChordSqOn3 / dot(vtChord, vtFinalDir);
		ndCf = *pndFinal - vtFinalDir * CfVectScale;
	}
	else if (numPoints == 3 && numEndDirsSet == 1)	// fit cubic: dir of control nodes seg is avg of end directions, translate (node of set dir) to fit mid point
	{	// set bezier as cubic with ends and one dir set, other dir set to include mid point, ok for small dir changes
		// calculate as 1D poly using vtChord as base axis, control nodes perpendicular from thirds along vtChord
		// OK for 3D
		bool bStartDirSet = ci.bStartDirSet;
		CVector* pvtDir;
		if (bStartDirSet)
			pvtDir = &ci.vtStartDir;
		else
			pvtDir = &ci.vtEndDir;

		CVector vtChord = *pndFinal - *pndInit;
		double magChordSq = vtChord.MagSq();
		double CiVectScale = magChordSq / (3*dot(vtChord, *pvtDir));
		if (bStartDirSet)
			ndCi = *pndInit + *pvtDir * CiVectScale;
		else
			ndCf = *pndFinal - *pvtDir * CiVectScale;

		// find s for mid point if s = [0,1] - vtChord parallel component moves linearly from init to final
		//	double s = dot(vtChord, *pnd1 - *pndInit) / magChordSq;		// can give s near 0 or 1
		double magSeg1 = (*pnd1 - *pndInit).Mag();
		double magSeg2 = (*pndFinal - *pnd1).Mag();
		double s = magSeg1 / (magSeg1 + magSeg2);
		ASSERT(s > 0 && s < 1);

		//	val = n0(1-3s+3s^2-s^3) + 3c0(s-2s^2+s^3) + 3c1(s^2-s^3) + n1(s^3)
		double sp2 = s*s;
		double sp3 = s*sp2;
		if (bStartDirSet)
			ndCf = (*pnd1 -  *pndInit*(1+3*(sp2-s)-sp3) - ndCi*(3*(s-2*sp2+sp3)) - *pndFinal*(sp3)) / (3*(sp2-sp3));
		else
			ndCi = (*pnd1 - *pndFinal*(1+3*(sp2-s)-sp3) - ndCf*(3*(s-2*sp2+sp3)) -  *pndInit*(sp3)) / (3*(sp2-sp3));
	}
	else
	{
		bStore1Seg = false;
		ASSERT(0);
	}

	if (bStore1Seg)
	{
		ndCi.type = ndCf.type = PNT_CONTROLNODE;
//		m_FittedBezierArray.Add(*pndInit);		don't add start!
		m_FittedBezierArray.Add(ndCi);
		m_FittedBezierArray.Add(ndCf);
		m_FittedBezierArray.Add(*pndFinal);
	}

}

void CPathNCIDoc::FitOneCubicToCurve(SCurveInfo& ci)
{
	int numPts = ci.numSegs + 1;
	int numEndDirsSet = ci.bStartDirSet + ci.bEndDirSet;
	ASSERT(numPts + numEndDirsSet >= 5);

	CPolySegFit segFit;
	CPathNode *pndInit, *pndFinal;

	NODEREF nr = ci.nrStart;
	pndInit = GetNode(nr);
	segFit.AddPoint(*pndInit);
	for (int i = 1; i < numPts; i++)
		segFit.AddPoint(*GetNextNode(nr));
	pndFinal = GetNode(nr);
	ASSERT(nr == ci.nrEnd);

	segFit.SetToInitialPoint();
	segFit.SetToFinalPoint();
	if (ci.bStartDirSet)
		segFit.SetInitialDir(ci.vtStartDir);
	if (ci.bEndDirSet)
		segFit.SetFinalDir(ci.vtEndDir);
	segFit.SolveCubic();

	CPathNode ndCi, ndCf;
	ndCi = segFit.m_Bez[1];
	ndCf = segFit.m_Bez[2];

	ndCi.type = ndCf.type = PNT_CONTROLNODE;
//	m_FittedBezierArray.Add(*pndInit);
	m_FittedBezierArray.Add(ndCi);
	m_FittedBezierArray.Add(ndCf);
	m_FittedBezierArray.Add(*pndFinal);
}


enum PlaneType
{
	PT_NOTSET = 0,
	PT_PLANAR,
	PT_NONPLANAR,
};
enum Dir
{
	DIR_FREE = 0,
	DIR_MATCHED,
	DIR_SET,
};
enum Loc
{
	LOC_FREE = 0,
	LOC_RANGE,
	LOC_FIXED,
};
enum CurveChange		// curve change reasons
{
	CC_NOTSET = 0,
	CC_ENDOFPLANE,
	CC_NONPLANAR2PLANAR,
	CC_DIRCHANGEOVER,
	CC_LASTNODE,
};


struct SCurveFit
{
	enum PlaneType nType;
	// start and end members
	enum Dir nStartDir, nEndDir;		// don't really need, always fit from previous unless first or last
	enum Loc nStartLoc, nEndLoc;		// don't really need, always fit from previous unless first or last
	CVector vtStartDir, vtEndDir;		// don't really need, always fit from previous unless first or last
	NODEREF nrStart, nrEnd;
	// end members
	int iEndRangeHigh, iEndRangeLow;
	enum CurveChange nCurveChangeEnd;
	// summing members
	int numSegs;
	double magDirCngSum;
	double magSegSum;

	void Reset();
	void SetStartFrom(const SCurveFit& fitStart);
	void SetEndFrom(const SCurveFit& fitEnd);
	void AddSumsFrom(const SCurveFit& other);
};

void SCurveFit::Reset()
{
	nStartLoc = nEndLoc;
	nrStart = nrEnd;
	nType = PT_NOTSET;
	nCurveChangeEnd = CC_NOTSET;
	nEndLoc = LOC_FREE;
	nStartDir = nEndDir = DIR_FREE;
	magDirCngSum = magSegSum = 0;
	numSegs = 0;
}

void SCurveFit::SetStartFrom(const SCurveFit& fitStart)
{
	nrStart = fitStart.nrStart;
	nStartLoc = fitStart.nStartLoc;
	nStartDir = fitStart.nStartDir;
	vtStartDir = fitStart.vtStartDir;
}

void SCurveFit::SetEndFrom(const SCurveFit& fitEnd)
{
	nrEnd = fitEnd.nrEnd;
	nEndLoc = fitEnd.nEndLoc;
	nEndDir = fitEnd.nEndDir;
	vtEndDir = fitEnd.vtEndDir;
	iEndRangeHigh = fitEnd.iEndRangeHigh;
	iEndRangeLow = fitEnd.iEndRangeLow;
	nCurveChangeEnd = fitEnd.nCurveChangeEnd;
}

void SCurveFit::AddSumsFrom(const SCurveFit& other)
{
	numSegs += other.numSegs;
	magSegSum += other.magSegSum;
	magDirCngSum += other.magDirCngSum;
}


struct SCurveFitThresholds
{
	double resolutionValues;
	double normComponentToPlane;
	double magCrossCngPlane;

	double magDirCngBreak;
	double magDirCngLow;
	int numSegsLow;
	int numSegsMin;

	SCurveFitThresholds();
};

SCurveFitThresholds::SCurveFitThresholds()
{
	resolutionValues = 1e-6;			// nci file numbers are 6 decimal place
	normComponentToPlane = resolutionValues * 10;
	magCrossCngPlane = resolutionValues * 100;
	magDirCngBreak = 45 * deg2rad;
	magDirCngLow   = 10 * deg2rad;
	numSegsLow = 5;
	numSegsMin = 3;	// would rather 4
}



void CPathNCIDoc::FitCubicToCurve(SCurveInfo& ci)
{
	int numPts = ci.numSegs + 1;
	int numEndDirsSet = ci.bStartDirSet + ci.bEndDirSet;
	ASSERT(numPts + numEndDirsSet >= 5);

	CPathNode* pndInit = GetNode(ci.nrStart);
	CPathNode* pndFinal = GetNode(ci.nrEnd);

	CPathNode nd;
//	nd = *pndInit;
//	nd.type = PNT_ENDNODE;
//	m_FittedBezierArray.Add(nd);  dont' add start!


	SNodeRelProps ndPrev2, ndPrev, ndCurr, ndNext;
	SSegRelProps segPrev2, segPrev, segNext;


	SCurveFitThresholds m_Threshold;
	SCurveFit curveFit;
	CList<SCurveFit, SCurveFit&> curveFitList;

	// init values
	curveFit.Reset();
	curveFit.numSegs = -1;		// allow for first node
	curveFit.nrStart = ci.nrStart;
	curveFit.nStartLoc = LOC_FIXED;

	ndNext.nr = ci.nrStart;
	ndNext.pNode = GetNode(ndNext.nr);
	ndNext.magDirCng = 0;		// is summed later
	segNext.num = 0;
	if (ci.bStartDirSet)
	{
		segNext.vt = ci.vtStartDir;
		segNext.mag = segNext.vt.Mag();
		curveFit.nStartDir = DIR_SET;		// free, set or matched to next
		curveFit.vtStartDir = ci.vtStartDir;
	}
	else
	{
		segNext.vt = 0.0;
		segNext.mag = -1;
		curveFit.nStartDir = DIR_FREE;		// free, set or matched to next
	}
	segNext.vtUnit = segNext.vt / segNext.mag;
	segPrev.mag = -1;			// not valid

	// setup for double cubic fitting
	CPolySegDblFit dblPolyFit;
	dblPolyFit.SetToInitialPoint();
	if (ci.bStartDirSet)
		dblPolyFit.SetInitialDir(ci.vtStartDir);



	enum PlaneType nPlaneType = PT_NOTSET;
	CVector vtPlaneCurveUnit;
	double dotSegToPlaneCurve;
	bool bOnLastNode;

	for (int i = 0; i < numPts; i++)
	{
		// advance node relative
		ndPrev2 = ndPrev;
		ndPrev = ndCurr;
		ndCurr = ndNext;
		// advance segment relative
		segPrev2 = segPrev;
		segPrev = segNext;

		bOnLastNode = (ndCurr.nr == ci.nrEnd);				// true if got to end

		// sum curveFit values up to current node
		curveFit.numSegs++;
		if (ndCurr.nr != ci.nrStart)
			curveFit.magSegSum += segPrev.mag;						// Sum segment lengths to ndCurr
		if (segPrev2.mag > 0)								// if segPrev2 valid
			curveFit.magDirCngSum += ndPrev.magDirCng;			// Sum total direction change to ndCurr

		ASSERT(ndCurr.pNode->type & PNTF_NODE);
		dblPolyFit.AddPoint(*ndCurr.pNode);


		if (!bOnLastNode)				// not at end
		{
			CPathNode* pNodeNext;
			for (;;)
			{
				ndNext.pNode = GetNextPathNode(ndNext.nr);	// have to insert non PNTF_NODE types in smoothed path!!!
				pNodeNext = ndNext.pNode;
				ASSERT(pNodeNext != NULL);
				if (pNodeNext->IsPoint())
					break;
				// have to insert non PNTF_NODE types in smoothed path!!!
				if (i != 0)			// OK to add if just after first node!
				{
					TRACE("Have to insert non PNTF_NODE types in smoothed path!!! in CPathNCIDoc::FitCubicToCurve()\n");
					ASSERT(0);		// end current fit and add this node
				}
			}
			ASSERT(pNodeNext->type == PNT_ENDNODE);	// must be end or control
			if (pNodeNext->type & PNTF_END)	// if control should go to next end node
				segNext.num++;


			// calculations requiring segNext to be valid
			// calculate next segment props from current and next nodes
			segNext.SetSegment(*ndNext.pNode - *ndCurr.pNode);
			
			if (segNext.mag == 0)		// then load the next node into pNextNode and retry
			{
				ASSERT(0);
				continue;		// skip past null segments
			}

			if (segPrev.mag <= 0)					// if segPrev not valid
				continue;
			// calculate node relative props from adjacent segments
			ndCurr.SetDirChange(segPrev, segNext);
			// calculate segment relative props using node relative props
			segPrev.SetCrossChange(ndPrev, ndCurr);



/*
	Check node locations for:
		[Segment change cusp]
			change in direction (cusp)
		[Segment change smooth]
			change in line type (straight <-> curved)
			change in plane or (planar <-> non-planar)
			step change in curvature
			step change in d(curvature) / dist
		[Segment change smooth, non specific location]
			unable to fit poly within tolerance
			total segment direction change


	Previous check method:
		change between planar/non-planar
		direction change above limit
		sudden change in direction
		sudden change in curvature
		unable to fit poly within tolerance
*/

			if (nPlaneType == PT_NOTSET)
			{
				if (curveFit.numSegs + (curveFit.nStartDir == DIR_SET ? 1:0) >= 3)		// need at least 3 segs to check
				{
					if (segPrev.magCrossCng <= m_Threshold.magCrossCngPlane)		// if CrossCng small then plane of curve not changing
					{
						nPlaneType = PT_PLANAR;
						vtPlaneCurveUnit = ndCurr.vtCross / ndCurr.magCross;
					}
					else
						nPlaneType = PT_NONPLANAR;
				}
			}
			else	// (nPlaneType != PT_NOTSET)
			{
				if (nPlaneType == PT_PLANAR)
				{
					dotSegToPlaneCurve = fabs(dot(vtPlaneCurveUnit, segNext.vt));		// must be ~0 if in current plane!
					if (dotSegToPlaneCurve > m_Threshold.normComponentToPlane)			// out of current plane
					{
						curveFit.nType = nPlaneType;
						curveFit.nrEnd = ndCurr.nr;			// earliest end node
						curveFit.iEndRangeHigh = 1;
						curveFit.iEndRangeLow = 0;				// end can be from node to next node
						curveFit.nEndLoc = LOC_RANGE;			// not fixed at nrEnd
						curveFit.nEndDir = DIR_MATCHED;
						curveFit.nCurveChangeEnd = CC_ENDOFPLANE;
						curveFitList.AddTail(curveFit);
						curveFit.Reset();
						curveFit.nStartDir = DIR_MATCHED;
						nPlaneType = PT_NOTSET;					// recheck from start of next curve fit
					}
				}
				else	// (nPlaneType == PT_NONPLANAR)		check last >=3 segs are planar, start new curve fit if so
				{
					if (segPrev.magCrossCng <= m_Threshold.magCrossCngPlane)			// got 4 planar segments, change to planar curve
					{
						curveFit.nType = nPlaneType;
						curveFit.nrEnd = ndPrev2.nr;			// start new curve from node before ndPrev
						curveFit.iEndRangeHigh = 0;
						curveFit.iEndRangeLow = -1;			// end can be from previous node to node
						curveFit.nEndLoc = LOC_RANGE;			// not fixed at nrEnd
						curveFit.nEndDir = DIR_MATCHED;
						curveFit.nCurveChangeEnd = CC_NONPLANAR2PLANAR;
						curveFit.numSegs -= 2;
						curveFit.magSegSum -= segPrev.mag + segPrev2.mag;
						curveFit.magDirCngSum -= ndPrev.magDirCng + ndPrev2.magDirCng;
						curveFitList.AddTail(curveFit);
						curveFit.Reset();
						curveFit.numSegs = 2;
						curveFit.magSegSum = segPrev.mag + segPrev2.mag;
						curveFit.magDirCngSum = ndPrev.magDirCng + ndPrev2.magDirCng;
						curveFit.nStartDir = DIR_MATCHED;
						nPlaneType = PT_PLANAR;
						vtPlaneCurveUnit = ndCurr.vtCross / ndCurr.magCross;
					}
				}
			}

			if (curveFit.numSegs >= 3)
			{
				double magDirCngSumIncCurr = curveFit.magDirCngSum + ndCurr.magDirCng;
				double magDirCngBreakThres = m_Threshold.magDirCngBreak;
				if (curveFit.numSegs <= 4)
					magDirCngBreakThres *= 1.5;
				if (magDirCngSumIncCurr >= magDirCngBreakThres)		// break curve fit if dir change above magDirCngBreak
				{
					// current DirChange will be included in next fit
					curveFit.nType = nPlaneType;
					curveFit.nrEnd = ndCurr.nr;			// break curve at ndCurr
					curveFit.nEndLoc = LOC_FREE;		// not fixed at nrEnd
					curveFit.nCurveChangeEnd = CC_DIRCHANGEOVER;
					curveFit.nEndDir = DIR_MATCHED;		// free, set or matched to next
					curveFitList.AddTail(curveFit);
					curveFit.Reset();					// reset sums and counters, keep current type
					curveFit.nStartDir = DIR_MATCHED;		// free, set or matched to next
					// nPlaneType = ;					// keep same type
				}
			}

		}
		else	//	(bOnLastNode)				// got to end
		{
			curveFit.nType = nPlaneType;
			curveFit.nrEnd = ndCurr.nr;		// break curve at ndCurr
			curveFit.nEndLoc = LOC_FIXED;		// fixed at nrEnd
			curveFit.nCurveChangeEnd = CC_LASTNODE;
			if (ci.bEndDirSet)
			{
				curveFit.nEndDir = DIR_SET;		// free, set or matched to next
				curveFit.vtEndDir = ci.vtEndDir;
			}
			else
				curveFit.nEndDir = DIR_FREE;		// free, set or matched to next
			curveFitList.AddTail(curveFit);
			curveFit.Reset();
			nPlaneType = PT_NOTSET;				// recheck from start of next curve fit
		}


		int numFits = curveFitList.GetCount();
		if (bOnLastNode)
		{
/*			
	if numFit >= 2
	{
		if curvechange is CC_DIRCHANGEOVER and next seg is not, and low segs and low dir change
			combine
		set dblPolyFit points to end of fit 2
		set join to end of fit 1
		set end conditions
		fit
		OK? -> no, move ends
		set init pos and dir of fit 2 to fitted join
		remove first half dblPolyFit
		remove fit 1
	}
	repeat
*/
		
		// scan through fit list and combine small fits to adjacent ones
		// if curvechange is CC_DIRCHANGEOVER and next seg is not, and low segs and low dir change then combine
		// if curvechange is CC_DIRCHANGEOVER and next seg is not, and low segs and high dir change then move start of next back so dir change about equal
			POSITION posFitCurr, posFitNext;
			posFitNext = curveFitList.GetHeadPosition();
			if (posFitNext != NULL)
				for (;;)
				{
					posFitCurr = posFitNext;
					SCurveFit& fitCurr = curveFitList.GetAt(posFitCurr);
					curveFitList.GetNext(posFitNext);
					// do checks that don't require fitNext - to be done on every curveFit
					ASSERT(fitCurr.numSegs >= m_Threshold.numSegsMin);		// check will have enough fit points!

					if (posFitNext == NULL)
						break;
					SCurveFit& fitNext = curveFitList.GetAt(posFitNext);

					if (fitCurr.nCurveChangeEnd == CC_DIRCHANGEOVER && fitNext.nCurveChangeEnd != CC_DIRCHANGEOVER)
						if (fitNext.numSegs <= m_Threshold.numSegsLow)
							if (fitNext.magDirCngSum < m_Threshold.magDirCngLow)	// low dir change
							{
								// Add fitCurr to fitNext and remove fitCurr
								ASSERT(fitNext.nType == fitCurr.nType);
								//fitNext.SetEndFrom(fitNext);
								fitNext.SetStartFrom(fitCurr);
								fitNext.AddSumsFrom(fitCurr);
								curveFitList.RemoveAt(posFitCurr);
								continue;				// fitCurr no longer valid!
							}
							else				// high dir change
							{
								// Move start of fitNext back so fitCurr and fitNext have similar dir change
								ASSERT(fitNext.nType == fitCurr.nType);
								int numSegsNext = (fitCurr.numSegs + fitNext.numSegs) / 2;
								if (numSegsNext <= m_Threshold.numSegsLow)		// combine fitNext into fitCurr
								{
									fitNext.SetStartFrom(fitCurr);
									fitNext.AddSumsFrom(fitCurr);
									curveFitList.RemoveAt(posFitCurr);
									continue;				// fitCurr no longer valid!
								}
								ASSERT(fitCurr.nEndDir != DIR_SET && fitNext.nStartDir != DIR_SET);
								int numSegsMove = numSegsNext - fitNext.numSegs;	// seg to move back
								fitNext.numSegs += numSegsMove;
								fitCurr.numSegs -= numSegsMove;
								// move end/start node ref				- summing members will be wrong!
								GetOffsetPointNodes(fitNext.nrStart, -numSegsMove);	// move back by numSegsMove point nodes
								fitCurr.nrEnd = fitNext.nrStart;
							}

				}	// for (;;)

			for (;;)		// fit curves to fit list
			{
				numFits = curveFitList.GetCount();
				if (numFits >= 2)
				{
					posFitNext = curveFitList.GetHeadPosition();
					SCurveFit& fitCurr = curveFitList.GetNext(posFitNext);
					SCurveFit& fitNext = curveFitList.GetAt(posFitNext);
					dblPolyFit.SetPolyJoinPointRef(fitCurr.numSegs);		// doesn't handle no int point ref yet!!
					dblPolyFit.SetNumPointsToUse(fitCurr.numSegs + fitNext.numSegs + 1);

					dblPolyFit.FitDoubleSegment();

					nd = dblPolyFit.GetBaseBezierNode(1);
					nd.type = PNT_CONTROLNODE;
					m_FittedBezierArray.Add(nd);
					nd = dblPolyFit.GetBaseBezierNode(2);
					m_FittedBezierArray.Add(nd);
					nd = dblPolyFit.GetBaseBezierNode(3);
					nd.type = PNT_ENDNODE;
					m_FittedBezierArray.Add(nd);
					

					dblPolyFit.SetInitialStateFromJoin();		// setup next start to continue from this fit
					dblPolyFit.RemoveFitAPoints();
					curveFitList.RemoveHead();
				}
				else if (numFits == 1)
				{
					SCurveFit& fitCurr = curveFitList.GetHead();
					if (fitCurr.nEndLoc == LOC_FIXED)		// fixed at nrEnd
						dblPolyFit.SetToFinalPoint();
					if (fitCurr.nEndDir == DIR_SET)		// free, set or matched to next
						dblPolyFit.SetFinalDir(fitCurr.vtEndDir);

					ASSERT(dblPolyFit.GetNumPointsStored() == fitCurr.numSegs + 1);
					dblPolyFit.UseAllPoints();
					dblPolyFit.FitSingleSegment();

					nd = dblPolyFit.m_Bez[1];
					nd.type = PNT_CONTROLNODE;
					m_FittedBezierArray.Add(nd);
					nd = dblPolyFit.m_Bez[2];
					m_FittedBezierArray.Add(nd);
					nd = dblPolyFit.m_Bez[3];
					nd.type = PNT_ENDNODE;
					m_FittedBezierArray.Add(nd);
					break;
				}
				else
				{
					ASSERT(0);
					break;
				}
			}	// for (;;)

			break;		// finished last node
		}	// if (bOnLastNode)

		



	}	//	for (int i = 0; i < numPts; i++)


}

