// CubicFit.h: interface for the CCubicFit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUBICFIT_H__81BD00C0_9B1A_11D5_86C3_CE86F91B0C24__INCLUDED_)
#define AFX_CUBICFIT_H__81BD00C0_9B1A_11D5_86C3_CE86F91B0C24__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000





struct SfPoint
{
	double x, y;
};

struct SPointData : SfPoint
{
//	stores data for each point when curve fitting
	int n;
	double yPoly, yError, yErrorSq;

	void Zero() { n = 0; yPoly = yError = yErrorSq = 0; }
};

struct SPointPowerData : SPointData		// base class for testing
{
//	stores data sums for each point when curve fitting
	double Xp[7];
	double YXp[4];
	double Y2;		// used to calc residual
	int num;			// same as Xp[0] but stored as int!

	void Zero();
	void SumUpPoint(const SPointPowerData& prevSum, SfPoint& pt);
};

class CPointSumData
{
public:
	CPointSumData();
	int AddPoint(SfPoint& pt);
	int GetSumsFor(int iInit, int iFinal, SPointPowerData& sumPoints);

public:	// for test   protected:
	SPointPowerData m_SumPoints[1000];
	int m_iStart, m_iEnd, m_iLength, m_iBufferSize;
	int m_iStartRef;
};

struct SPolySegInfo
{
	double coef[4];		// coefficents
	double derFinal[3];	// value and derivatives and final x

	bool bPolySet;
	int nNumPoints;
	double Sr, Sabs;	// Sum(err^2), Sum(abs(err)) - depend on poly
	int nSumRefInit, nSumRefFinal;
	double Xinit, Xfinal;

// Functions
	SPolySegInfo();
	void Reset();
	double ValueAt(double x);
	void AddPoint(SPointData& pt);
	int NumPoints() { return nNumPoints; }
	void SetPolyFrom(SPolySegInfo& src);
	void CalcPointError(SPointData& pt);
};

bool LUFullSymSolve(int n, const double* vectA, double x[], const double* b);
//	Solves for x where: Ax = b   where A[j,k] = vectA[j+k]

/////////////////////////////////////////

class CCubicFit  
{
public:
	CCubicFit();
	virtual ~CCubicFit();

	void Init();
//	int NextPoint(SMotionState& ms);
	int NextPoint(double x, double y);

protected:
// Data
	SPointData m_CurrPt;
	CPointSumData m_PointSumData;
	int m_nFitStatus;		// 0- no poly yet, 1- a poly established

	SPolySegInfo m_PrevPoly;
	SPolySegInfo m_CurrPoly;
	SPolySegInfo m_ExtrapoPoly;

	double m_TolY;
	double m_TolFirstSr;
	double m_SuggestedInc;
	int m_nReductions;


protected:
// Functions
	void InitFromFirstPoint();
	int TryPoint();
	void AddFitPoint();
	void FitPolyToPoints(SPolySegInfo& poly);
	void FitPolyToPointsContPV(SPolySegInfo& poly);
	void FitDblPolyToPointsContPV(SPolySegInfo& poly);
	double GetSumResidual(SPolySegInfo& poly);
	void GetPolyErrors(SPolySegInfo& poly);

};

#endif // !defined(AFX_CUBICFIT_H__81BD00C0_9B1A_11D5_86C3_CE86F91B0C24__INCLUDED_)
