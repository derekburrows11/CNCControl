// PosConverter.cpp: implementation of the CPosConverter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PosConverter.h"

#include "cnccontrol.h"
#include "Settings.h"
#include "StrUtils.h"
#include <float.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CPosConverter g_utPosConvert;		// define global object for use in user thread
CPosConverter g_mtPosConvert;		// define global object for use in monitor thread

// static members
CMatrix CPosConverter::m_mxCorrCoeffsServo[3];
bool CPosConverter::m_bUsePosCorrection;
TVector<int> CPosConverter::m_vtbUsePosCorrection;
CVector CPosConverter::m_vtZAxisHead2Tip;			// offset of bit tip from Z Axis Head
CVector CPosConverter::m_vtZAxisHead2ProbeRetracted;	// offset of contracted probe tip from Z Axis Head

// for new B-Spline method
CSize CPosConverter::m_szGrid;
CSize CPosConverter::m_szNodes;
int CPosConverter::m_szNodesZ;
CVector CPosConverter::m_vtGridLocMin;
CVector CPosConverter::m_vtGridLocMax;
CMatrix CPosConverter::m_vtXGridLocs;
CMatrix CPosConverter::m_vtYGridLocs;



int GetValues(char*& pCh, double* arVal, int maxVals)
{
	char* pChEnd;
	int numVal = 0;
	while (*pCh != 0)
	{
		arVal[numVal] = strtod(pCh, &pChEnd);
		if (pChEnd == pCh)		// no value
			break;
		pCh = pChEnd;				// advance past value
		if (++numVal >= maxVals)
			break;
	}
	skipWS(pCh);
	return numVal;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPosConverter::CPosConverter()
{
	m_bUsePosCorrection = false;
	m_vtbUsePosCorrection = false;
	m_vtZAxisHead2Tip = 0;
	m_vtZAxisHead2ProbeRetracted = 0;
}

CPosConverter::~CPosConverter()
{
}

void CPosConverter::Init()
{
	// load dimension values
	SMachineDimensions& md = g_Settings.MachDimensions;
	ASSERT(md.Valid());
	SetZAxisHead2Tip(md.GetTip2ZAxisHeadDist());
	SProbeDimensions& pd = g_Settings.ProbeDimensions;
	ASSERT(pd.Valid());
	SetZAxisHead2ProbeRetracted(pd.m_AxisHead2TipRetracted);

	SetCorrectionCoefficents();
	m_bUsePosCorrection = true;
	m_vtbUsePosCorrection = true;

	//Interpolate(0, 500);
	//Interpolate(1, 500);
	//Interpolate(2, -1000);

//	InterpolateAtPoints(2, 1);


}

void CPosConverter::OutputCorrectionPoints(char* szFile, double scale, const CMatrix& vtXLocs, const CMatrix& vtYLocs, const CMatrix& mxZVal)
{
	ofstream os;
	os.open(szFile);
	os.setf(ios::fixed);
	os.precision(3);
	os.unsetf(ios::showpoint);		// no different!
/*
	// group y values
	for (int iy = 0; iy < vtYLocs.Length(); iy++)
	{
		for (int ix = 0; ix < vtXLocs.Length(); ix++)
			os << vtXLocs[ix] << "  " << vtYLocs[iy] << "  " << scale * mxZVal.elem(iy, ix) << endl;
		os << endl;
	}
*/
	// group x values
	for (int ix = 0; ix < vtXLocs.Length(); ix++)
	{
		for (int iy = 0; iy < vtYLocs.Length(); iy++)
			os << vtXLocs[ix] << "  " << vtYLocs[iy] << "  " << scale * mxZVal.elem(iy, ix) << endl;
		os << endl;
	}
	os.close();
}

void CPosConverter::Interpolate(int ax, double scale)
{
	ofstream os;
	char szName[] = "Config\\CorrInterpolate#.txt";
	char* pAxChar = strrchr(szName, '#');
	ASSERT(pAxChar);
	*pAxChar = char('X' + ax);
	TRACE1("Writing %s\n", szName);	
	LOGEVENT1("Writing %s", szName);	

	os.open(szName);
	os.setf(ios::fixed);
	os.precision(3);

	//double zOffset = m_vtZAxisHead2Tip.z;
	double zOffset = -232.5;			// pos at calibration
	double zServo = 62;
	CVector2 ptMin(0, 0);
	CVector2 ptMax(1850, 850);
	CVector2 vtStep(25, 25);

	CVector vtPosServo;
	CVector vtTipCorr;
	double x, y;
	for (y = ptMin.y; y <= ptMax.y; y += vtStep.y)
	{
		for (x = ptMin.x; x <= ptMax.x; x += vtStep.x)
		{
			vtPosServo.Set(x, y, zServo);
			GetTipCorrFromPosServoSpline(vtPosServo, zOffset, vtTipCorr);
			os << x << "  " << y << "  " << scale * vtTipCorr[ax] << endl;
		}
		os << endl;
	}
	os << endl;

	for (x = ptMin.x; x <= ptMax.x; x += vtStep.x)
	{
		for (y = ptMin.y; y <= ptMax.y; y += vtStep.y)
		{
			vtPosServo.Set(x, y, zServo);
			GetTipCorrFromPosServoSpline(vtPosServo, zOffset, vtTipCorr);
			os << x << "  " << y << "  " << scale * vtTipCorr[ax] << endl;
		}
		os << endl;
	}
	os.close();
}

void CPosConverter::InterpolateAtPoints(int ax, double scale)
{
	ofstream os;
	char szName[] = "Config\\CorrInterpolate#.txt";
	char* pAxChar = strrchr(szName, '#');
	ASSERT(pAxChar);
	*pAxChar = char('X' + ax);
	TRACE1("Writing %s\n", szName);	
	LOGEVENT1("Writing %s", szName);	

	os.open(szName);

	// make sure m_bUsePosCorrection is enabled on axis
	bool bUseCorr = m_bUsePosCorrection;
	m_bUsePosCorrection = true;
	int bUseAxCorr = m_vtbUsePosCorrection[ax];
	m_vtbUsePosCorrection[ax] = true;

	//double zOffset = m_vtZAxisHead2Tip.z;
	double zOffset = -232.5;			// pos at calibration
	double zServo = 62;

	// set locations
	CMatrix vtXLocs(15);
	CMatrix vtYLocs(9);
//	CMatrix vtYLocs(5);
	int idx = 0;
	vtXLocs[idx++] = 0;
	vtXLocs[idx++] = 0.5;
	for (int i = 1; i < vtXLocs.Length()-3; i++)
		vtXLocs[idx++] = i;
	vtXLocs[idx++] = i-0.5;
	vtXLocs[idx++] = i;
	for (i = 0; i < vtXLocs.Length(); i++)
		vtXLocs[i] = 30 + vtXLocs[i] * 148;
//		vtXLocs[i] = 5 + vtXLocs[i] * 152;
	for (i = 0; i < vtYLocs.Length(); i++)
		vtYLocs[i] = 25 + i * 100;
//		vtYLocs[i] = 1 + i * 212;


	// write locations
	os.unsetf(ios::fixed);
	//os.precision(2);
	os << "xLocs = [";
	for (i = 0; i < vtXLocs.Length(); i++)
		os << vtXLocs[i] << "  ";
	os << "]\n";
	os << "yLocs = [";
	for (i = 0; i < vtYLocs.Length(); i++)
		os << vtYLocs[i] << "  ";
	os << "]\n";

	// write values
	os.setf(ios::fixed);
	os.precision(2);
	CVector vtPosServo;
	CVector vtTipCorr;
	// group y values
	for (int iy = 0; iy < vtYLocs.Length(); iy++)
	{
		for (int ix = 0; ix < vtXLocs.Length(); ix++)
		{
			vtPosServo.Set(vtXLocs[ix], vtYLocs[iy], zServo);
			GetTipCorrFromPosServo(vtPosServo, zOffset, vtTipCorr);
			os.width(6);
			os << scale * vtTipCorr[ax] << " ";
		}
		os << endl;
	}

	os.close();

	// reset values
	m_bUsePosCorrection = bUseCorr;
	m_vtbUsePosCorrection[ax] = bUseAxCorr;
}

//////////////////////////////

void CPosConverter::SetPatchCorrectionCoefficents()
{
/*
	position corrections are Head position relative to Servo postion
	at the defined Servo positions
*/
/*

	Num B-spline nodes = (Xgrids+3) * (Ygrids+3)
	Set points to match B-spline nodes required

	*-----*-----*-----*
	|                 |
	|                 |
	*     *     *     *
	|                 |
	|                 |
	*     *     *     *
	|                 |
	|                 |
	*-----*-----*-----*



	*--------*--------*-----------------*--------*--------*
	|                 |                 |                 |
	|                 |                 |                 |
	|                 |                 |                 |
	*        *        *                 *        *        *
	|                 |                 |                 |
	|                 |                 |                 |
	|                 |                 |                 |
	*--------*--------*-----------------*--------*--------*
	|                 |                 |                 |
	|                 |                 |                 |
	|                 |                 |                 |
	*        *        *                 *        *        *
	|                 |                 |                 |
	|                 |                 |                 |
	|                 |                 |                 |
	*--------*--------*-----------------*--------*--------*



	To calc value:
	vt(Xvals B-spline) * [4x4 B-spl] * vt(Yvals B-spline)'
                        [ nodes   ]

	vtXPower3 * [4x4 B-spl] * [4x4 B-spl] * [4x4 B-spl]' * vtYPower3'
	            [ coeffs  ]   [ nodes   ]   [ coeffs  ]

  
	for X^3, Y^3, Z^1

	vt(Xvals B-spline) * [4x8 B-spl] * [vt(Yvals B-spline) Z*vt(Yvals B-spline)]'
                        [ nodes   ]

	vtXPower3 * [4x4 B-spl] * [4x8 B-spl] * [4x4 B-spl]' * vtYPower3'
	            [ coeffs  ]   [ nodes   ]   [ coeffs  ]

	to find nodes:

	[x1 B-spline] * [4x8 B-spl] * [y1 B-spline) Z*vt(Yvals B-spline)]'
	[x. B-spline]   [ nodes   ]

*/

	// zCalc = zServo - [Bit Tip to Z Axis Head]
	double dTip2ZAxisHeadCal = 143.0 + 56.0 + 33.5;	// Tip to Axis Head when caibrated: Axis head to mount to chuck to tip
	double zCenter = -dTip2ZAxisHeadCal;
	double arZLocs[2] = { 62, 380 };	// router tip at base = 62
	// with z servo at 0
	// base to X trolley = 460, base to Y trolley = 530
	// base to X screw = 500, base to Y screw = 650



	m_vtGridLocMin.Set(0, 0, 0);
	m_vtGridLocMax.Set(1836, 850, 0);

	SSolveData sd;
	CMatrix& vtXLocs = sd.vtXLocs;
	CMatrix& vtYLocs = sd.vtYLocs;
	CMatrix& vtZLocs = sd.vtZLocs;
	CMatrix mxCorr;
	CMatrix mxCoeffs;

	// Read Corrections
	////////////////////////

	ReadCorrectionFile("Config\\CorrPnts.txt");
//	SetSizeNodes();

	vtZLocs.SetSize(2);
	vtZLocs.SetFromArray(arZLocs);
	vtZLocs += zCenter;


////////////////////
	
	// X Correction
	////////////////////////
	int iAxis = 0;
	int iDat = 0;
	ASSERT(iDat < m_arCountAxisData[iAxis]);
	vtXLocs = m_arAxisCorrData[iAxis][iDat].vtXLocs;
	vtYLocs = m_arAxisCorrData[iAxis][iDat].vtYLocs;
	const CMatrix& mxXVal = m_arAxisCorrData[iAxis][iDat].mxVal;

	int xLocs = vtXLocs.Length();
	int yLocs = vtYLocs.Length();
	SetSizeNodes(xLocs, yLocs);
	mxCorr.SetSize(xLocs, yLocs * vtZLocs.Length());

	CMatrix mxXValT;
	mxXVal.Transpose(mxXValT);
	mxXValT.CopyToDestLoc(mxCorr, 0, 0);
	// dX = f(x,y) * (ztip - z0) / (ztipCal - z0)
	// ztip = zservo - TipToHead
	double z0 = 500;				// z pos head at X screw thread - where dX = 0
	mxXValT *= (vtZLocs[1] - z0) / (62-232.5 - z0);		// scale at z1
	mxXValT.CopyToDestLoc(mxCorr, 0, yLocs);


	double dXondYSkew = -1.7e-3;
	CMatrix mxXCorrSkew;
	mxXCorrSkew.SetSize(mxCorr);
//	double y0 = 0.5 * (m_vtGridLocMax.y + m_vtGridLocMin.y);		// origin half way
	double y0 = m_vtGridLocMin.y;			// origin at 0 for skew direction so x=0 is possible
	for (int iy = 0; iy < yLocs; iy++)
	{
		double val = dXondYSkew * (vtYLocs[iy] - y0);
		mxXCorrSkew.SetColumn(iy, val);
		mxXCorrSkew.SetColumn(iy+yLocs, val);
	}
	mxCorr += mxXCorrSkew;

	GetLocInverses(sd);
	if (!GetCoeffs(m_mxCorrCoeffsServo[iAxis], mxCorr, sd))
		LOGERROR("Error calculating X pos correction");





	// Y Correction
	////////////////////////
	iAxis = 1;
	// use same Locs as X
	xLocs = vtXLocs.Length();
	yLocs = vtYLocs.Length();
	mxCorr.SetSize(xLocs, yLocs * vtZLocs.Length());
	
	// dY = dYonYZ * (y - y0) * (ztip - z0)
	// ztip = zservo - TipToHead
	z0 = 650;				// z pos head at Y screw thread - where dY = 0
	//double dYonYZ = -1.08 / (-424 * (62-232.5 - z0));
	double dYonYZ = -0.93 / (-424 * (62-232.5 - z0));
	CMatrix mxYCorrDiverge;
	mxYCorrDiverge.SetSize(mxCorr);
	y0 = 0.5 * (m_vtGridLocMax.y + m_vtGridLocMin.y);		// origin half way
	for (iy = 0; iy < yLocs; iy++)
	{
		double dYonZ = dYonYZ * (vtYLocs[iy] - y0);
		mxYCorrDiverge.SetColumn(iy,       dYonZ * (vtZLocs[0] - z0));
		mxYCorrDiverge.SetColumn(iy+yLocs, dYonZ * (vtZLocs[1] - z0));
	}
	mxCorr = 0;
	mxCorr += mxYCorrDiverge;

	//GetLocInverses(sd);		// Locs haven't changed so don't need to recalc
	if (!GetCoeffs(m_mxCorrCoeffsServo[iAxis], mxCorr, sd))
		LOGERROR("Error calculating Y pos correction");



	// Z Correction
	////////////////////////
	iAxis = 2;
	// sum all axis correction data
	for (iDat = 0; iDat < m_arCountAxisData[iAxis]; iDat++)
	{
		vtXLocs = m_arAxisCorrData[iAxis][iDat].vtXLocs;
		vtYLocs = m_arAxisCorrData[iAxis][iDat].vtYLocs;
		const CMatrix& mxZVal = m_arAxisCorrData[iAxis][iDat].mxVal;

		xLocs = vtXLocs.Length();
		yLocs = vtYLocs.Length();
		SetSizeNodes(xLocs, yLocs);
		mxCorr.SetSize(xLocs, yLocs * vtZLocs.Length());

		CMatrix mxZValT;
		mxZVal.Transpose(mxZValT);
		mxZValT.CopyToDestLoc(mxCorr, 0, 0);
		mxZValT.CopyToDestLoc(mxCorr, 0, yLocs);

		GetLocInverses(sd);
		if (!GetCoeffs(mxCoeffs, mxCorr, sd))
			LOGERROR("Error calculating Z pos correction");
		if (iDat == 0)
			m_mxCorrCoeffsServo[iAxis] = mxCoeffs;
		else
			m_mxCorrCoeffsServo[iAxis] += mxCoeffs;

		//OutputCorrectionPoints("Config\\CorrPntsOut.txt", -1000, vtXLocs, vtYLocs, mxZVal);
		//afxDump << "mxZVal: " << mxZVal;
	}

//	InterpolateAtPoints(0, 1);
//	InterpolateAtPoints(1, 1);
//	InterpolateAtPoints(2, 1);

}

void CPosConverter::GetLocInverses(SSolveData& sd)
{
	GetNormalisedLocs(sd.vtXLocsNorm, sd.vtXLocs, m_vtXGridLocs);
	GetNormalisedLocs(sd.vtYLocsNorm, sd.vtYLocs, m_vtYGridLocs);

	int xLocs = sd.vtXLocs.Length();
	int yLocs = sd.vtYLocs.Length();
	int zLocs = sd.vtZLocs.Length();

	// calc using 3 seperate inverses!
	sd.mxXLocsSpline.SetSize(xLocs, xLocs);
	sd.mxYLocsSplineT.SetSize(yLocs, yLocs);
	sd.mxZLocsPowerT.SetSize(zLocs, zLocs);
	GetBSplineAtLocs(sd.mxXLocsSpline, sd.vtXLocsNorm);
	GetBSplineAtLocs(sd.mxYLocsSplineT, sd.vtYLocsNorm);
	sd.mxZLocsPowerT.Power1Matrix(sd.vtZLocs);
	sd.mxYLocsSplineT.Transpose();
	sd.mxZLocsPowerT.Transpose();

	sd.mxXLocsSpline.LUInverse(sd.mxXLocsSplineInv);	// (xLocs,xLocs)
	sd.mxYLocsSplineT.LUInverse(sd.mxYLocsSplineInv);	// (yLocs,yLocs)
	sd.mxZLocsPowerT.LUInverse(sd.mxZLocsPowerInv);		// (zLocs,zLocs)

	sd.mxYZLocsSplineInv.BlockScale(sd.mxYLocsSplineInv, sd.mxZLocsPowerInv);	// (yzLocs,yzLocs)

	// mxYZLocsSpline only used to check
	sd.mxYZLocsSpline.BlockScale(sd.mxYLocsSplineT, sd.mxZLocsPowerT);
}

bool CPosConverter::GetCoeffs(CMatrix& mxCoefs, const CMatrix& mxVals, SSolveData& sd)
{
	// check correct sizes!
	ASSERT(mxVals.h == sd.vtXLocs.Length());
	ASSERT(mxVals.w == sd.vtYLocs.Length() * sd.vtZLocs.Length());

	sd.mxXCoef.Prod(sd.mxXLocsSplineInv, mxVals);
	mxCoefs.Prod(sd.mxXCoef, sd.mxYZLocsSplineInv);		// (xLocs, yzLocs)
	// to check:
	sd.mxCalcCorrDiff = sd.mxXLocsSpline * mxCoefs * sd.mxYZLocsSpline - mxVals;
	return sd.mxCalcCorrDiff.Mag() <= 1e-8;
}

enum
{
	SECT_GENERAL = 0,
	SECT_DXHEADER,
	SECT_DXDATA,
	SECT_DYHEADER,
	SECT_DYDATA,
	SECT_DZHEADER,
	SECT_DZDATA,

	SECT_DXCORRECT,
	SECT_DYCORRECT,
	SECT_DZCORRECT,
};


void CPosConverter::ReadCorrectionFile(char* szFile)
{
	m_is.open(szFile, ios::nocreate);
	if (m_is.fail())
	{
		LOGERROR1("Couldn't open correction file %s", szFile);
		return;
	}
	m_strRemark = "//";

	for (int i = 0; i < 3; i++)
		m_arCountAxisData[i] = 0;
	SCorrectionData* pCorrData = NULL;
	int axCorr = -1;

	int iSection = SECT_GENERAL;
	m_numLine = 0;
	while (!m_is.eof())
	{
		char* pCh = GetNextLine();
		bool bSyntaxError = false;

		if (m_numVal == 0)
		{
			extractWS(pCh);
			if (*pCh == 0)			// blank line
				;
			else if (striextract(pCh, m_strRemark))		// remark
				;
			else if (striextract(pCh, "dX Correction"))
			{
				axCorr = 0;
				pCorrData = m_arAxisCorrData[axCorr];
				iSection = SECT_DXCORRECT;
			}
			else if (striextract(pCh, "dY Correction"))
			{
				axCorr = 1;
				pCorrData = m_arAxisCorrData[axCorr];
				iSection = SECT_DYCORRECT;
			}
			else if (striextract(pCh, "dZ Correction"))
			{
				axCorr = 2;
				pCorrData = m_arAxisCorrData[axCorr];
				iSection = SECT_DZCORRECT;
			}
			else if (striextract(pCh, "Array Locations"))
			{
				ASSERT(axCorr != -1);
				int idxSet = m_arCountAxisData[axCorr]++;
				ASSERT(idxSet < NUM_AXISCORRSETS);
				ReadArrayLocs(&pCorrData[idxSet]);
			}
			else if (striextract(pCh, "Point Locations"))
			{
				ASSERT(axCorr != -1);
				int idxSet = m_arCountAxisData[axCorr]++;
				ASSERT(idxSet < NUM_AXISCORRSETS);
				ReadPointLocs(&pCorrData[idxSet]);
			}
			else
				bSyntaxError = true;
		}
		else
			bSyntaxError = true;


		if (bSyntaxError)
			LOGERROR1("Syntax error in correction file line %i", m_numLine);
	}	// while (!is.eof())

	m_is.close();
}



char* CPosConverter::GetNextLine()
{
	if (!m_bReReadLine || m_numLine == 0)
	{
		m_is.getline(m_szLine, MAX_CHARS_LINE);
		m_numLine++;
	}
	m_bReReadLine = false;
	char* pCh = m_szLine;
	m_numVal = GetValues(pCh, m_arVal, MAX_VALUES_LINE);
	return pCh;
}

bool CPosConverter::PutLineBack()
{
	if (m_bReReadLine)
	{
		ASSERT(0);
		return false;
	}
	m_bReReadLine = true;
	return true;
}

void CPosConverter::ReadArrayLocs(SCorrectionData* pCD)
{
	ASSERT(pCD != NULL);
	CMatrix& vtXLocs = pCD->vtXLocs;
	CMatrix& vtYLocs = pCD->vtYLocs;
	CMatrix& mxVal = pCD->mxVal;

	double dScale = 1;
	double dOffset = 0;
	int iCountYLocs = 0;
	int numXLocs = 0;
	int numYLocs = 0;

	while (!m_is.eof())
	{
		char* pCh = GetNextLine();
		bool bSyntaxError = false;
		bool bBlankLine = false;

		if (m_numVal == 0)
		{
			if (m_numVal == 0)
			{
				extractWS(pCh);

				if (*pCh == 0)
					bBlankLine = true;
				else if (striextract(pCh, m_strRemark))		// remark, ignore
					;
				else if (striextract(pCh, "scale"))
				{
					if (GetValues(pCh, m_arVal, MAX_VALUES_LINE) == 1)
						dScale = m_arVal[0];
					else
						LOGERROR1("Expected value in line: %i", m_numLine);
				}
				else if (striextract(pCh, "offset"))
				{
					if (GetValues(pCh, m_arVal, MAX_VALUES_LINE) == 1)
						dOffset = m_arVal[0];
					else
						LOGERROR1("Expected value in line %i", m_numLine);
				}
				else if (striextract(pCh, "xLocs"))
				{
					extractWS(pCh);
					extractCh(pCh, '=');
					extractWS(pCh);
					extractCh(pCh, '[');
					numXLocs = GetValues(pCh, m_arVal, MAX_VALUES_LINE);
					extractCh(pCh, ']');
					vtXLocs.SetSize(numXLocs);
					vtXLocs.SetFromArray(m_arVal);
				}
				else if (striextract(pCh, "yLocs"))
				{
					extractWS(pCh);
					extractCh(pCh, '=');
					extractWS(pCh);
					extractCh(pCh, '[');
					numYLocs = GetValues(pCh, m_arVal, MAX_VALUES_LINE);
					extractCh(pCh, ']');
					vtYLocs.SetSize(numYLocs);
					vtYLocs.SetFromArray(m_arVal);
				}
				else
					bSyntaxError = true;
			}
			else
				bSyntaxError = true;

			if (!bBlankLine && iCountYLocs != 0)	// check if finished point data
			{
				PutLineBack();		// put line back to be read again!
				break;
			}
		}
		else		// (m_numVal != 0)
		{
			if (numXLocs != 0 && numYLocs != 0)
			{
				if (iCountYLocs == 0)
					mxVal.SetSize(numYLocs, numXLocs);		// set value matrix to appropriate size
				if (m_numVal != numXLocs)
					LOGERROR3("Wrong num xLocs (%i not %i) line: %i", m_numVal, numXLocs, m_numLine);
				else if (iCountYLocs >= numYLocs)
					LOGERROR3("Wrong num yLocs (%i not %i) line: %i", iCountYLocs+1, numYLocs, m_numLine);
				else
					mxVal.SetRow(iCountYLocs++, m_arVal);
			}
			else
				bSyntaxError = true;
		}

		if (bSyntaxError)
			LOGERROR1("Syntax error in correction file line %i", m_numLine);

	}	//	while (!m_is.eof())


	if (iCountYLocs != numYLocs)
		LOGERROR3("Wrong num yLocs (%i not %i) line: %i", iCountYLocs, numYLocs, m_numLine);

	if (dOffset != 0)
		mxVal += dOffset;
	if (dScale != 1)
		mxVal *= dScale;

	// order x min to max
	int idxMax = numXLocs - 1;
	for (int iDest = 0; iDest < idxMax; iDest++)
	{
		int iSrc = iDest;
		double val = vtXLocs[iSrc];
		for (int i = iDest+1; i <= idxMax; i++)
			if (vtXLocs[i] < val)
			{
				iSrc = i;
				val = vtXLocs[iSrc];
			}
		if (iSrc != iDest)
		{
			vtXLocs[iSrc] = vtXLocs[iDest];
			vtXLocs[iDest] = val;
			mxVal.SwapColumns(iDest, iSrc);
		}
	}
	// order y min to max
	idxMax = numYLocs - 1;
	for (iDest = 0; iDest < idxMax; iDest++)
	{
		int iSrc = iDest;
		double val = vtYLocs[iSrc];
		for (int i = iDest+1; i <= idxMax; i++)
			if (vtYLocs[i] < val)
			{
				iSrc = i;
				val = vtYLocs[iSrc];
			}
		if (iSrc != iDest)
		{
			vtYLocs[iSrc] = vtYLocs[iDest];
			vtYLocs[iDest] = val;
			mxVal.SwapRows(iDest, iSrc);
		}
	}
//	afxDump << "vtXLocs: " << vtXLocs;
//	afxDump << "vtYLocs: " << vtYLocs;
//	afxDump << "mxVal: " << mxVal;

}

void CPosConverter::ReadPointLocs(SCorrectionData* pCD)
{
	ASSERT(pCD != NULL);

	CArray<double, double&> xLocArray;
	CArray<double, double&> yLocArray;
	CMatrix mxValFile(8, 32);		// large enough to hold file data!
	double zero = 0;
	mxValFile = 0/zero;				// set to nan
	CVector vtPt;

	double dScale = 1;
	double dOffset = 0;
	int iCountPoints = 0;
	int iCountDoubleUp = 0;
	int iLineDoubleUp;

	while (!m_is.eof())
	{
		char* pCh = GetNextLine();
		bool bSyntaxError = false;
		bool bBlankLine = false;

		if (m_numVal == 3)
		{
			vtPt.SetFromArray(m_arVal);

			// find x & y in list
			int size;
			double val;
			val = vtPt.x;
			size = xLocArray.GetSize();
			for (int ix = 0;; ix++)
			{
				if (ix >= size)
				{
					xLocArray.Add(val);
					break;
				}
				if (fabs(val - xLocArray[ix]) <= 0.01)		// close enough to a previous location!
					break;
				else if (fabs(val - xLocArray[ix]) < 10)	// check not close but no match to previous location
					LOGERROR3("Corr xLoc %f too close to %f line %i", val , xLocArray[ix], m_numLine); 
			}

			val = vtPt.y;
			size = yLocArray.GetSize();
			for (int iy = 0;; iy++)
			{
				if (iy >= size)
				{
					yLocArray.Add(val);
					break;
				}
				if (fabs(val - yLocArray[iy]) <= 0.2)		// close enough to a previous location!
					break;
				else if (fabs(val - yLocArray[iy]) < 10)	// check not close but no match to previous location
					LOGERROR3("Corr yLoc %f too close to %f line %i", val , yLocArray[iy], m_numLine); 
			}

			if (!_finite(mxValFile.elem(iy, ix)))
				mxValFile.elem(iy, ix) = vtPt.z;
			else
			{
				iCountDoubleUp++;
				iLineDoubleUp = m_numLine;
			}
			iCountPoints++;
		}
		else		// (m_numVal != 3)
		{
			if (m_numVal == 0)
			{
				extractWS(pCh);
				if (*pCh == 0)
					bBlankLine = true;
				else if (striextract(pCh, "scale"))
				{
					if (GetValues(pCh, m_arVal, MAX_VALUES_LINE) == 1)
						dScale = m_arVal[0];
					else
						LOGERROR1("Expected value in line: %i", m_numLine);
				}
				else if (striextract(pCh, "offset"))
				{
					if (GetValues(pCh, m_arVal, MAX_VALUES_LINE) == 1)
						dOffset = m_arVal[0];
					else
						LOGERROR1("Expected value in line %i", m_numLine);
				}
				else if (!striextract(pCh, m_strRemark))
					bSyntaxError = true;
			}
			else
				bSyntaxError = true;

			if (!bBlankLine && iCountPoints != 0)	// check if finished point data
			{
				PutLineBack();	
				break;
			}

			if (bSyntaxError)
				LOGERROR1("Syntax error in correction file line %i", m_numLine);
		}

	}	// for (;;)

	int numXLocs = xLocArray.GetSize();
	int numYLocs = yLocArray.GetSize();

	CMatrix& vtXLocs = pCD->vtXLocs;
	CMatrix& vtYLocs = pCD->vtYLocs;
	CMatrix& mxVal = pCD->mxVal;

	vtXLocs.SetSize(numXLocs);
	vtXLocs.SetFromArray(xLocArray.GetData());
	vtYLocs.SetSize(numYLocs);
	vtYLocs.SetFromArray(yLocArray.GetData());

	// set value matrix to appropriate size
	mxVal.SetSize(numYLocs, numXLocs);
	mxVal.CopyFromSrcLoc(mxValFile, 0,0);

	if (dOffset != 0)
		mxVal += dOffset;
	if (dScale != 1)
		mxVal *= dScale;


	if (iCountDoubleUp != 0)
		LOGERROR2("%i double up point in pos corr file line %i", iCountDoubleUp, iLineDoubleUp);
	int numExpected = numXLocs * numYLocs;
	int numUnique = iCountPoints - iCountDoubleUp;
	if (numUnique != numExpected)
		LOGERROR2("Only %i points of %i expected in pos corr file", numUnique, numExpected);


	// order x min to max
	int idxMax = numXLocs - 1;
	for (int iDest = 0; iDest < idxMax; iDest++)
	{
		int iSrc = iDest;
		double val = vtXLocs[iSrc];
		for (int i = iDest+1; i <= idxMax; i++)
			if (vtXLocs[i] < val)
			{
				iSrc = i;
				val = vtXLocs[iSrc];
			}
		if (iSrc != iDest)
		{
			vtXLocs[iSrc] = vtXLocs[iDest];
			vtXLocs[iDest] = val;
			mxVal.SwapColumns(iDest, iSrc);
		}
	}
	// order y min to max
	idxMax = numYLocs - 1;
	for (iDest = 0; iDest < idxMax; iDest++)
	{
		int iSrc = iDest;
		double val = vtYLocs[iSrc];
		for (int i = iDest+1; i <= idxMax; i++)
			if (vtYLocs[i] < val)
			{
				iSrc = i;
				val = vtYLocs[iSrc];
			}
		if (iSrc != iDest)
		{
			vtYLocs[iSrc] = vtYLocs[iDest];
			vtYLocs[iDest] = val;
			mxVal.SwapRows(iDest, iSrc);
		}
	}

//	afxDump << "vtXLocs: " << vtXLocs;
//	afxDump << "vtYLocs: " << vtYLocs;
//	afxDump << "mxVal: " << mxVal;

}

void CPosConverter::SetSizeGrid(int xGrids, int yGrids)
{
	m_szGrid = CSize(xGrids, yGrids);
	m_szNodes = m_szGrid + CSize(3,3);

	//	Set Grid Locations
	m_vtXGridLocs.SetSize(m_szGrid.cx + 1);
	m_vtYGridLocs.SetSize(m_szGrid.cy + 1);
	int span = m_vtXGridLocs.Length() - 1;
	for (int i = 0; i <= span; i++)
		m_vtXGridLocs[i] = (m_vtGridLocMin.x * (span-i) + m_vtGridLocMax.x * i) / span;
	span = m_vtYGridLocs.Length() - 1;
	for (i = 0; i <= span; i++)
		m_vtYGridLocs[i] = (m_vtGridLocMin.y * (span-i) + m_vtGridLocMax.y * i) / span;

	m_numEqu = 0;
}

void CPosConverter::SetSizeNodes(int xNodes, int yNodes)
{
	m_szNodes = CSize(xNodes, yNodes);
	m_szNodesZ = 2;
	m_szGrid = m_szNodes - CSize(3,3);
	SetSizeGrid(m_szGrid.cx, m_szGrid.cy);
}
void CPosConverter::SetSizeNodes()
{
	// find largest set of correction point data
	CSize szNodesMax(0,0);
	for (int ax = 0; ax < 3; ax++)
		for (int i = 0; i < m_arCountAxisData[ax]; i++)
		{
			szNodesMax.cx = max(szNodesMax.cx, m_arAxisCorrData[ax][i].vtXLocs.Length());
			szNodesMax.cy = max(szNodesMax.cy, m_arAxisCorrData[ax][i].vtYLocs.Length());
		}

	m_szNodes = szNodesMax;
	m_szNodesZ = 2;
	m_szGrid = m_szNodes - CSize(3,3);
	SetSizeGrid(m_szGrid.cx, m_szGrid.cy);
}

void CPosConverter::SetAtPoint(const CVector2& vtPt, const CVector& vtVal)
{
	CPoint ptGrid;
	CVector2 vtGridLoc;
	GetNormalisedPnt(ptGrid, vtGridLoc, vtPt);
	GetBCoeffsAtPnt(m_mxBCoeffs, vtGridLoc);
	AddBCoeffEquAtPnt(m_mxBCoeffs, ptGrid, vtVal);
}

void CPosConverter::GetNormalisedPnt(CVector2& vtGridNorm, const CVector2& vtPt)
{
	int len;
	len = m_vtXGridLocs.Length() - 1;
	for (int s = 1; s < len; s++)
		if (vtPt.x < m_vtXGridLocs[s])
			break;
	s--;

	len = m_vtYGridLocs.Length() - 1;
	for (int t = 1; t < len; t++)
		if (vtPt.y < m_vtYGridLocs[t])
			break;
	t--;

	vtGridNorm.x = s + (vtPt.x - m_vtXGridLocs[s]) / (m_vtXGridLocs[s+1] - m_vtXGridLocs[s]);
	vtGridNorm.y = t + (vtPt.y - m_vtYGridLocs[t]) / (m_vtYGridLocs[t+1] - m_vtYGridLocs[t]);
}
void CPosConverter::GetNormalisedPnt(CPoint& ptGrid, CVector2& vtGridLoc, const CVector2& vtPt)
{
	int len;
	len = m_vtXGridLocs.Length() - 1;
	for (int s = 1; s < len; s++)
		if (vtPt.x < m_vtXGridLocs[s])
			break;
	s--;

	len = m_vtYGridLocs.Length() - 1;
	for (int t = 1; t < len; t++)
		if (vtPt.y < m_vtYGridLocs[t])
			break;
	t--;

	ptGrid.x = s;		// will give ptGrid in grid range but vtGridLoc outside [0 1] if point is outside grid
	ptGrid.y = t;
	vtGridLoc.x = (vtPt.x - m_vtXGridLocs[s]) / (m_vtXGridLocs[s+1] - m_vtXGridLocs[s]);
	vtGridLoc.y = (vtPt.y - m_vtYGridLocs[t]) / (m_vtYGridLocs[t+1] - m_vtYGridLocs[t]);
}

void CPosConverter::GetNormalisedLocs(CMatrix& vtLocsNorm, const CMatrix& vtLocs, const CMatrix& vtGridLocs)
{
	vtLocsNorm.SetSize(vtLocs);
	int maxIdxGrid = vtGridLocs.Length() - 1;
	int lenLocs = vtLocs.Length();
	for (int i = 0; i < lenLocs; i++)
	{
		double pt = vtLocs[i];
		for (int s = 1; s < maxIdxGrid; s++)
			if (pt < vtGridLocs[s])
				break;
		s--;
		vtLocsNorm[i] = s + (pt - vtGridLocs[s]) / (vtGridLocs[s+1] - vtGridLocs[s]);
	}
}


void CPosConverter::GetBCoeffsAtLoc(CMatrix& vtBCoeffs, double gridLoc)
{
//	GetB2CoeffsAtLoc(vtBCoeffs, gridLoc);
//	return;
/*
	for x^3 B-Spline
	equations for each section:
	t = [-2, -1]			 1/6.t^3 + 1.t^2 + 2.t + 4/3
	t = [-1,  0]			-1/2.t^3 - 1.t^2 + 0.t + 2/3
	t = [ 0,  1]			 1/2.t^3 - 1.t^2 + 0.t + 2/3
	t = [ 1,  2]			-1/6.t^3 + 1.t^2 - 2.t + 4/3

	realigned t to [0 1]
	t = [-2, -1]	t+=2	 1/6.t^3 +   0.t^2 +   0.t + 0        1/6.t^3 + (1-6/6).t^2 + (2-4+12/6).t + 4/3 -4+4-4/3
	t = [-1,  0]	t+=1	-1/2.t^3 + 1/2.t^2 + 1/2.t + 1/6     -1/2.t^3 + (-1+3/2).t^2 + (2-3/2).t + 2/3 -1+1/2
	t = [ 0,  1]	t+=0	 1/2.t^3 -   1.t^2 +   0.t + 2/3
	t = [ 1,  2]	t-=1	-1/6.t^3 + 1/2.t^2 - 1/2.t + 1/6
*/
	double s = gridLoc;
	vtBCoeffs.SetSize(1,4);		// row vector
	// s direction is reversed for value relative to node
	if (s == 0)
	{
		vtBCoeffs[3] = 0;
		vtBCoeffs[2] = vtBCoeffs[0] = 1.0/6;
		vtBCoeffs[1] = 2.0/3;
	}
	else if (s == 0.5)
	{
		vtBCoeffs[3] = vtBCoeffs[0] =  1.0/48;
		vtBCoeffs[2] = vtBCoeffs[1] = 23.0/48;
	}
	else
	{
		double s1on2 = 0.5*s;
		double s2on2 = s1on2*s;
		double s3on2 = s2on2*s;
		vtBCoeffs[3] =  s3on2/3;									// [ 1,  2]
		vtBCoeffs[2] = -s3on2   +   s2on2 + s1on2 + 1.0/6;	// [ 0,  1]
		vtBCoeffs[1] =  s3on2   - 2*s2on2         + 2.0/3;	// [-1,  0]
		vtBCoeffs[0] = -s3on2/3 +   s2on2 - s1on2 + 1.0/6;	// [-2, -1]
	}
}

void CPosConverter::GetB2CoeffsAtLoc(CMatrix& vtBCoeffs, double gridLoc)
{
/*
	for x^2 B-Spline
	equations for each section:
	t = [-2, -1]			 1/4.t^2 +   1.t + 1
	t = [-1,  0]			-1/4.t^2 +   0.t + 1/2
	t = [ 0,  1]			-1/4.t^2 +   0.t + 1/2
	t = [ 1,  2]			 1/4.t^2 -   1.t + 1

	realigned t to [0 1]
	t = [-2, -1]	t+=2	 1/4.t^2 +   0.t + 0
	t = [-1,  0]	t+=1	-1/4.t^2 + 1/2.t + 1/4
	t = [ 0,  1]	t+=0	-1/4.t^2 +   0.t + 1/2
	t = [ 1,  2]	t-=1	 1/4.t^2 - 1/2.t + 1/4
*/
	double s = gridLoc;
	vtBCoeffs.SetSize(1,4);		// row vector
	// s direction is reversed for value relative to node
	if (s == 0)
	{
		vtBCoeffs[3] = 0;
		vtBCoeffs[2] = vtBCoeffs[0] = 1.0/4;
		vtBCoeffs[1] = 1.0/2;
	}
	else if (s == 0.5)
	{
		vtBCoeffs[3] = vtBCoeffs[0] = 1.0/16;
		vtBCoeffs[2] = vtBCoeffs[1] = 7.0/16;
	}
	else
	{
		double s1on2 = 0.5*s;
		double s2on4 = s1on2*s1on2;
		vtBCoeffs[3] =  s2on4;						// [ 1,  2]
		vtBCoeffs[2] = -s2on4 + s1on2 + 0.25;	// [ 0,  1]
		vtBCoeffs[1] = -s2on4         + 0.5;	// [-1,  0]
		vtBCoeffs[0] =  s2on4 - s1on2 + 0.25;	// [-2, -1]
	}
}

void CPosConverter::GetBCoeffsAtPnt(CMatrix& mxBCoeffs, const CVector2& vtGridLoc)
{
	CMatrix vtsBCoeffs;
	CMatrix vttBCoeffs;
	GetBCoeffsAtLoc(vtsBCoeffs, vtGridLoc.x);		// creates a row vector
	GetBCoeffsAtLoc(vttBCoeffs, vtGridLoc.y);		// creates a row vector
	mxBCoeffs.ProdTl(vtsBCoeffs, vtsBCoeffs);		// creates a 4x4 matrix
}

void CPosConverter::GetBSplineAtLocs(CMatrix& mxBSplineLocs, const CMatrix& vtLocsNorm)
{
	// each vtLocsNorm value is used for a matrix row
	int len = vtLocsNorm.Length();
	ASSERT(mxBSplineLocs.h == len);		// size must be set already! - square generally
	mxBSplineLocs = 0;
	CMatrix vtBCoeffs(1,4);			// will be a row vector
	for (int i = 0; i < len; i++)
	{
		double locNorm = vtLocsNorm[i];
		int iNodeLoc = (int)floor(locNorm);
		GetBCoeffsAtLoc(vtBCoeffs, locNorm-iNodeLoc);		// creates a row vector
		vtBCoeffs.CopyToDestLoc(mxBSplineLocs, i, iNodeLoc);
	}
}

void CPosConverter::GetBSplineAtLoc(CMatrix& mxBSplineLoc, double locNorm)
{
	// locNorm value is used for a row vector
	ASSERT(mxBSplineLoc.IsVector());		// size must be set already!
	mxBSplineLoc = 0;
	CMatrix vtBCoeffs(1,4);			// will be a row vector
	int iNodeLoc = (int)floor(locNorm);
	GetBCoeffsAtLoc(vtBCoeffs, locNorm-iNodeLoc);		// creates a row vector
	vtBCoeffs.CopyToDestLoc(mxBSplineLoc, 0, iNodeLoc);
}


void CPosConverter::AddBCoeffEquAtPnt(const CMatrix& mxBCoeffs, const CPoint& ptGrid, const CVector& vtVal)
{
	// ptNode(base of 4x4) = ptGrid
	for (int t = 0; t < 4; t++)
	{
		int iColMx = (t + ptGrid.y) * m_szNodes.cx + ptGrid.x;
		for (int s = 0; s < 4; s++)
		{
			m_mxEqu.elem(m_numEqu, iColMx) = mxBCoeffs.elem(t, s);
			iColMx++;
		}
	}
	m_vtVal.SetRow(m_numEqu, vtVal);
	m_numEqu++;
}

//////////////////////////////////////////////////////

void CPosConverter::GetTipCorrFromPosServoSpline(const CVector& vtPosServo, double zOffset, CVector& vtTipCorr)
{
	// find correction distance of tip position relative to servo position from servo position
	// vtPosTip = (vtPosServo + vtZAxisHead2Tip) + vtTipCorr(vtPosServo, vtZAxisHead2Tip)
	// vtPosTip = (vtPosServo + vtZAxisHead2Tip) + ~vtTipCorr(vtPosServo + vtZAxisHead2Tip)
	vtTipCorr = 0;
	if (!m_bUsePosCorrection)
		return;

	CVector vtPosServoTip = vtPosServo;
	vtPosServoTip.z += zOffset;			// convert at tip position
	CPoint ptGrid;
	CVector2 vtGridLoc;
	GetNormalisedPnt(ptGrid, vtGridLoc, vtPosServoTip);	// vtPosServo.z not used

	CMatrix& vtXBCoeff  = m_vtXPower;		// row (1,4)
	CMatrix& vtYBCoeff  = m_vtYPower;		// row (1,4)
	CMatrix& vtZPower   = m_vtZPower;		// row (1,2)
	CMatrix& vtYZCoeff  = m_vtYZPower;		// col (8,1)
	CMatrix& vtResYZ    = m_vtResYZ;			// col (4,1)
	CMatrix& mxLocalNodes = m_mxLocalNodes;	// matrix (4,8)
	mxLocalNodes.SetSize(4,8);

	GetBCoeffsAtLoc(vtXBCoeff, vtGridLoc.x);
	GetBCoeffsAtLoc(vtYBCoeff, vtGridLoc.y);
	vtZPower.Power1Vector(vtPosServoTip.z);			// convert at tip position
//	vtYZCoeff.ProdTl(vtZPower, vtYBCoeff);				// matrix (2,4)
//	vtYZCoeff.ResizeToVectorByRows();					// column vector (8,1)
	vtYZCoeff.BlockScale(vtYBCoeff, vtZPower);		// row vector (1,8)

	CSize sizeCpy(4,4);
	CPoint ptDest1(0,0);
	CPoint ptDest2(4,0);
	CPoint ptSrc1(ptGrid.y, ptGrid.x);
	CPoint ptSrc2(ptGrid.y + m_szNodes.cy, ptGrid.x);

	for (int ax = 0; ax < 3; ax++)
	{
		if (!m_vtbUsePosCorrection[ax])
			continue;
		// get 4x(4x2) of nodes to use (2 layers)
		mxLocalNodes.CopyToLocFromSrcLoc(ptDest1, m_mxCorrCoeffsServo[ax], ptSrc1, sizeCpy);
		mxLocalNodes.CopyToLocFromSrcLoc(ptDest2, m_mxCorrCoeffsServo[ax], ptSrc2, sizeCpy);
		vtResYZ.ProdTr(mxLocalNodes, vtYZCoeff);			// col (4,1)
		vtTipCorr[ax] = dot(vtXBCoeff, vtResYZ);
	}
}











//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

void CPosConverter::SetCorrectionCoefficents()
{
	SetPatchCorrectionCoefficents();
	return;
/*
	position corrections are Head position relative to Servo postion
	at the defined Servo positions
*/
	// zCalc = zServo - [Bit Tip to Z Axis Head]
	double dTip2ZAxisHeadCal = 143.0 + 56.0 + 32.0;	// Tip to Axis Head when caibrated: Axis head to mount to chuck to tip

	// array locations are relative to centers
	double xCenter = 920;
	double arXLocs[4] = { -750, -250, 250, 750 };	// total X span 1500
	double yCenter = 420;
	double arYLocs[4] = { -390, -130, 130, 390 };	// total Y span 780
	double zCenter = -dTip2ZAxisHeadCal;
	double arZLocs[2] = { 60, 410 };	// router tip at base = 60
	// base to X trolley ~= 650, base to Y trolley ~= 700

	// dx correction
	double arXCorr[8][4] =
	{
		// at z0
		{  2.70,  2.70,  2.70,  2.70 },		// at y0
		{  1.80,  1.80,  1.80,  1.80 },		// at y1
		{  0.90,  0.90,  0.90,  0.90 },		// at y2
		{  0.00,  0.00,  0.00,  0.00 },		// at y3
		// at z1
		{  2.70,  2.70,  2.70,  2.70 },		// at y0
		{  1.80,  1.80,  1.80,  1.80 },		// at y1
		{  0.90,  0.90,  0.90,  0.90 },		// at y2
		{  0.00,  0.00,  0.00,  0.00 },		// at y3
	};
	// vtXLinearFactor = (0 ,-2.7/780.0 , 0);

	// dy correction
	double arYCorr[8][4] =
	{
		// at z0
		{ -0.54, -0.54, -0.54, -0.54 },		// at y0
		{ -0.18, -0.18, -0.18, -0.18 },		// at y1
		{  0.18,  0.18,  0.18,  0.18 },		// at y2
		{  0.54,  0.54,  0.54,  0.54 },		// at y3
		// at z1
		{ -0.27, -0.27, -0.27, -0.27 },		// at y0
		{ -0.09, -0.09, -0.09, -0.09 },		// at y1
		{  0.09,  0.09,  0.09,  0.09 },		// at y2
		{  0.27,  0.27,  0.27,  0.27 },		// at y3
	};
	// dY = dYonYZ * (y - y0) * (ztip - z0)
	// ztip = zservo - TipToHead
	// -0.54 = dYonYZ * (30 - y0) * (60-231 - z0)
	// y0 = 420
	// z0 = 700
	// -0.54 = dYonYZ * (-390) * (-871)

	// dz correction
	double arZCorr[8][4] =
	{
		// at z0
		{  0.30,  0.10, -0.10, -0.30 },		// at y0
		{ -0.10, -0.30, -0.50, -0.70 },		// at y1
		{ -0.10, -0.30, -0.50, -0.70 },		// at y2
		{  0.30,  0.10, -0.10, -0.30 },		// at y3
		// at z1
		{  0.30,  0.10, -0.10, -0.30 },		// at y0
		{ -0.10, -0.30, -0.50, -0.70 },		// at y1
		{ -0.10, -0.30, -0.50, -0.70 },		// at y2
		{  0.30,  0.10, -0.10, -0.30 },		// at y3
	};
	// vtZLinearFactor = (-0.4e-3, 0, 0);


/*
		// at z0
		{  0.00,  0.00,  0.00,  0.00 },		// at y0
		{  0.00,  0.00,  0.00,  0.00 },		// at y1
		{  0.00,  0.00,  0.00,  0.00 },		// at y2
		{  0.00,  0.00,  0.00,  0.00 },		// at y3
		// at z1
		{  0.00,  0.00,  0.00,  0.00 },		// at y0
		{  0.00,  0.00,  0.00,  0.00 },		// at y1
		{  0.00,  0.00,  0.00,  0.00 },		// at y2
		{  0.00,  0.00,  0.00,  0.00 },		// at y3

  // dx correction
	double arXCorr[8][4] =
	{
		// at z0
		{ -1.00, -0.30,  0.30,  1.00 },		// at y0
		{ -1.00, -0.30,  0.30,  1.00 },		// at y1
		{ -1.00, -0.30,  0.30,  1.00 },		// at y2
		{ -1.00, -0.30,  0.30,  1.00 },		// at y3
		// at z1
		{ -0.50, -0.15,  0.15,  0.50 },		// at y0
		{ -0.50, -0.15,  0.15,  0.50 },		// at y1
		{ -0.50, -0.15,  0.15,  0.50 },		// at y2
		{ -0.50, -0.15,  0.15,  0.50 },		// at y3
	};

	// dy correction
	double arYCorr[8][4] =
	{
		// at z0
		{ -0.60, -0.60, -0.60, -0.60 },		// at y0
		{ -0.20, -0.20, -0.20, -0.20 },		// at y1
		{  0.20,  0.20,  0.20,  0.20 },		// at y2
		{  0.60,  0.60,  0.60,  0.60 },		// at y3
		// at z1
		{ -0.30, -0.30, -0.30, -0.30 },		// at y0
		{ -0.10, -0.10, -0.10, -0.10 },		// at y1
		{  0.10,  0.10,  0.10,  0.10 },		// at y2
		{  0.30,  0.30,  0.30,  0.30 },		// at y3
	};
	
	// dz correction
	double arZCorr[8][4] =
	{
		// at z0
		{  0.00,  0.00,  0.00,  0.00 },		// at y0
		{ -0.40, -0.40, -0.40, -0.40 },		// at y1
		{ -0.40, -0.40, -0.40, -0.40 },		// at y2
		{  0.00,  0.00,  0.00,  0.00 },		// at y3
		// at z1
		{  0.00,  0.00,  0.00,  0.00 },		// at y0
		{ -0.40, -0.40, -0.40, -0.40 },		// at y1
		{ -0.40, -0.40, -0.40, -0.40 },		// at y2
		{  0.00,  0.00,  0.00,  0.00 },		// at y3
	};
	// dz correction
	double arZCorr[8][4] =
	{
		// at z0
		{  0.00, -1.00, -1.00,  0.00 },		// at y0
		{ -0.40, -1.40, -1.40, -0.40 },		// at y1
		{ -0.40, -1.40, -1.40, -0.40 },		// at y2
		{  0.00, -1.00, -1.00,  0.00 },		// at y3
		// at z1
		{  0.00, -1.00, -1.00,  0.00 },		// at y0
		{ -0.40, -1.40, -1.40, -0.40 },		// at y1
		{ -0.40, -1.40, -1.40, -0.40 },		// at y2
		{  0.00, -1.00, -1.00,  0.00 },		// at y3
	};
*/

	
	CMatrix vtXLocs(4);
	CMatrix vtYLocs(4);
	CMatrix vtZLocs(2);
	vtXLocs.SetFromArray(arXLocs);
	vtYLocs.SetFromArray(arYLocs);
	vtZLocs.SetFromArray(arZLocs);
	vtXLocs += xCenter;
	vtYLocs += yCenter;
	vtZLocs += zCenter;

	// calc using 3 seperate inverses!
	CMatrix mxXLocsPower;
	CMatrix mxYLocsPowerT;
	CMatrix mxZLocsPowerT;
	mxXLocsPower.Power3Matrix(vtXLocs);			// (4,4)	
	mxYLocsPowerT.Power3Matrix(vtYLocs);		// (4,4)
	mxZLocsPowerT.Power1Matrix(vtZLocs);		// (2,2)
	mxYLocsPowerT.Transpose();
	mxZLocsPowerT.Transpose();

	CMatrix mxXLocsPowerInv;
	CMatrix mxYLocsPowerInv;
	CMatrix mxZLocsPowerInv;
	mxXLocsPower.LUInverse(mxXLocsPowerInv);		// (4,4)
	mxYLocsPowerT.LUInverse(mxYLocsPowerInv);		// (4,4)
	mxZLocsPowerT.LUInverse(mxZLocsPowerInv);		// (2,2)

	CMatrix mxYZLocsPowerInv;							// (8,8);
	mxYZLocsPowerInv.BlockScale(mxYLocsPowerInv, mxZLocsPowerInv);
	
	
	
/*	
	mxXLocsPower.Power3Matrix(vtXLocs);
	mxYLocsPowerT.Power3Matrix(vtYLocs);
	mxYLocsPowerT.Transpose();

	mxYLocsPowerT.CopyToDestLoc(mxYZLocsPower, 0,0);
	mxYLocsPowerT.CopyToDestLoc(mxYZLocsPower, 0,4);
	(mxYLocsPowerT * vtZLocs[0]).CopyToDestLoc(mxYZLocsPower, 4,0);
	(mxYLocsPowerT * vtZLocs[1]).CopyToDestLoc(mxYZLocsPower, 4,4);

	CMatrix mxXLocsPowerInv;			// (4,4)
	CMatrix mxYZLocsPowerInv;			// (8,8)
	mxXLocsPower.LUInverse(mxXLocsPowerInv);
	mxYZLocsPower.LUInverse(mxYZLocsPowerInv);
*/

	CMatrix mxCorr(4,8);
	CMatrix mxXCoef(4,8);

	// to check
	CMatrix mxYZLocsPower;							// (8,8);
	mxYZLocsPower.BlockScale(mxYLocsPowerT, mxZLocsPowerT);
	CMatrix mxCalcCorrDiff;
	int iCheck = 0;
	double diffMax = 1e-8;

	// for X
	mxCorr.SetFromArrayByCols(*arXCorr);
	mxXCoef.Prod(mxXLocsPowerInv, mxCorr);
	m_mxCorrCoeffsServo[0].Prod(mxXCoef, mxYZLocsPowerInv);		// (4,8)
	// to check:
	mxCalcCorrDiff = mxXLocsPower * m_mxCorrCoeffsServo[0] * mxYZLocsPower - mxCorr;
	if (mxCalcCorrDiff.Mag() > diffMax)
		iCheck++;

	// for Y
	mxCorr.SetFromArrayByCols(*arYCorr);
	mxXCoef.Prod(mxXLocsPowerInv, mxCorr);
	m_mxCorrCoeffsServo[1].Prod(mxXCoef, mxYZLocsPowerInv);
	// to check:
	mxCalcCorrDiff = mxXLocsPower * m_mxCorrCoeffsServo[1] * mxYZLocsPower - mxCorr;
	if (mxCalcCorrDiff.Mag() > diffMax)
		iCheck++;

	// for Z
	mxCorr.SetFromArrayByCols(*arZCorr);
	mxXCoef.Prod(mxXLocsPowerInv, mxCorr);
	m_mxCorrCoeffsServo[2].Prod(mxXCoef, mxYZLocsPowerInv);
	// to check:
	mxCalcCorrDiff = mxXLocsPower * m_mxCorrCoeffsServo[2] * mxYZLocsPower - mxCorr;
	if (mxCalcCorrDiff.Mag() > diffMax)
		iCheck++;


//	afxDump << "mxCorr: " << mxCorr;
//	afxDump << "mxCalcCorr: " << mxCalcCorr;
//	afxDump << "m_mxCorrCoeffsServo: " << m_mxCorrCoeffsServo[0];

	if (iCheck != 0)
	{
		ASSERT(0);
		LOGERROR("Error calculating correction coefficents");
	}



/*
	mxCorr.SetSize(4,4);
	CMatrix mxXCoef(4,4);

	// for X
	mxCorr.SetFromArrayByCols(arXCorr[0]);
	mxXCoefl0.Prod(mxXLocsPowerInv, mxCorr);
	m_mxCorrCoeffsServo[0].Prod(mxXCoef, mxYZLocsPowerInv);		// (4,8)
*/	

}


void CPosConverter::GetTipCorrFromPosServo(const CVector& vtPosServo, double zOffset, CVector& vtTipCorr)
{
	GetTipCorrFromPosServoSpline(vtPosServo, zOffset, vtTipCorr);
	return;

	// find correction distance of tip position relative to servo position from servo position
	// vtPosTip = (vtPosServo + vtZAxisHead2Tip) + vtTipCorr(vtPosServo, vtZAxisHead2Tip)
	// vtPosTip = (vtPosServo + vtZAxisHead2Tip) + ~vtTipCorr(vtPosServo + vtZAxisHead2Tip)
	vtTipCorr = 0;
	if (!m_bUsePosCorrection)
		return;

	CMatrix& vtXPower = m_vtXPower;		// row (1,4)
	CMatrix& vtYPower = m_vtYPower;		// row (1,4)
	CMatrix& vtZPower = m_vtZPower;		// row (1,2)
	CMatrix& vtYZPower = m_vtYZPower;	// col (8,1)
	CMatrix& vtResYZ = m_vtResYZ;			// col (4,1)

	vtXPower.Power3Vector(vtPosServo.x);
	vtYPower.Power3Vector(vtPosServo.y);
	vtZPower.Power1Vector(vtPosServo.z + zOffset);	// convert at tip position
	vtYZPower.ProdTl(vtZPower, vtYPower);		// (2,4)
	vtYZPower.ResizeToVectorByRows();			// column vector (8,1)

	//CMatrix mxResVal(1);
	for (int ax = 0; ax < 3; ax++)
	{
		if (!m_vtbUsePosCorrection[ax])
			continue;
		vtResYZ.Prod(m_mxCorrCoeffsServo[ax], vtYZPower);
		vtTipCorr[ax] = dot(vtXPower, vtResYZ);
		//mxResVal.Prod(vtXPower, vtResYZ);
		//vtTipCorr[ax] = mxResVal[0];
	}
}

void CPosConverter::GetTipCorrFromPosTip(const CVector& vtPosTip, CVector& vtTipCorr)
{
	// find correction distance of tip position relative to servo position from tip position
	// just assume servo position ~= head position for now!
	// could get different coefficents for corrections at head position
	CVector vtTipCorr1;
	GetTipCorrFromPosServo(vtPosTip, m_vtZAxisHead2Tip.z, vtTipCorr1);		// servo position ~= head position

	CVector vtPosServo1 = vtPosTip - vtTipCorr1;		// do an iteration to get approx servo pos
	GetTipCorrFromPosServo(vtPosServo1, m_vtZAxisHead2Tip.z, vtTipCorr);		// servo position ~= head position
	CVector vtTipCorrChange = vtTipCorr - vtTipCorr1;
	double magChange = vtTipCorrChange.MaxAbsElem();
	if (magChange > 0.04)
		LOGERROR1("large corr change %f", magChange);
}


//////////////////////////////

void CPosConverter::GetPosServoFromPosTip(const CVector& vtPosTip, CVector& vtPosServo)
{
	CVector vtTipCorr;
	GetTipCorrFromPosTip(vtPosTip, vtTipCorr);
	vtPosServo = vtPosTip - vtTipCorr;
}
void CPosConverter::GetPosServoFromPosTip(CVector& vtPos)
{
	CVector vtTipCorr;
	GetTipCorrFromPosTip(vtPos, vtTipCorr);
	// adjust vtPos from  PosTip to PosServo
	vtPos -= vtTipCorr;
}

void CPosConverter::GetPosTipFromPosServo(const CVector& vtPosServo, CVector& vtPosTip)
{
	CVector vtTipCorr;
	GetTipCorrFromPosServo(vtPosServo, m_vtZAxisHead2Tip.z, vtTipCorr);
	vtPosTip = vtPosServo + vtTipCorr;
}
void CPosConverter::GetPosTipFromPosServo(CVector& vtPos)
{
	CVector vtTipCorr;
	GetTipCorrFromPosServo(vtPos, m_vtZAxisHead2Tip.z, vtTipCorr);
	// adjust vtPos from PosServo to PosTip
	vtPos += vtTipCorr;
}

void CPosConverter::GetPosProbeTipFromPosServo(const CVector& vtPosServo, double dProbeExt, CVector& vtPosProbeTip)
{
	CVector vtProbeTipCorr;
	double zAxisHead2ProbeTip = m_vtZAxisHead2ProbeRetracted.z - dProbeExt;
	GetTipCorrFromPosServo(vtPosServo, zAxisHead2ProbeTip, vtProbeTipCorr);
	vtPosProbeTip = vtPosServo + vtProbeTipCorr;
	vtPosProbeTip.z += zAxisHead2ProbeTip - m_vtZAxisHead2Tip.z;
}

