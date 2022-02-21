// PosConverter.h: interface for the CPosConverter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POSCONVERTER_H__D6586A80_D0E0_11D8_86C3_0008A15E291C__INCLUDED_)
#define AFX_POSCONVERTER_H__D6586A80_D0E0_11D8_86C3_0008A15E291C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "Matrix.h"
#include <fstream.h>


struct SSolveData
{
	CMatrix vtXLocs;
	CMatrix vtYLocs;
	CMatrix vtZLocs;
	CMatrix vtXLocsNorm;
	CMatrix vtYLocsNorm;
	CMatrix vtZLocsNorm;
	CMatrix mxXLocsSpline;		// only for checking
	CMatrix mxYLocsSplineT;
	CMatrix mxZLocsPowerT;
	CMatrix mxYZLocsSpline;		// only for checking

	CMatrix mxXLocsSplineInv;	// required
	CMatrix mxYLocsSplineInv;
	CMatrix mxZLocsPowerInv;
	CMatrix mxYZLocsSplineInv;	// required

	CMatrix mxXCoef;				// required
	CMatrix mxCalcCorrDiff;		// only for checking
};

struct SCorrectionData
{
	CMatrix vtXLocs;
	CMatrix vtYLocs;
	CMatrix mxVal;
};

class CPosConverter  
{
public:
	CPosConverter();
	virtual ~CPosConverter();

	void UsePosCorrection(int bUse) { m_bUsePosCorrection = (bUse != 0); }
	bool UsingPosCorrection() { return m_bUsePosCorrection; }
	void UsePosCorrection(int bUseX, int bUseY, int bUseZ) { m_vtbUsePosCorrection.Set(bUseX, bUseY, bUseZ); }
	void UsingPosCorrection(int& bUseX, int& bUseY, int& bUseZ) { m_vtbUsePosCorrection.Get(bUseX, bUseY, bUseZ); }

	void SetZAxisHead2Tip(double dist) { m_vtZAxisHead2Tip.Set(0,0,-dist); }
	double GetZAxisHead2Tip() { return -m_vtZAxisHead2Tip.z; }

	void SetZAxisHead2ProbeRetracted(double dist) { m_vtZAxisHead2ProbeRetracted.Set(0,0,-dist); }
	double GetZAxisHead2ProbeRetracted() { return -m_vtZAxisHead2ProbeRetracted.z; }

	void Init();

/*
	Position coord systems are:
	vtPosServo = machine readings
	vtPosTip = vtPosServo + vtTipCorr(vtPosServo, vtZAxisHead2Tip)

	vtPosHead = vtPosTip (don't use PosHead!! - correction is for tip so gets confusing!)
	Tip relative to base - not used by PosConverter = vtPosTip - vtPosBase (z only)
*/

	void GetPosServoFromPosTip(const CVector& vtPosTip, CVector& vtPosServo);
	void GetPosServoFromPosTip(CVector& vtPos);
	void GetPosTipFromPosServo(const CVector& vtPosServo, CVector& vtPosTip);
	void GetPosTipFromPosServo(CVector& vtPos);

	void GetPosProbeTipFromPosServo(const CVector& vtPosServo, double dProbeExt, CVector& vtPosProbeTip);

protected:
	// correction functions
	void GetTipCorrFromPosServo(const CVector& vtPosServo, double zOffset, CVector& vtTipCorr);
	void GetTipCorrFromPosTip(const CVector& vtPosTip, CVector& vtTipCorr);



protected:
	void SetCorrectionCoefficents();





	// attributes
protected:
	// static members for global values
	static CMatrix m_mxCorrCoeffsServo[3];		// array of size(4,8)
	static bool m_bUsePosCorrection;
	static TVector<int> m_vtbUsePosCorrection;

	static CVector m_vtZAxisHead2Tip;			// offset of bit tip from Z Axis Head
	static CVector m_vtZAxisHead2ProbeRetracted;	// offset of contracted probe tip from Z Axis Head


	// non-static members which are modifed while used in a thread
	// matricies used in frequent calculations
	CMatrix m_vtXPower;		// row (1,4)
	CMatrix m_vtYPower;		// row (1,4)
	CMatrix m_vtZPower;		// row (1,2)
	CMatrix m_vtYZPower;		// col (8,1)
	CMatrix m_vtResYZ;		// col (4,1)
	CMatrix m_mxLocalNodes;	// matrix (4,8)

///////////////////////////////////////
	// for new B-Spline method


protected:
	void SetPatchCorrectionCoefficents();

	void SetSizeGrid(int xGrids, int yGrids);
	void SetSizeNodes(int xNodes, int yNodes);
	void SetSizeNodes();
	void SetAtPoint(const CVector2& vtPt, const CVector& vtVal);
	void GetNormalisedPnt(CVector2& vtGridNorm, const CVector2& vtPt);
	void GetNormalisedPnt(CPoint& ptGrid, CVector2& vtGridLoc, const CVector2& vtPt);
	void GetNormalisedLocs(CMatrix& vtLocsNorm, const CMatrix& vtLocs, const CMatrix& vtGridLocs);
	void GetBCoeffsAtLoc(CMatrix& vtBCoeffs, double GridLoc);
	void GetB2CoeffsAtLoc(CMatrix& vtBCoeffs, double GridLoc);
	void GetBCoeffsAtPnt(CMatrix& mxBCoeffs, const CVector2& vtGridLoc);
	void GetBSplineAtLoc(CMatrix& mxBSplineLoc, double locNorm);
	void GetBSplineAtLocs(CMatrix& mxBSplineLocs, const CMatrix& vtLocsNorm);


	void GetLocInverses(SSolveData& sd);
	bool GetCoeffs(CMatrix& mxCoefs, const CMatrix& mxVals, SSolveData& sd);


	void OutputCorrectionPoints(char* szFile, double scale, const CMatrix& vtXLocs, const CMatrix& vtYLocs, const CMatrix& mxZVal);
	void Interpolate(int ax, double scale);
	void InterpolateAtPoints(int ax, double scale);
	void GetTipCorrFromPosServoSpline(const CVector& vtPosServo, double zOffset, CVector& vtTipCorr);

	// used?
	void AddBCoeffEquAtPnt(const CMatrix& mxBCoeffs, const CPoint& ptGrid, const CVector& vtVal);
	
	
	
protected:
	// used to read correction file
	ifstream m_is;
	int m_numLine;
	bool m_bReReadLine;
	#define MAX_CHARS_LINE 256
	#define MAX_VALUES_LINE 32
	char m_szLine[MAX_CHARS_LINE];
	double m_arVal[MAX_VALUES_LINE];
	int m_numVal;
	char* m_strRemark;

	// correction data from file
	#define NUM_AXISCORRSETS 4
	SCorrectionData m_arAxisCorrData[3][NUM_AXISCORRSETS];		// [axis][data set]
	int m_arCountAxisData[3];


	void ReadCorrectionFile(char* szFile);
	char* GetNextLine();
	bool PutLineBack();
	void ReadArrayLocs(SCorrectionData* pCD);
	void ReadPointLocs(SCorrectionData* pCD);


	// used??
	CMatrix m_mxBCoeffs;		// (4,4)
	CMatrix m_mxEqu;			// (points,points) - big
	CMatrix m_vtVal;			// (points,3)
	int m_numEqu;
	//////

	// static members for global values
	static CSize m_szGrid;
	static CSize m_szNodes;
	static int m_szNodesZ;
	static CVector m_vtGridLocMin;
	static CVector m_vtGridLocMax;
	static CMatrix m_vtXGridLocs;
	static CMatrix m_vtYGridLocs;

};


extern CPosConverter g_utPosConvert;		// declare global object for use in user thread
extern CPosConverter g_mtPosConvert;		// declare global object for use in monitor thread


#endif // !defined(AFX_POSCONVERTER_H__D6586A80_D0E0_11D8_86C3_0008A15E291C__INCLUDED_)
