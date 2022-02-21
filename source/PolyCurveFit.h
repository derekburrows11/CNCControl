// PolyCurveFit.h: interface for the CPolyCurveFit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POLYCURVEFIT_H__26ED5BCC_12C0_4AE1_81D7_162B38A202C5__INCLUDED_)
#define AFX_POLYCURVEFIT_H__26ED5BCC_12C0_4AE1_81D7_162B38A202C5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "PathDoc.h"
#include "PolySegDblFit.h"


/////////////////////////////////////////////////////////////////////////////



struct SCurveFitThresholds
{
	double resolutionValues;
	double distNormToPlane;
	double magCrossCngPlane;
	double lengthSetPlane;

	double magDirCngBreak;
	double magDirCngLow;
	int numSegsLow;
	int numSegsMin;

	SCurveFitThresholds();
};

struct SCurveFit
{
	enum PlaneType nType;
	CVector vtPlaneCurveUnit;
	double fPlaneAspectRatio;
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
typedef CList<SCurveFit, SCurveFit&> CCurveFitList;


// Node relative properties
struct SSegRelProps;
struct SNodeRelProps
{
	NODEREF nr;
	CPathNode* pNode;
	double l;				// distance on node line (cumulative sum of previous segments)
	double span;			// (segPrev + segNext) / 2
//	double magRatioAbs;	// just for checking
//	double angDirCng;		// just for checking
//	double chord;			// just for checking
//	double arcSpan;		// just for checking
//	double magCurveError;		// just for checking
	CVector vtDirCng;
	double magDirCng;
	double magDirCngSum;	// cumulative sum of magDirCng
	int nMagDirCng;		// DIR_CHANGE_NONE, DIR_CHANGE_SMALL or DIR_CHANGE_LARGE
	CVector vtCross;
	double magCross;
	CVector vtCurve;				// vtCross direction with mag of magCurve
	double magCurve;				// curvature (radians / mm)
	double scalarCurve;			// curvature (radians / mm) with sign of most significant vtCurve component
	CVector vtd2Curve;			// change in vtdCurve from segPrev to segNext / span

	double errArcPrev;
	double errArcNext;

	double ratioSplit;
	double magCurveSplit;
	double split;

	double dCurveFit;
	double dCurveFitResidual;

	void SetDirChange(const SSegRelProps& segPrev, const SSegRelProps& segNext);		// prerequisit: SetSegment()
	void SetCurveChange2(const SSegRelProps& segPrev, const SSegRelProps& segNext);	// prerequisit: SetCrossChange()
};

// Segment relative properties
struct SSegRelProps
{
	CVector vt;
	CVector vtUnit;
	double mag;
	int nMag;					// MAG_SHORT or MAG_LONG
	int nType;
	int num;
	bool bValid;
	double magCurveCng;
	double scalarCurveCng;
	double dScalarCurve;
	CVector vtCurveCng;
	CVector vtdCurve;			// change in vtCurve from prev node

	double magCrossCng;
	CVector vtCrossCng;

	void SetSegment(const CVector& vtSeg);						// prerequisit: none
	void SetCrossChange(const SNodeRelProps& ndPrev, const SNodeRelProps& ndNext);	// prerequisit: SetDirChange()
};

struct SNodeSums
{
	NODEREF nr;
	double l;				// distance on node line (cumulative sum of previous segments)
	double magDirCngSum;	// cumulative sum of magDirCng
};
typedef CArray<SNodeSums, SNodeSums&> CNodeSumsArray;


/////////////////////////////////////////////////////////////////////////////



class CPolyCurveFit  
{
public:
	CPolyCurveFit();
	virtual ~CPolyCurveFit();


	void SetPath(CPathDoc* pPath) { m_pPath = pPath; }
	void FitCubicsToCurve(SCurveInfo& ci);


protected:
	void FitOneCubicToCurve(SCurveInfo& ci);	// not used!!
	void FindCubics(CCurveFitList& curveFitList, SCurveInfo& ci);


// Attributes
protected:

	CPathDoc* m_pPath;
	SCurveFitThresholds m_Threshold;
	CPolySegDblFit m_DblPolyFit;


};

#endif // !defined(AFX_POLYCURVEFIT_H__26ED5BCC_12C0_4AE1_81D7_162B38A202C5__INCLUDED_)
