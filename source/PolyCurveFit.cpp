// PolyCurveFit.cpp: implementation of the CPolyCurveFit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"
#include "PolyCurveFit.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif






//////////////////////////////////////////////////////////////////////




void SSegRelProps::SetSegment(const CVector& vtSeg)
{
	vt = vtSeg;
	mag = vtSeg.Mag();
	vtUnit = vtSeg / mag;
}

void SNodeRelProps::SetDirChange(const SSegRelProps& segPrev, const SSegRelProps& segNext)
{
	vtDirCng = segNext.vtUnit - segPrev.vtUnit;
	magDirCng = vtDirCng.Mag();						// is radians for small angles - is area of (magCurve x length to mid segs)
	span = 0.5 * (segPrev.mag + segNext.mag);
	// actual angle and arc span are within ~2% of magDirCng and span for relevent small angle changes!!
//	angDirCng = 2 * asin(0.5*magDirCng);
//	chord = 0.5 * sqrt(segPrev.mag*segPrev.mag + segNext.mag*segNext.mag + 2*segPrev.mag*segNext.mag*cos(angDirCng));
//	arcSpan = chord * (angDirCng/magDirCng);
	magCurve = magDirCng / span;						// curvature at this node
//	magCurveError = 0.01 * 2 / (segPrev.mag * segNext.mag);		// * error in position of mid node (~0.01)
	vtCross.Cross(segPrev.vt, segNext.vt);			// curve plane at this node
	magCross = vtCross.Mag();
	vtCurve = vtCross * (magCurve / magCross);
	// scalarCurve is magCurve with sign of largest component of vtCurve
	scalarCurve = (vtCross.MaxAbsElem() < 0) ? -magCurve : magCurve;
	//scalarCurve = vtCurve.z;

	// vtd2Curve  can't be found yet

	//errArc = Curve/8 * magSeg^2
	errArcPrev = 0.125*magCurve * segPrev.mag*segPrev.mag;
	errArcNext = 0.125*magCurve * segNext.mag*segNext.mag;
}


void SSegRelProps::SetCrossChange(const SNodeRelProps& ndPrev, const SNodeRelProps& ndNext)
{
	magCurveCng = ndNext.magCurve - ndPrev.magCurve;
	scalarCurveCng = ndNext.scalarCurve - ndPrev.scalarCurve;
	vtCurveCng = ndNext.vtCurve - ndPrev.vtCurve;
	vtdCurve = vtCurveCng / mag;
	dScalarCurve = scalarCurveCng / mag;
	vtCrossCng.Cross(ndPrev.vtCross, ndNext.vtCross);	// could take dir change of vtCrossUnit
	magCrossCng = vtCrossCng.Mag();
}

void SNodeRelProps::SetCurveChange2(const SSegRelProps& segPrev, const SSegRelProps& segNext)
{
	vtd2Curve = (segNext.vtdCurve - segPrev.vtdCurve) / span;	// segNext.vtdCurve may not be set yet!!!
}






void SCurveFit::Reset()
{
	nrStart = nrEnd;
	nStartLoc = nEndLoc;
	nStartDir = nEndDir;
	nType = PT_NOTSET;
	nCurveChangeEnd = CC_NOTSET;
	nEndLoc = LOC_FREE;
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



SCurveFitThresholds::SCurveFitThresholds()
{
	resolutionValues = 1e-6;			// nci file numbers are 6 decimal place
	distNormToPlane = 0.01;				// was 0.01
	magCrossCngPlane = resolutionValues * 100;
	magDirCngBreak = 45 * deg2rad;
	magDirCngLow   = 10 * deg2rad;
	numSegsLow = 5;
	numSegsMin = 3;			// would rather 4
	lengthSetPlane = 2;		// min length required to set a curve plane
}










//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPolyCurveFit::CPolyCurveFit()
{
	m_pPath = NULL;

}

CPolyCurveFit::~CPolyCurveFit()
{

}


//////////////////////////////////////////////////////////////////////





// not used!!!
void CPolyCurveFit::FitOneCubicToCurve(SCurveInfo& ci)
{
	int numPts = ci.numSegs + 1;
	int numEndDirsSet = 0;
	if (ci.nStartDir == DIR_SET)
		numEndDirsSet++;
	if (ci.nEndDir == DIR_SET)
		numEndDirsSet++;
	ASSERT(numPts + numEndDirsSet >= 5);

	CPolySegFit segFit;
	CPathNode *pndInit, *pndFinal;

	NODEREF nr = ci.nrStart;
	pndInit = m_pPath->GetNode(nr);
	segFit.AddPoint(*pndInit);
	for (int i = 1; i < numPts; i++)
		segFit.AddPoint(*m_pPath->GetNextNode(nr));
	pndFinal = m_pPath->GetNode(nr);
	ASSERT(nr == ci.nrEnd);

	segFit.SetToInitialPoint();
	segFit.SetToFinalPoint();
	if (ci.nStartDir == DIR_SET)
		segFit.SetInitialDir(ci.vtStartDir);
	if (ci.nEndDir == DIR_SET)
		segFit.SetFinalDir(ci.vtEndDir);
	segFit.SolveCubic();

	CPathNode ndCi, ndCf;
	ndCi = segFit.m_Bez[1];
	ndCf = segFit.m_Bez[2];

	ndCi.type = ndCf.type = PNT_CONTROLNODE;
//	m_pPath->m_FittedBezierArray.Add(*pndInit);
	m_pPath->m_FittedBezierArray.Add(ndCi);
	m_pPath->m_FittedBezierArray.Add(ndCf);
	m_pPath->m_FittedBezierArray.Add(*pndFinal);
}







void CPolyCurveFit::FitCubicsToCurve(SCurveInfo& ci)
{
	ASSERT(m_pPath != NULL);

	int numPts = ci.numSegs + 1;
	int numEndDirsSet = 0;
	if (ci.nStartDir == DIR_SET)
		numEndDirsSet++;
	if (ci.nEndDir == DIR_SET)
		numEndDirsSet++;
	ASSERT(numPts + numEndDirsSet >= 5);

//	CPathNode* pndInit = m_pPath->GetNode(ci.nrStart);
//	CPathNode* pndFinal = m_pPath->GetNode(ci.nrEnd);


//	CPathNode nd;
//	nd = *pndInit;
//	nd.type = PNT_ENDNODE;
//	m_FittedBezierArray.Add(nd);  dont' add start!


	SNodeRelProps ndPrev2, ndPrev, ndCurr, ndNext;
	SSegRelProps segPrev2, segPrev, segNext;


	SCurveFit curveFit;
	CCurveFitList curveFitList;


	// init values
	curveFit.Reset();
	curveFit.numSegs = -1;		// allow for first node
	curveFit.nrStart = ci.nrStart;
	curveFit.nStartLoc = LOC_FIXED;

	ndPrev.pNode = NULL;
	ndCurr.pNode = NULL;
//	ndCurr.magRatioAbs = 0;		// not set
	ndNext.nr = ci.nrStart;
	ndNext.pNode = m_pPath->GetNode(ndNext.nr);
	ndNext.magDirCng = 0;		// is summed later
//	ndNext.magRatioAbs = 0;		// not set
	segNext.num = 0;
	if (ci.nStartDir == DIR_SET)
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


	int idxFBA = 0;
	ci.FitBreakArray.Add(-1);		// end of array flag
	while (ndNext.nr >= ci.FitBreakArray[idxFBA])
	{
		if (ci.FitBreakArray[idxFBA] == -1)
			break;
		idxFBA++;
	}

	// setup for double cubic fitting
	m_DblPolyFit.Reset();
	m_DblPolyFit.SetToInitialPoint();
	if (ci.nStartDir == DIR_SET)
		m_DblPolyFit.SetInitialDir(ci.vtStartDir);



	enum PlaneType& nPlaneType = curveFit.nType;
	CVector vtPlaneCurveUnit;

	for (int i = 0; i < numPts; i++)
	{
		// advance node relative
		ndPrev2 = ndPrev;
		ndPrev = ndCurr;
		ndCurr = ndNext;
		// advance segment relative
		segPrev2 = segPrev;
		segPrev = segNext;

		bool bOnLastNode = (ndCurr.nr == ci.nrEnd);		// true if got to end
		if (bOnLastNode)
			ASSERT(i == numPts-1);

		// sum curveFit values up to current node
		curveFit.numSegs++;
		if (ndCurr.nr != ci.nrStart)
			curveFit.magSegSum += segPrev.mag;				// Sum segment lengths to ndCurr
		if (segPrev2.mag > 0)									// if segPrev2 valid
			curveFit.magDirCngSum += ndPrev.magDirCng;	// Sum total direction change to ndCurr

		ASSERT(ndCurr.pNode->type & PNTF_NODE);
		m_DblPolyFit.AddPoint(*ndCurr.pNode);


		if (bOnLastNode)				// got to end
		{
			ASSERT(i == numPts-1);
			curveFit.nrEnd = ndCurr.nr;		// break curve at ndCurr
			curveFit.nEndLoc = LOC_FIXED;		// fixed at nrEnd
			curveFit.nCurveChangeEnd = CC_LASTNODE;
			if (ci.nEndDir == DIR_SET)
			{
				curveFit.nEndDir = DIR_SET;		// free, set or matched to next
				curveFit.vtEndDir = ci.vtEndDir;
			}
			else
				curveFit.nEndDir = DIR_FREE;		// free, set or matched to next

			if (curveFit.numSegs <= m_Threshold.numSegsMin)	// just add to previous fit for now!
				if (curveFitList.GetCount() >= 1)
				{
					SCurveFit& curveFitLast = curveFitList.GetTail();
					curveFitLast.AddSumsFrom(curveFit);
					curveFitLast.SetEndFrom(curveFit);
				}
				else
				{
					ASSERT(curveFit.numSegs == m_Threshold.numSegsMin);
					curveFitList.AddTail(curveFit);
				}
			else
				curveFitList.AddTail(curveFit);
			curveFit.Reset();

			FindCubics(curveFitList, ci);
			break;
		}

		// not on last node

		CPathNode* pNodeNext;
		for (;;)
		{
			ndNext.pNode = m_pPath->GetNextPathNode(ndNext.nr);	// have to insert non PNTF_NODE types in smoothed path!!!
			pNodeNext = ndNext.pNode;
			ASSERT(pNodeNext != NULL);
			if (pNodeNext->IsPoint())
				break;
			if (!pNodeNext->IsHiddenPoint())			// have to insert non PNTF_NODE types in smoothed path!!!
				if (i != 0)			// OK to add if just after first node!
				{
					TRACE("Have to insert non PNTF_NODE types in smoothed path!!! in CPolyCurveFit::FitCubicsToCurve()\n");
					ASSERT(0);		// end current fit and add this node
				}
		}
		ASSERT(pNodeNext->IsEndPoint());	// must be end or control
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
//		if (ndPrev.pNode == NULL)			// if start dir set segPrev.mag gets set with dir when first node is ndCurr
//			ndCurr.magRatioAbs = 0;		// segPrev was set to start dir, not actual segment
		// calculate segment relative props using node relative props
		segPrev.SetCrossChange(ndPrev, ndCurr);
		//ndPrev.SetCurveChange2(segPrev2, segPrev);



/*
Check node locations for:
	[Segment change cusp]
		change in direction (cusp)
	[Segment change smooth]
		change in line type (straight <-> curved)
		change in plane or (planar <-> non-planar)
		step change in curvature
		step change in d(curvature) / dist
		step change in seg length (generally associated with step change in curve)
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

		// checks for curve fit ending at ndPrev2
		// - check change from non-planar to planar
		if (nPlaneType == PT_NONPLANAR)		// check if last >=3 segs are planar, start new curve fit if so
		{
			if (segPrev.magCrossCng <= m_Threshold.magCrossCngPlane && curveFit.numSegs >= 5)	// got 4 planar segments, change to planar curve
			{
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
				nPlaneType = PT_PLANAR;
				vtPlaneCurveUnit = ndCurr.vtCross / ndCurr.magCross;
			}
		}

		// checks for curve fit ending at ndPrev
		// - check change in seg mag
/*		if (ndPrev.magRatioAbs >= 4 && ndCurr.magRatioAbs < 2 && ndPrev2.magRatioAbs < 2 && ndPrev2.magRatioAbs != 0)	// ndPrev2.magRatioAbs 0 at start!
			if (curveFit.numSegs >= 4)		// will be reduced by 1, min. 3 segs
			{		// step change in seg size at ndPrev
				curveFit.nrEnd = ndPrev.nr;			// start new curve from ndPrev
				curveFit.nEndLoc = LOC_FIXED;			// fixed at nrEnd
				curveFit.nEndDir = DIR_MATCHED;
				curveFit.nCurveChangeEnd = CC_SEGMAGCHANGE;
				curveFit.numSegs--;
				curveFit.magSegSum -= segPrev.mag;
				curveFit.magDirCngSum -= ndPrev.magDirCng;
				curveFitList.AddTail(curveFit);
				curveFit.Reset();
				curveFit.numSegs = 1;
				curveFit.magSegSum = segPrev.mag;
				curveFit.magDirCngSum = ndPrev.magDirCng;
			}
*/

		// checks for curve fit ending at ndCurr
		// - check step change in curve
		ASSERT(ndCurr.nr <= ci.FitBreakArray[idxFBA] || ci.FitBreakArray[idxFBA] == -1);
		if (ndCurr.nr == ci.FitBreakArray[idxFBA])
		{
			idxFBA++;
			ASSERT(ci.FitBreakArray[idxFBA] > ndCurr.nr || ci.FitBreakArray[idxFBA] == -1);
			if (curveFit.numSegs > m_Threshold.numSegsMin)
			{
				ASSERT(curveFit.numSegs >= m_Threshold.numSegsMin);		// check will have enough fit points!
				curveFit.nrEnd = ndCurr.nr;			// earliest end node
				curveFit.nEndLoc = LOC_FIXED;			// fixed at nrEnd
				curveFit.nEndDir = DIR_MATCHED;
				curveFit.nCurveChangeEnd = CC_CURVESTEPCHANGE;
				curveFitList.AddTail(curveFit);
				curveFit.Reset();
			}
		}

		// - check change end of plane
		if (nPlaneType == PT_PLANAR)
		{
			double distOffPlane = fabs(dot(vtPlaneCurveUnit, segNext.vt));		// must be ~0 if in current plane!
			if (distOffPlane > m_Threshold.distNormToPlane)			// out of current plane
			{
				curveFit.nrEnd = ndCurr.nr;			// earliest end node
				curveFit.iEndRangeHigh = 1;
				curveFit.iEndRangeLow = 0;				// end can be from node to next node
				curveFit.nEndLoc = LOC_RANGE;			// not fixed at nrEnd
				curveFit.nEndDir = DIR_MATCHED;
				curveFit.nCurveChangeEnd = CC_ENDOFPLANE;
				curveFitList.AddTail(curveFit);
				curveFit.Reset();
			}
		}

/*		// - check number of segments
		if (curveFit.numSegs >= 3)
		{
			double magDirCngSumIncCurr = curveFit.magDirCngSum + ndCurr.magDirCng;
			double magDirCngBreakThres = m_Threshold.magDirCngBreak;
			if (curveFit.numSegs <= 4)
				magDirCngBreakThres *= 1.5;
			else
				magDirCngBreakThres *= 6.0 / curveFit.numSegs;
			if (magDirCngSumIncCurr >= magDirCngBreakThres)		// break curve fit if dir change above magDirCngBreak
			{
				// current DirChange will be included in next fit
				curveFit.nrEnd = ndCurr.nr;			// break curve at ndCurr
				curveFit.nEndLoc = LOC_FREE;		// not fixed at nrEnd
				curveFit.nCurveChangeEnd = CC_DIRCHANGEOVER;
				curveFit.nEndDir = DIR_MATCHED;		// free, set or matched to next
				curveFitList.AddTail(curveFit);
				curveFit.Reset();					// reset sums and counters, keep current type
			}
		}
*/

		
		// check curve type
		if (nPlaneType == PT_NOTSET
				&& curveFit.numSegs + (curveFit.nStartDir == DIR_SET ? 1:0) >= 2)		// (was >=3) need at least 3 segs to check (2 + segNext)
			if (curveFit.magSegSum + segNext.mag >= m_Threshold.lengthSetPlane)	// check to ndNext
			{
				// if start dir not set get vector from start to curr node
				CVector vtNodeStart = *m_pPath->GetNode(curveFit.nrStart);
				CVector vtNextRelStart = *ndNext.pNode - vtNodeStart;
				double distNextRelStart = vtNextRelStart.Mag();
				CVector vtPlaneAlongUnit;
				if (curveFit.nStartDir == DIR_SET)
					curveFit.vtStartDir.Unit(vtPlaneAlongUnit);
				else
					vtPlaneAlongUnit = vtNextRelStart / distNextRelStart;

				// get cross prod of each seg
				CVector vtNodeRelStart;
				CVector vtNodeCross;
				CVector vtNodeCrossMax;
				double distSqCrossMax = 0;		// Mag Squared
				for (NODEREF nr = curveFit.nrStart;;)
				{
					CVector* pNode = m_pPath->GetNextNode(nr);
					if (nr >= ndNext.nr)
						break;
					vtNodeRelStart = *pNode - vtNodeStart;
					vtNodeCross.Cross(vtPlaneAlongUnit, vtNodeRelStart);
					double distSqCross = vtNodeCross.MagSq();
					if (distSqCross > distSqCrossMax)
					{
						distSqCrossMax = distSqCross;
						vtNodeCrossMax = vtNodeCross;
					}
				}
				ASSERT(nr == ndNext.nr);
				// magCrossMax represents confidence of plane - square of perp dist from vtCurrRelStart
				double aspectPlane = sqrt(distSqCrossMax) / distNextRelStart;

				if (aspectPlane >= 0.05)		// plane aspect ratio
				{
					vtPlaneCurveUnit = vtNodeCrossMax / sqrt(distSqCrossMax);
					curveFit.vtPlaneCurveUnit = vtPlaneCurveUnit;
					curveFit.fPlaneAspectRatio = aspectPlane;
					nPlaneType = PT_PLANAR;
					// test all nodes on curve plane
					for (nr = curveFit.nrStart;;)
					{
						CVector* pNode = m_pPath->GetNextNode(nr);
						if (nr >= ndNext.nr)
							break;
						vtNodeRelStart = *pNode - vtNodeStart;
						double distOffPlane = fabs(dot(vtPlaneCurveUnit, vtNodeRelStart));
						if (distOffPlane > m_Threshold.distNormToPlane)
						{
							nPlaneType = PT_NONPLANAR;
							break;
						}
					}
				}
		}

	}	//	for (int i = 0; i < numPts; i++)

}


void CPolyCurveFit::FindCubics(CCurveFitList& curveFitList, SCurveInfo& ci)
{
	int numFits = curveFitList.GetCount();

	int numPtsTotal = ci.numSegs + 1;
	ASSERT(ci.pNodeSumsArray != NULL);
	CNodeSumsArray& nodeSumsArray = *(CNodeSumsArray*)ci.pNodeSumsArray;
	ASSERT(nodeSumsArray.GetSize() == numPtsTotal);
	ASSERT(nodeSumsArray[0].nr == ci.nrStart);
	ASSERT(nodeSumsArray[numPtsTotal-1].nr == ci.nrEnd);

/*			
	if numFit >= 2
	{
		if curvechange is CC_DIRCHANGEOVER and next seg is not, and low segs and low dir change
			combine
		set m_DblPolyFit points to end of fit 2
		set join to end of fit 1
		set end conditions
		fit
		OK? -> no, move ends
		set init pos and dir of fit 2 to fitted join
		remove first half m_DblPolyFit
		remove fit 1
	}
	repeat
*/
		
// scan through fit list and combine small fits to adjacent ones
// if curvechange is CC_DIRCHANGEOVER and next seg is not, and low segs and low dir change then combine
// if curvechange is CC_DIRCHANGEOVER and next seg is not, and low segs and high dir change then move start of next back so dir change about equal
	POSITION posFitCurr, posFitNext;
	posFitNext = curveFitList.GetHeadPosition();
	const int segsMax = 10;
	int iStartNode;
	int iEndNode = 0;
	for (int idxFit = 0;; idxFit++)
	{
		posFitCurr = posFitNext;
		if (posFitNext == NULL)
			break;
		bool bSecondHalfOfFits = 2*idxFit >= numFits;	// won't be set if bMiddleFit set!
		bool bMiddleFit = 2*idxFit == numFits-1;
		ASSERT(!(bMiddleFit && bSecondHalfOfFits));
		SCurveFit& fitCurr = curveFitList.GetNext(posFitNext);
		// do checks that don't require fitNext - to be done on every curveFit
		ASSERT(fitCurr.numSegs >= m_Threshold.numSegsMin);		// check will have enough fit points!
		iStartNode = iEndNode;
		iEndNode += fitCurr.numSegs;
		ASSERT(fitCurr.nrStart == nodeSumsArray[iStartNode].nr);
		ASSERT(fitCurr.nrEnd == nodeSumsArray[iEndNode].nr);

		int numSegs = fitCurr.numSegs;
		if (numSegs > segsMax)
		{
			SCurveFit fitNew = fitCurr;
			//fitNew.SetStartFrom(fitCurr);
			int iFitStartNode;
			int iFitEndNode = iStartNode;
			int numNewFits = 1 + (numSegs-1) / segsMax;
			int segsPerFit = numSegs / numNewFits;
			int rem = numSegs % numNewFits;
			int sum = -numNewFits/2;
			// adjust sum so round's up if >=0.5 instead of >0.5 in second half of fits
			sum = bSecondHalfOfFits ? -(numNewFits+1)/2 : -(numNewFits+2)/2;
			for (int i = 0; i < numNewFits; i++)
			{
				fitNew.numSegs = segsPerFit;
				if (bMiddleFit && 2*i == numNewFits)	// if at centre adjust sum to round up if >=0.5
					sum += (numNewFits+1) % 2;				// +1 if numNewFits even
				sum += rem;
				if (sum >= 0)
				{
					sum -= numNewFits;
					fitNew.numSegs++;
				}
				iFitStartNode = iFitEndNode;
				iFitEndNode += fitNew.numSegs;
				fitNew.magDirCngSum = nodeSumsArray[iFitEndNode].magDirCngSum - nodeSumsArray[iFitStartNode].magDirCngSum;
				//fitNew.l = nodeSumsArray[iFitEndNode].l - nodeSumsArray[iFitStartNode].l;
				fitNew.nrStart = nodeSumsArray[iFitStartNode].nr;
				fitNew.nrEnd = nodeSumsArray[iFitEndNode].nr;
				curveFitList.InsertBefore(posFitCurr, fitNew);
			}
			ASSERT(fitNew.nrEnd == fitCurr.nrEnd);
			ASSERT(iFitEndNode == iEndNode);
			POSITION posRemove = posFitCurr;
			curveFitList.GetPrev(posFitCurr);
			curveFitList.RemoveAt(posRemove);
		}

		if (posFitNext != NULL)
		{
			SCurveFit& fitCurr = curveFitList.GetAt(posFitCurr);
			SCurveFit& fitNext = curveFitList.GetAt(posFitNext);
			if (fitCurr.nCurveChangeEnd == CC_DIRCHANGEOVER && fitNext.nCurveChangeEnd != CC_DIRCHANGEOVER)
				if (fitNext.numSegs <= m_Threshold.numSegsLow)
					if (fitNext.magDirCngSum < m_Threshold.magDirCngLow)	// low dir change
					{
						// Add fitCurr to fitNext and remove fitCurr
						ASSERT(fitNext.nType == fitCurr.nType || fitNext.nType == PT_NOTSET);
						//fitNext.SetEndFrom(fitNext);
						fitNext.SetStartFrom(fitCurr);
						fitNext.AddSumsFrom(fitCurr);
						curveFitList.RemoveAt(posFitCurr);
						continue;				// fitCurr no longer valid!
					}
					else				// high dir change
					{
						// Move start of fitNext back so fitCurr and fitNext have similar dir change
						ASSERT(fitNext.nType == fitCurr.nType || fitNext.nType == PT_NOTSET);
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
						m_pPath->GetOffsetPointNodes(fitNext.nrStart, -numSegsMove);	// move back by numSegsMove point nodes
						fitCurr.nrEnd = fitNext.nrStart;
					}
		}
	}	// for (int idxFit = 0;; idxFit++)
	ASSERT(idxFit == numFits);

	CPathNode nd;
	for (;;)		// fit curves to fit list
	{
		numFits = curveFitList.GetCount();
		int nrPathNext = m_pPath->m_FittedBezierArray.GetSize() - 1;

		if (numFits >= 2)
		{
			posFitNext = curveFitList.GetHeadPosition();
			SCurveFit& fitCurr = curveFitList.GetNext(posFitNext);
			SCurveFit& fitNext = curveFitList.GetAt(posFitNext);
			m_DblPolyFit.SetPolyJoinPointRef(fitCurr.numSegs);		// doesn't handle non int point ref yet!!
			int iPntsOfNext = min(fitNext.numSegs, 6);
			m_DblPolyFit.SetNumPointsToUse(fitCurr.numSegs + 1 + iPntsOfNext);

			m_DblPolyFit.FitDoubleSegment();
			if (m_DblPolyFit.GetMaxResidual() >= 0.3)
				LOGMESSAGE1("Large Maximum Residual in FitDoubleSegment() of: %g", m_DblPolyFit.GetMaxResidual());

			double minCont = m_DblPolyFit.GetBaseMinControlSpan();
			if (minCont < 0.05)
				LOGERROR2("Small control node ratio (%g) in smoothing double bezier first node %i", minCont, nrPathNext);


			nd = m_DblPolyFit.GetBaseBezierNode(1);
			nd.type = PNT_CONTROLNODE;
			m_pPath->m_FittedBezierArray.Add(nd);
			nd = m_DblPolyFit.GetBaseBezierNode(2);
			m_pPath->m_FittedBezierArray.Add(nd);
			nd = m_DblPolyFit.GetBaseBezierNode(3);
			nd.type = PNT_ENDNODE;
			m_pPath->m_FittedBezierArray.Add(nd);
			

			m_DblPolyFit.SetInitialStateFromJoin();		// setup next start to continue from this fit
			m_DblPolyFit.RemoveFitAPoints();
			curveFitList.RemoveHead();
		}
		else if (numFits == 1)
		{
			SCurveFit& fitCurr = curveFitList.GetHead();
			if (fitCurr.nEndLoc == LOC_FIXED)		// fixed at nrEnd
				m_DblPolyFit.SetToFinalPoint();
			if (fitCurr.nEndDir == DIR_SET)		// free, set or matched to next
				m_DblPolyFit.SetFinalDir(fitCurr.vtEndDir);

			ASSERT(m_DblPolyFit.GetNumPointsStored() == fitCurr.numSegs + 1);
			m_DblPolyFit.UseAllPoints();

			m_DblPolyFit.FitSingleSegment();
			if (m_DblPolyFit.GetMaxResidual() >= 0.3)
				LOGMESSAGE1("Large Maximum Residual in FitSingleSegment() of: %g", m_DblPolyFit.GetMaxResidual());

			double minCont = m_DblPolyFit.GetMinControlSpan();
			if (minCont < 0.05)
				LOGERROR2("Small control node ratio (%g) in smoothing single bezier first node %i", minCont, nrPathNext);
			

			nd = m_DblPolyFit.m_Bez[1];
			nd.type = PNT_CONTROLNODE;
			m_pPath->m_FittedBezierArray.Add(nd);
			nd = m_DblPolyFit.m_Bez[2];
			m_pPath->m_FittedBezierArray.Add(nd);
			nd = m_DblPolyFit.m_Bez[3];
			nd.type = PNT_ENDNODE;
			if (fitCurr.nEndLoc == LOC_FIXED)		// fixed at nrEnd - set type from point!
			{
				ASSERT((*m_pPath->GetNode(fitCurr.nrEnd) - nd).SumAbs() <= 1e-4);
				nd.type = m_pPath->GetNode(fitCurr.nrEnd)->type;
			}
			m_pPath->m_FittedBezierArray.Add(nd);
			break;
		}
		else
		{
			ASSERT(0);
			break;
		}
	}	// for (;;)
}


