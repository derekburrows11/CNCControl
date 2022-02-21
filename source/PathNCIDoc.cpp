// PathNCIDoc.cpp : implementation file
//

#include "stdafx.h"
#include "CNCControl.h"
#include "PathNCIDoc.h"

#include "PolyFunc.h"
#include "PolyCurveFit.h"

#include "ReadNCIFile.h"
#include "Settings.h"

#include <fstream.h>
#include <iomanip.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////



struct SNodeSegProps : SNodeRelProps, SSegRelProps
{
	void SetFrom(const SNodeRelProps& ndCurr)	{ *(SNodeRelProps*)this = ndCurr; }
	void SetFrom(const SSegRelProps& segPrev) { *(SSegRelProps*)this = segPrev; }
//	static char* GetHeading() { return " NR    l       seg     span               vtCurve               dCurve.z   d2Curve.z  ratioSplit  magCurveSplit  split \n"; }
	static char* GetHeading() { return " NR    l       seg     span               vtCurve               dCurve.z   d2Curve.z  ratioSplit  magCurveSplit  split  dCurveFit  dCurveFitResidual \n"; }
//	static char* GetHeading() { return " NR    l       seg     span               vtCurve            magCurveError  dCurve.z   d2Curve.z  ratioSplit  magCurveSplit  split \n"; }
//	static char* GetHeading() { return " NR    l       seg     span   arcSpan chord               vtCurve            magCurveError   mag/ang    dCurve.z   d2Curve.z  ratioSplit  magCurveSplit  split \n"; }
};
typedef CArray<SNodeSegProps, SNodeSegProps&> CNodeSegPropsArray;


ostream& operator<<(ostream& os, const SNodeSegProps& nd)
{
	// first line is for segment properties
	os.setf(ios::fixed | ios::showpoint);
	os.precision(2);
	os << setw(11) << ' ';
	os << setw(8) << nd.mag;
	os << setw(9+32) << ' ';
//	os << setw(12) << ' ';		// checking
	os.unsetf(ios::fixed);
	os.precision(3);
//	os << setw(12) << nd.vtCurveCng.z;
	os << setw(12) << nd.vtdCurve.z;
	os << endl;

	// second line is for node properties
	os.setf(ios::fixed);
	os.precision(2);
	os << setw(3) << nd.nr;
	os << setw(8) << nd.l;
	os << setw(8) << "--";
//	os.precision(3);
	os << setw(8) << nd.span;
//	os << setw(8) << nd.arcSpan;		// checking
//	os << setw(8) << nd.chord;			// checking
	os.unsetf(ios::fixed);
	os.precision(3);
	os << ' ' << nd.vtCurve;
//	os << setw(12) << nd.magCurveError;					// checking
//	os << setw(12) << nd.magDirCng / nd.angDirCng;	// checking
	os << setw(12) << "--";
	os << setw(12) << nd.vtd2Curve.z;
	os << setw(12) << nd.ratioSplit;
	os << setw(12) << nd.magCurveSplit;
	os << setw(10) << nd.split;

	os << setw(12) << nd.dCurveFit;
	os << setw(12) << nd.dCurveFitResidual;


	os << endl;
	return os;
}


/////////////////////////////////////////////////////////////////////////////
// CPathNCIDoc

IMPLEMENT_DYNCREATE(CPathNCIDoc, CPathDoc)

CPathNCIDoc::CPathNCIDoc()
{
	m_pCurveFitter = NULL;
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

		if (g_Settings.PathOptions.m_SmoothSegs.bSmooth)
		{
			try
			{
				FindNodeBounds();
				SmoothCurves();
			}
			catch (...)
			{
				TRACE("Got exception\n");
				ASSERT(0);

			}

			m_NodeArray.RemoveAll();
			m_NodeArray.SetSize(0);
			m_CurveInfoArray.SetSize(0);		// no longer relevent
			m_NodeArray.Copy(m_FittedBezierArray);		// set to fitted array
		}

		if (g_Settings.PathOptions.m_JoinTab.bAdd)
			JoiningTabs();

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

	CPolyCurveFit curveFitter;
	curveFitter.SetPath(this);
	m_pCurveFitter = &curveFitter;

	// method
	bool bUsingLengths   = 0;	// one or the other!
	bool bUsingArcErrors = 1;

	// For all methods
	// 1 rad = 57.3 deg,  0.1 rad ~= 6 deg
	double angChangeSmall = g_Settings.PathOptions.m_SmoothSegs.angCusp * deg2rad;			// was 10 deg
	double angChangeNone = 0.1e-3 * deg2rad;		// was 0.06e-3 deg
	double magSegTiny = 0.5 * g_Settings.PathOptions.m_SmoothSegs.segTol;

	// Only for bUsingArcErrors method
	double errArcSmall = g_Settings.PathOptions.m_SmoothSegs.segTol;					// mm was 0.1, 0.05, 0.016, 0.30 (for tight curves?)
	double errArcSmallDirSet = g_Settings.PathOptions.m_SmoothSegs.segTolDirSet;	// mm was errArcSmall * 11.0/8
	double magSegStraightMin = g_Settings.PathOptions.m_SmoothSegs.segStraightMin;	// mm was 3.0;

	// Only for bUsingLengths method
	double magSegShort = 4.0;							// mm maybe 3mm?

	

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
		ST_BEZCONTROL,

	};

	bool bInCurve = false;
	bool bGotActualCurve = false;		// set when a bend is encounted

	m_CurveInfoArray.SetSize(0,512);
	m_FittedBezierArray.SetSize(0,1024);
	m_nrLastAddedToFittedArray = -1;		// start value

	CNodeSegPropsArray curveNodeArray;
	CNodeSumsArray nodeSumsArray;

	ofstream os;
//	os.open("CurveNodes,txt");

	SCurveInfo curve;
	curve.nStartDir = DIR_FREE;
	curve.nEndDir = DIR_FREE;
	curve.numSegs = 0;
	curve.pNodeSumsArray = &nodeSumsArray;


	NODEREF nrLastDirChange;
	bool bEndCurveDirSetAtLastDirChange;
	CVector vtEndCurveDirAtLastDirChange;

	int iNumStraightSegs = 0;		// not in straight
	int iNumStraightSegsPotential = 0;
	double magStraightSegs;
	NODEREF nrStartStraight;
	CVector vtStartStraightDir;



	#define SIZENODEARRAY 128
	#define NUMNODESAHEAD 1
	#define NUMNODESBEHIND 3
	SNodeRelProps arNode[SIZENODEARRAY];
	SSegRelProps arSeg[SIZENODEARRAY];
	int idxCurr = NUMNODESBEHIND - 1;
	
	SNodeRelProps* pNdCurr = arNode + idxCurr;
	SSegRelProps* pSegPrev = arSeg + idxCurr;
	SSegRelProps* pSegNext = pSegPrev + 1;

	SNodeRelProps& ndCurr = pNdCurr[0];
	SNodeRelProps& ndNext = pNdCurr[1];
	SSegRelProps& segNext = pSegNext[0];


	segNext.vt = 0.0;
	segNext.mag = -1;
	segNext.num = 0;
	segNext.bValid = false;
	segNext.nType = ST_NOTSET;


	ndNext.pNode = GetFirstNode(ndNext.nr);
	ndNext.l = 0;					// will be changed though be init segPrev!
	ndNext.vtd2Curve = 0;		// will remain 0
	if (ndNext.pNode == NULL)
		return;			// end of list
	ASSERT(ndNext.pNode->type & PNTF_END);		// first will be end node

	// Set initial condition - set segPrev.vt so change from segPrev.vt to segNext.vt is large
	ndCurr.nr = ndNext.nr;					// preload next node without changing nrNext
	ndCurr.pNode = GetNextNode(ndCurr.nr);		// setup so segPrev.vt is opposite segNext.vt - therefor change is large
	ndCurr.l = 0;
	if (ndCurr.pNode == NULL)
		return;			// end of list

	int iTinySegCount = 0;

	bool bAtEndOfNodes = false;
	while (!bAtEndOfNodes)
	{
		// advance node and segment references - reset if at end
		if (++idxCurr >= SIZENODEARRAY - NUMNODESAHEAD)
		{
			// copy close nodes and segs back to start of arrays
			int iDest = 0;
			for (int iSrc = idxCurr-NUMNODESBEHIND; iSrc < SIZENODEARRAY; iSrc++, iDest++)
			{
				arNode[iDest] = arNode[iSrc];		// number copied = NUMNODESBEHIND + NUMNODESAHEAD
				arSeg[iDest] = arSeg[iSrc];
			}
			idxCurr = NUMNODESBEHIND;
		}
		pNdCurr = arNode + idxCurr;
		pSegPrev = arSeg + idxCurr;
		pSegNext = pSegPrev + 1;

//		SNodeRelProps& ndPrev2= pNdCurr[-2];
		SNodeRelProps& ndPrev = pNdCurr[-1];
		SNodeRelProps& ndCurr = pNdCurr[0];
		SNodeRelProps& ndNext = pNdCurr[1];
//		SSegRelProps& segPrev3= pSegPrev[-2];
		SSegRelProps& segPrev2= pSegPrev[-1];
		SSegRelProps& segPrev = pSegPrev[0];
		SSegRelProps& segNext = pSegNext[0];
		CPathNode*& pNextNode = ndNext.pNode;


		// read ndNext

		segNext.nType = ST_NOTSET;
		bool bBreakCurve = false;
		ndNext.nr = ndCurr.nr;		// start from ndCurr
		for (;;)
		{
			pNextNode = GetNextPathNode(ndNext.nr);
			if (pNextNode == NULL)		// end of list		- curve must be finalised
			{
				bAtEndOfNodes = true;
				pNextNode = ndPrev.pNode;	// setup so segPrev.vt is opposite segNext.vt - therefor change is large
				ASSERT(ndCurr.pNode->type & PNTF_END);		// last will be end node
				break;
			}
			if (pNextNode->IsPoint())
				break;
			if (pNextNode->IsCommand())	// command node, force break in curve
				bBreakCurve = true;
		}



		ASSERT(pNextNode->type & (PNTF_END | PNTF_CONTROL));	// must be end or control
		segNext.bValid = (pNextNode->type & PNTF_END) && (ndCurr.pNode->type & PNTF_END);
		segNext.num = segPrev.num;
		if (pNextNode->type & PNTF_END)
			segNext.num++;

		if (!segNext.bValid)
		{
			segNext.nType = ST_BEZCONTROL;
			if (!segPrev.bValid)		// neither segments are valid
			{
				ASSERT(!bInCurve);
				continue;
			}
			if (!bInCurve)				// next seg not valid but not in curve so end dir won't be needed
				continue;
			bBreakCurve = true;
		}

		if (!segPrev.bValid)				// (ndPrev is control) if prev seg was not calculated, do it now - may be needed for start curve dir
		{
			segPrev.SetSegment(*ndCurr.pNode - *ndPrev.pNode);		// don't do on first pass!
			segPrev.nMag = (segPrev.mag <= magSegShort) ? MAG_SHORT : MAG_LONG;
		}
		if (ndPrev.pNode->type & PNTF_CONTROL)
			segPrev.nType = ST_BEZCONTROL;


		segNext.SetSegment(*pNextNode - *ndCurr.pNode);
		segNext.nMag = (segNext.mag <= magSegShort) ? MAG_SHORT : MAG_LONG;

		if (segNext.mag == 0)		// then load the next node into pNextNode and retry
		{
			ASSERT(0);
			continue;		// skip past null segments
		}

		// remove tiny segments
		if (segNext.bValid && segNext.mag <= magSegTiny)
		{
			iTinySegCount++;
			ASSERT(iTinySegCount <= 5);
			HideEndNode(ndNext.nr);
			// do adjustments required to restart and load ndNext again
			idxCurr--;			// segNext.num
			continue;
		}
		else
			iTinySegCount = 0;


		
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
		// errArc = curve/8 * magSeg^2
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





		ndCurr.l = ndPrev.l + segPrev.mag;
		ndCurr.magDirCngSum = ndPrev.magDirCngSum + 0.5*(ndPrev.magDirCng + ndCurr.magDirCng);
		SNodeSums ndCurrSums;
		ndCurrSums.nr = ndCurr.nr;
		ndCurrSums.l = ndCurr.l;
		ndCurrSums.magDirCngSum = ndCurr.magDirCngSum;
		nodeSumsArray.Add(ndCurrSums);

		segPrev.SetCrossChange(ndPrev, ndCurr);
		ndPrev.SetCurveChange2(segPrev2, segPrev);	// only for vtd2Curve


		// FindCurveSplit - match adjacent curvatures
		//double c0 = ndPrev2.vtCurve.z;			// take mag of curve in common (curve normal) direction
		//double c1 = ndCurr.vtCurve.z;
		// FindCurveSplit - match ramping curvatures
/*		double c0 = ndPrev2.vtCurve.z + segPrev2.mag * segPrev3.vtdCurve.z;		// take mag of curve in common (curve normal) direction
		double c1 = ndCurr.vtCurve.z - segPrev.mag * segNext.vtdCurve.z;
		double cmid = ndPrev.vtCurve.z;
		double ratio = (cmid - c0) * segPrev2.mag / ((c1 - cmid) * segPrev.mag);
		ndPrev.ratioSplit = ratio;
		ndPrev.magCurveSplit = c1 - c0;
*/
		{
			// requires pSegNext[1].vtdCurve calc'd -> pSegNext[2] set -> ndCurr[3] set
			int idx = idxCurr - 2;
			SNodeRelProps* pNdCurr = arNode + idx;
			SSegRelProps* pSegPrev = arSeg + idx;
			SSegRelProps* pSegNext = pSegPrev + 1;

			// FindCurveSplit - match ramping curvatures
	// Curve(n1) = ((c0+2c1)d0 + (2c2+c3)d1) / 3(d0+d1)
	// Curve(n1) = (c0.d0 + c3.d1 + 2(c1.d0 + c2.d1)) / 3(d0+d1)
	// c1.d0 + c2.d1 = 3/2.Curve(n1).(d0+d1) - (c0.d0 + c3.d1) / 2
	// if no step (cmid = c1 = c2)
	// cmid = 3/2.Curve(n1) - (c0.d0 + c3.d1) / 2(d0+d1)
	// cmid = (c1.d0 + c2.d1) / (d0+d1)
			// calc expected step values extrapolating from adjacent curvature
			double d0 = pSegPrev[0].mag;
			double d1 = pSegNext[0].mag;
			double c0 = pNdCurr[-1].scalarCurve;
			double c3 = pNdCurr[+1].scalarCurve;
			double cc = pNdCurr[0].scalarCurve;
			// calc expected step values extrapolating from adjacent curvature
			double c1 = c0 + d0 * pSegPrev[-1].dScalarCurve;		// take mag of curve in common (curve normal) direction
			double c2 = c3 - d1 * pSegNext[+1].dScalarCurve;
			double cmid = 1.5*cc - 0.5*(c0*d0+c3*d1) / (d0+d1);
			double ratio = (cmid - c1) * pSegPrev[0].mag / ((c2 - cmid) * pSegNext[0].mag);
			double magSplit = c2 - c1;
			//double magCurveChange = pSegNext[+1].dScalarCurve - pSegPrev[-1].dScalarCurve;
			pNdCurr[0].ratioSplit = ratio;
			pNdCurr[0].magCurveSplit = magSplit;
			// calc curve change requirement at ndCurr
			// using curve at 2 nodes both sides




			// do least squares fit of line to 3 adjacent curves - Kreyszig p.1030
			// [ Sx0  Sx1 ] * [a0] = [ Sx0y1 ]
			// [ Sx1  Sx2 ]   [a1]   [ Sx1y1 ]
			// Sum Squares = Sy2 + a0*a0*Sx0 * a1*a1*Sx2
			//					- 2*(a0*Sx0y1 + a1*Sx1y1)
			//					+ 2*(a0*a1*Sx1)
			// (-d0,c0) (0,cmid) (d1,c3)
			double Sx0 = 3;
			double Sx1 = d1 - d0;
			double Sx2 = d0*d0 + d1*d1;
			double Sx0y1 = c0 + cmid + c3;
			double Sx1y1 = d1*c3 - d0*c0;
			double Sy2 = c0*c0 + cmid*cmid + c3*c3;
			double det = Sx0*Sx2 - Sx1*Sx1;
			det = 1.0 / det;
			double a0 = det * ( Sx2*Sx0y1 - Sx1*Sx1y1);
			double a1 = det * (-Sx1*Sx0y1 + Sx0*Sx1y1);
			double q1 = Sy2 + a0*a0*Sx0 + a1*a1*Sx2 - 2*(a0*Sx0y1 + a1*Sx1y1) + 2*(a0*a1*Sx1);
//			double e0 = c0-a0+a1*d0;
//			double e1 = cmid-a0;
//			double e2 = c3-a0-a1*d1;
//			double q2 = e0*e0 + e1*e1 + e2*e2;
			double q = sqrt(q1);
			pNdCurr[0].dCurveFit = a1;
			pNdCurr[0].dCurveFitResidual = q;




			// check for appropriateness of split
//			double curveCngNodeMax = max(fabs(pSegPrev[-1].scalarCurveCng), fabs(pSegNext[1].scalarCurveCng));
			double curveNodeMax = max(fabs(c0), fabs(c3));
			double ratioSplit = (curveNodeMax != 0) ? (c3 - c0) / curveNodeMax : 0;

			bool bSplit = false;
			pNdCurr[0].split = 0;
			//pNdCurr[0].split = ratioSplit;
			//if (fabs(magSplit) > 0.5 * curveNodeMax && curveNodeMax != 0)
			if (fabs(ratioSplit) > 0.5)
			{
				if (ratio < 1 && ratio > 0.1)
					ratio = 1/ratio;
				if (ratio > 0.9 && ratio < 1.5)
				{
					bSplit = true;
					pNdCurr[0].split += 1;
				}
			}

			// if curve change % > 2 before and after
			double ratioPrev = fabs((c0 != 0) ? cc / c0 : cc * 1e8);
			double ratioNext = fabs((cc != 0) ? c3 / cc : c3 * 1e8);
			double ratioThres = 2;
			double ratioThresInv = 0.5;
			double ratioThresSq = 4;
			double ratioThresInvSq = 0.25;

			if (ratioPrev > ratioThres && ratioNext > ratioThres)
			{
				bSplit = true;
				pNdCurr[0].split += 10;
			}
			if (ratioPrev < ratioThresInv && ratioNext < ratioThresInv)
			{
				bSplit = true;
				pNdCurr[0].split += 100;
			}
			// 
			if (!bSplit && (ratioPrev > ratioThresSq || ratioPrev < ratioThresInvSq))
				if (pNdCurr[-1].split == 0)
				{
					bSplit = true;
					pNdCurr[0].split += 1000;
				}

			if (bSplit)
				curve.FitBreakArray.Add(pNdCurr[0].nr);


			SNodeSegProps ndStore;
			ndStore.SetFrom(pNdCurr[0]);
			ndStore.SetFrom(pSegPrev[0]);
			curveNodeArray.Add(ndStore);
		}




/*
		int num = curveNodeArray.GetSize();
		if (num >= 1)
		{
			SNodeSegPropsStore& ndStorePrev = curveNodeArray.ElementAt(num-1);
			//ndStorePrev.vtd2Curve = (ndStore.vtdCurve - ndStorePrev.vtdCurve) / ndStorePrev.span;
			ndStorePrev.vtd2Curve = ndPrev.vtd2Curve;
			if (num >= 2)
			{
				SNodeSegPropsStore& ndStorePrev2 = curveNodeArray.ElementAt(num-2);
				ndStorePrev2.ratioSplit = ndPrev2.ratioSplit;
				ndStorePrev2.magCurveSplit = ndPrev2.magCurveSplit;
			}
		}
		SNodeSegPropsStore ndStore;
		ndStore.SetFrom(ndCurr);
		ndStore.SetFrom(segPrev);
		ndStore.vtd2Curve = 0;
		ndStore.ratioSplit = 0;
		ndStore.magCurveSplit = 0;
		curveNodeArray.Add(ndStore);
*/

/*

	split ndCurr.Curve -> ndCurr.preCurve / ndCurr.postCurve
	weighted avg == ndCurr.Curve
	(segPrev.mag * ndCurr.preCurve + segNext.mag * ndCurr.postCurve) / (2*ndCurr.span)

		such that segPrev.dCurve ~= segPrev2.dCurve
		      and segNext.dCurve ~= segNext2.dCurve

	if split fits and step is small,				no change in curve needed
	if split fits and step is significant,		change curve at node
	if split won't fit,								cusp at node



*/



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
					curve.nStartDir = DIR_FREE;
					break;
				case DIR_CHANGE_SMALL:		// start curve at this node, match end direction
					bCouldStartCurve = true;
					curve.nrStart = ndCurr.nr;
					curve.nStartDir = DIR_SET;
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
				curve.nEndDir = DIR_FREE;
				break;
			case DIR_CHANGE_SMALL:		// end curve at this node, match end direction
				bCouldEndCurve = true;
				curve.nrEnd = ndCurr.nr;
				curve.nEndDir = DIR_SET;
				curve.vtEndDir = segNext.vt;
				break;
			case DIR_CHANGE_NONE:		// end curve at earlier node where direction changes
				bCouldEndCurve = true;
				curve.nrEnd = nrLastDirChange;
				if (bEndCurveDirSetAtLastDirChange)
				{
					curve.nEndDir = DIR_SET;
					curve.vtEndDir = vtEndCurveDirAtLastDirChange;	// valid if bEndDirSetAtLastDirChange == true
				}
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
				curve.nEndDir = DIR_FREE;

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
	iErrArcPrev		iErrArcPrevIfDirSet			In curve						Not in curve
			iErrArcNext		iErrArcNextIfDirSet
		s		s			x		x					continue						start prev node, dir free

		s		l			s		x			end curr node, dir next seg*	start curr node, dir free
		s		l			l		x			end curr node, dir free			start curr node, dir free

		l		s			x		s			end curr node, dir free			start curr node, dir prev seg*
		l		s			x		l			end curr node, dir free			start curr node, dir free

		l		l			x		x			end curr node, dir free			start curr node, dir free

		* only if next or prev segs are not to be curve fitted

	curve end directions are free or set from adjacent straight line or existing bezier (from arc etc.)

*/




if (bUsingArcErrors)	// using arc errors
{

//	if (segPrev.nMag == MAG_LONG || segNext.nMag == MAG_LONG)		// added for tree frame letters
//		bBreakCurve = true;

	bool bArcError = (iErrArcPrev == ERROR_LARGE) || (iErrArcNext == ERROR_LARGE);
	// check end curve conditions
//	int nrPathNext = m_FittedBezierArray.GetSize() - 1;	// just for debugging ref

	if (bInCurve)
	{
		if (nMagDirChange == DIR_CHANGE_NONE)		// keep track of consecutive straight sections
		{
			// check for start of potential straight
			if (iNumStraightSegsPotential == 0)			// if not in straight
			{
				nrStartStraight = ndPrev.nr;		// check doesn't get here first time!!
				magStraightSegs = segPrev.mag + segNext.mag;
				iNumStraightSegsPotential = 2;
			}
			else
			{
				iNumStraightSegsPotential++;
				magStraightSegs += segNext.mag;
			}
			// check if straight is valid
			if (iNumStraightSegs == 0)			// if not set as valid yet
			{
				if (iNumStraightSegsPotential >= 3 || magStraightSegs >= magSegStraightMin)
				{
					vtStartStraightDir = (segPrev.mag > segNext.mag) ? segPrev.vt : segNext.vt;
					iNumStraightSegs = iNumStraightSegsPotential;
				}
			}
			else
				iNumStraightSegs = iNumStraightSegsPotential;
		}
		else									// changing direction
			iNumStraightSegs = iNumStraightSegsPotential = 0;		// not in straight


		if (bArcError || bBreakCurve || (iNumStraightSegs >= 2))		// end curve signals
		{
			curve.nrEnd = ndCurr.nr;
			curve.nEndDir = DIR_FREE;
			if (nMagDirChange == DIR_CHANGE_NONE)
			{
				if (iNumStraightSegs >= 2)
				{
					curve.nrEnd = nrStartStraight;		// valid even if iNumStraightSegs <= 2
					curve.nEndDir = DIR_SET;				// dir will be cont if all in a curve
					curve.vtEndDir = vtStartStraightDir;
					curve.numSegs -= iNumStraightSegs-1;		// curve.numSegs is inc later but iNumStraightSegs is inc previously
					int iRemove = iNumStraightSegs-1;
					nodeSumsArray.RemoveAt(nodeSumsArray.GetSize()-iRemove, iRemove);
				}
				else		// must have got here with !bSegNextValid
				{
					curve.nEndDir = DIR_SET;				// dir will be cont if all in a curve
					curve.vtEndDir = segNext.vt;
				}
			}
			else if (nMagDirChange == DIR_CHANGE_LARGE)
			{
					// end here, DIR_FREE
			}
			else	// if (nMagDirChange == DIR_CHANGE_SAMLL)
			{
				if (iErrArcPrev <= ERROR_SMALL)
				{
					if (iErrArcNext <= ERROR_SMALL)
					{
							// end here, DIR_FREE		got here from bBreakCurve or iNumStraightSegs
					}
					else if (iErrArcPrevIfDirSet <= ERROR_SMALL)
					{
						curve.nEndDir = DIR_SET;		// only if segNext is 
						curve.vtEndDir = segNext.vt;		// set dir to next - or just to match
						bGotActualCurve = true;
						segNext.nType = ST_STRAIGHT;
					}
					else
					{
							// end here, DIR_FREE
					}
				}
				else if (iErrArcNext <= ERROR_SMALL)
				{
							// end here, DIR_FREE
				}
				else		// both iErrArc == ERROR_LARGE
				{
							// end here, DIR_FREE
				}

			}

			// found end of curve segments, fit curve
//			ASSERT(bGotActualCurve == (curve.numSegs >= 2 || (curve.numSegs == 1 && (curve.nStartDir == DIR_SET || curve.nEndDir == DIR_SET))));
			if (bGotActualCurve)
			{
				os.open("CurveNodes.txt", ios::out);
				int sizeArray = curveNodeArray.GetSize();
				for (int i = 0; i < sizeArray; i++)
				{
					if (i % 20 == 0)
						os << endl << SNodeSegProps::GetHeading();
					os << curveNodeArray[i];
				}
				os << endl;
				os.close();
				ndCurr.l = 0;

				ASSERT(curve.numSegs >= 2 || (curve.numSegs == 1 && (curve.nStartDir == DIR_SET || curve.nEndDir == DIR_SET)));	// check if curve applicable
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
		iNumStraightSegs = iNumStraightSegsPotential = 0;		// not in straight
		bInCurve = true;
		bGotActualCurve = false;	// set when a bend is encounted
		curve.nrStart = ndCurr.nr;
		curve.nStartDir = DIR_FREE;	// default
		curve.FitBreakArray.RemoveAll();

		// reset sums
		ndCurr.l = 0;
		ndCurr.magDirCngSum = 0;
		curveNodeArray.RemoveAll();
		nodeSumsArray.RemoveAll();
		ndCurrSums.nr = ndCurr.nr;
		ndCurrSums.l = ndCurr.l;
		ndCurrSums.magDirCngSum = ndCurr.magDirCngSum;
		nodeSumsArray.Add(ndCurrSums);

		if (nMagDirChange == DIR_CHANGE_NONE)
		{
			if (segPrev.bValid)
			{
				bInCurve = false;		// continue straight segments
				segNext.nType = ST_STRAIGHT;
			}
			else							// if prev seg was invalid (bezier curve) then allow a start
			{
				curve.nStartDir = DIR_SET;
				curve.vtStartDir = segPrev.vt;
			}
		}
		else if (nMagDirChange == DIR_CHANGE_LARGE)
		{
				// start here, DIR_FREE
		}
		else	// if (nMagDirChange == DIR_CHANGE_SMALL)
		{
			if (segNext.nType == ST_STRAIGHT)		// next seg has been used as a straight reference already
			{
				bInCurve = false;							// this bit should be changed ??
			}
			else if (iErrArcNext <= ERROR_SMALL)
			{
				if (iErrArcNextIfDirSet <= ERROR_SMALL && (segPrev.nType == ST_STRAIGHT || segPrev.nType == ST_BEZCONTROL))
				{
					// previous seg direction set - match direction
					curve.nStartDir = DIR_SET;
					curve.vtStartDir = segPrev.vt;
					bGotActualCurve = true;		// needed for single short seg between longer straight ones with small angles
				}
				else if (iErrArcPrev <= ERROR_SMALL && !bBreakCurve && segPrev.bValid)
				{
					// cusp at ndPrev then small changes - start ndPrev
					ASSERT(segPrev.nType != ST_STRAIGHT);
					ASSERT(ndPrev.nMagDirCng != DIR_CHANGE_NONE);
					curve.nrStart = ndPrev.nr;
					curve.numSegs++;				// cause started ndPrev
					bGotActualCurve = true;

					nodeSumsArray[0].nr = ndPrev.nr;
					ndCurr.l = segPrev.mag;
					ndCurr.magDirCngSum = 0.5*(ndPrev.magDirCng + ndCurr.magDirCng);
					ndCurrSums.nr = ndCurr.nr;
					ndCurrSums.l = ndCurr.l;
					ndCurrSums.magDirCngSum = ndCurr.magDirCngSum;
					nodeSumsArray.Add(ndCurrSums);
				}
				else
				{
					// start here, DIR_FREE
				}
			}
			else if (iErrArcPrev <= ERROR_SMALL)
			{
					// start here, DIR_FREE
			}
			else
			{
					// start here, DIR_FREE
			}

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
	ASSERT(ci.numSegs >= 2 || ci.nStartDir == DIR_SET || ci.nEndDir == DIR_SET)	;	// check if curve applicable
// store list of curve info
	m_CurveInfoArray.Add(ci);


	// Add all nodes from origional array to m_FittedBezierArray up to and including ci.nrStart
	NODEREF nr = m_nrLastAddedToFittedArray;		// starts at -1
	while (++nr <= ci.nrStart)
		m_FittedBezierArray.Add(*GetPathNode(nr));

	// Add any command nodes directly after start node and check for none later in curve section - or curve should be broken(not handled yet!)
	int iSeg = 0;
	for (; nr <= ci.nrEnd; nr++)
		if (GetPathNode(nr)->IsPoint())		// is point node of a segment
			iSeg++;
		else if (GetPathNode(nr)->IsCommand())
			if (iSeg == 0)			// OK to add if just after first node!
				m_FittedBezierArray.Add(*GetPathNode(nr));
			else
			{
				TRACE("Have to insert command node in smoothed path!!! in CPathNCIDoc::FindCurve()\n");
				ASSERT(0);		// end current fit and add this node
			}
	ASSERT(iSeg > 0);
	ASSERT(nr == ci.nrEnd + 1);
	m_nrLastAddedToFittedArray = ci.nrEnd;			// will be after fitted curve added



	// find number of constraints
	int numEndDirsSet = 0;
	if (ci.nStartDir == DIR_SET)
		numEndDirsSet++;
	if (ci.nEndDir == DIR_SET)
		numEndDirsSet++;
	int numPoints = ci.numSegs + 1;
	ASSERT(numPoints + numEndDirsSet >= 3);

	// check curvature of ajoining segments if direction is set to it
	bool bStartStraight, bEndStraight;
	nr = ci.nrStart;
	CPathNode* pNode = GetPrevNode(nr);
	// if prev seg is part of a fitted curve then must be disjoin anyway, curving if not end to end segment
	bStartStraight = (ci.nStartDir == DIR_SET) && pNode && (pNode->type & PNTF_END);
	nr = ci.nrEnd;
	pNode = GetNextNode(nr);
	// if next seg is part of a fitted curve then must be disjoin anyway, curving if not end to end segment
	bEndStraight = (ci.nEndDir == DIR_SET) && pNode && (pNode->type & PNTF_END);

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
		m_pCurveFitter->FitCubicsToCurve(ci);
	}
	else if (numPoints == 4)
	{
		if (numEndDirsSet >= 1)
		{
			// OK for 3D
			bStore1Seg = false;
			m_pCurveFitter->FitCubicsToCurve(ci);
		}
		else		// fit exact cubic
		{
			// OK for 3D
			//ASSERT(0);		// change to fit to nodes and seg mid nodes - exact fit can give bad results!
			LOGMESSAGE("Improve exact fit to 4 points!");
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
		if (ci.nStartDir == DIR_SET)
			if (bStartStraight)
				FitBezierToNoCurveArcDir(*pndInit, ci.vtStartDir, *pndFinal, ndCi, ndCf);	// all CVector base
			else 
				FitBezierToArcDir(*pndInit, ci.vtStartDir, *pndFinal, ndCi, ndCf);	// all CVector base
		else if (ci.nEndDir == DIR_SET)
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


		// find s for mid point if s = [0,1] - vtChord parallel component moves linearly from init to final
		//	double s = dot(vtChord, *pnd1 - *pndInit) / magChordSq;		// can give s near 0 or 1
		double magSeg1 = (*pnd1 - *pndInit).Mag();
		double magSeg2 = (*pndFinal - *pnd1).Mag();
		double magSegs = magSeg1 + magSeg2;
		double s = magSeg1 / magSegs;
		double sp2 = s*s;
		double sp3 = s*sp2;
		ASSERT(s > 0 && s < 1);


//		CVector vtChord = *pndFinal - *pndInit;
//		double magChordSq = vtChord.MagSq();
//		double CiVectScale = magChordSq / (3*dot(vtChord, *pvtDir));	// or could use magSeg1 + magSeg2 instead of magChord

		bool bStartDirSet = (ci.nStartDir == DIR_SET);

		//	val = Ni(1-3s+3s^2-s^3) + 3Ci(s-2s^2+s^3) + 3Cf(s^2-s^3) + Nf(s^3)
		if (bStartDirSet)
		{
			CVector& vtDir = ci.vtStartDir;
			double ContvtDirScale = magSegs / (3 * vtDir.Mag());		// use magSegs as full s range (~length of result curve)
			ndCi = *pndInit + vtDir * ContvtDirScale;
			ndCf = (*pnd1 -  *pndInit*(1+3*(sp2-s)-sp3) - ndCi*(3*(s-2*sp2+sp3)) - *pndFinal*(sp3)) / (3*(sp2-sp3));
		}
		else	// if (bEndDirSet)
		{
			CVector& vtDir = ci.vtEndDir;
			double ContvtDirScale = magSegs / (3 * vtDir.Mag());		// use magSegs as full s range (~length of result curve)
			ndCf = *pndFinal - vtDir * ContvtDirScale;
			ndCi = (*pnd1 -  *pndInit*(1+3*(sp2-s)-sp3) - ndCf*(3*(sp2-sp3)) - *pndFinal*(sp3)) / (3*(s-2*sp2+sp3));
		}

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







