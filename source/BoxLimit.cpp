// BoxLimit.cpp: implementation of the CBoxLimit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BoxLimit.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBoxLimit::CBoxLimit()
{
	bSet = false;
}

CBoxLimit::~CBoxLimit()
{

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


bool CBoxLimit::SolveMaxMaOrig()
{
/*	Bounding box limit
	Finds the maximum positive scalar 'Ma' such that
	Ma.vtA + Mb.vtB is on the surface of the bounding box +/-<Box>
	vtBound contains (the sign of) the limiting bounds
	vtA & vtB can be any non-parallel vectors (don't need to be perpendicular)
*/

// Ma.<A> + Mb.<B> <= +/-<Max>

// Find corner to maximise Ma
// Method 1: Find corner with max (vtA component perpendicular to vtB) using the plane with
//				a normal as the vtA component perpendicular to vtB (plane normal in the vtA, vtB plane)
#ifdef _DEBUG
	for (int iax = 0; iax < 3; iax++)		// check vtBoxMin are <= vtBoxMax elements
		ASSERT(vtBoxMin[iax] <= vtBoxMax[iax]);
#endif	// _DEBUG
	
//	Method 1 for finding limit edge (one attached to vtCorner)
	CVector vtCorner;
	CVector vtAxB = cross(vtA, vtB);				// vector normal to vtA x vtB plane
	CVector vtAperpB = cross(vtB, vtAxB);					// vtAperpB   is vtA component perpendicular to vtB and in the plane of vtA & vtB
//	vtAperpB = cross(vtB, cross(vtA, vtB));	// direction should be aligned with vtB, if vtA & vtB are perp will be dir of vtA
	double AComp = dot(vtAperpB, vtA);			// vtA component in vtAperpB
	int bound;
	if (AComp < 0)
	{
		vtAperpB.Neg();
		AComp = -AComp;
		ASSERT(0);		// vtBperp should be aligned after cross(cross())
		return false;
	}
	if (AComp == 0)
	{
		vtCorner = 0.0;		// maybe set to plane that vtA hits (if vtB is 0)
		ASSERT(0);		// not handled properly
		return false;
	}
	else
	{
		// find Box corner with most pos vtBperp component
		for (int ax = 0; ax < 3; ax++)
		{
			if (vtAperpB[ax] == 0)
			{
				bound = (vtB[ax] > 0) ? -1 : 1;	// could set bound to 0 if (vtB[ax] == 0)
				if (bMaxMbBound)
					bound = -bound;
			}
			else
				bound = (vtAperpB[ax] > 0) ? 1 : -1;

			vtBound[ax] = (char)bound;
			vtCorner[ax] = (bound > 0) ? vtBoxMax[ax] : vtBoxMin[ax];
		}

//		MaMax = dot(vtAperpB, vtCorner) / AComp;		// this value is >= the true solution value
	}
//	vtBoundValue = vtCorner;

// Method 3 - after finding vtCorner
// check intersection of vtA x vtB plane and each edge attached to corner

	Ma = -1;		// anything > 0 will be larger!
	axEdgeDir = -1;

	for (int ax = 0; ax < 3; ax++)	// check each edge attached to corner for intersection with plane
	{
		if (vtAxB[ax] == 0)
		{
			arMa[ax] = 0;
			arvtBound[ax] = 0;
			continue;
		}
		CVector vtEdge = vtCorner;
		vtEdge[ax] = 0;
		double d = dot(vtEdge, vtAxB);		// vtAxB is coefficents for plane equation
		double edgeVal = - d / vtAxB[ax];	// point along edge where plane intersects
		vtEdge[ax] = edgeVal;
		double MaEdge = dot(vtEdge, vtAperpB) / AComp;		// get vtAperpB component of vtEdge

		arMa[ax] = MaEdge;						// bl.vtMa is a vector of the Ma's of each axis used to determine when a change of edge occurs
		arvtBound[ax] = vtBound;
		arvtBound[ax][ax] = 0;
		ASSERT(MaEdge > 0);
		if (MaEdge > Ma && edgeVal >= vtBoxMin[ax] && edgeVal <= vtBoxMax[ax])
		{
			axEdgeDir = ax;
			Ma = MaEdge;
			CVector vtTotalB = vtEdge - (MaEdge * vtA);
			Mb = dot(vtTotalB, vtB) / vtB.MagSq();		// is |vtTotalB| / |vtB| with sign if vector are aligned!
			ASSERT((vtEdge - Ma * vtA - Mb * vtB).Mag() <= 1e-8);
		}
	}
	ASSERT(axEdgeDir != -1);
	ASSERT(Ma > 0);
	vtBound[axEdgeDir] = 0;	// used by other functions (CPathTracker::GetAbreakaway) to indicate bounding edge
	return true;

}

bool CBoxLimit::SolveMaxMa()
{
/*	Bounding box limit
	Finds the maximum positive scalar 'Ma' such that
	Ma.vtA + Mb.vtB is on the surface of the bounding box +/-<Box>
	vtBound contains (the sign of) the limiting bounds
	vtA & vtB can be any non-parallel vectors (don't need to be perpendicular)
*/
/*
	Find best solution to 2x2 simultaneous equation for edges in each axis direction

	X axis solution:
	[ vtA.y  vtB.y ] * Ma = vtCorner.y
	[ vtA.z  vtB.z ]   Mb   vtCorner.z
	=>
	Ma = 1/det * [ vtB.z  -vtB.y ] * vtCorner.y
	Mb           [-vtA.z   vtA.y ]   vtCorner.z
	det = vtA.y*vtB.z - vtA.z*vtB.y = vtAxB.x

	Y axis solution:
	[ vtA.z  vtB.z ] * Ma = vtCorner.z
	[ vtA.x  vtB.x ]   Mb   vtCorner.x
	=>
	Ma = 1/det * [ vtB.x  -vtB.z ] * vtCorner.z
	Mb           [-vtA.x   vtA.z ]   vtCorner.x
	det = vtA.z*vtB.x - vtA.x*vtB.z = vtAxB.y

	Z axis solution:
	[ vtA.x  vtB.x ] * Ma = vtCorner.x
	[ vtA.y  vtB.y ]   Mb   vtCorner.y
	=>
	Ma = 1/det * [ vtB.y  -vtB.x ] * vtCorner.x
	Mb           [-vtA.y   vtA.x ]   vtCorner.y
	det = vtA.x*vtB.y - vtA.y*vtB.x = vtAxB.z

	for max positive Ma:
	sign(vtCorner.y) = sign( vtB.z * det)
	sign(vtCorner.z) = sign(-vtB.y * det)

*/
#ifdef _DEBUG
	for (int iax = 0; iax < 3; iax++)		// check vtBoxMin are <= vtBoxMax elements
		ASSERT(vtBoxMin[iax] <= vtBoxMax[iax]);
#endif



	CVector vtAxB = cross(vtA, vtB);				// vector normal to vtA x vtB plane
	double magAxBVerySmall = vtAxB.SumAbs() * 1e-8;		// approx magintude * 1e-8
	CVector vtCorner;
	TVector<char> vtBnd;			// bound
	Ma = -1;		// anything > 0 will be larger!
	axEdgeDir = -1;

	char signMbDef = char(bMaxMbBound ? 1 : -1);

	for (int ax = 0; ax < 3; ax++)	// check each edge attached to corner for intersection with plane
	{
		if (fabs(vtAxB[ax]) <= magAxBVerySmall)
		{
			arMa[ax] = 0;
			arEdgeVal[ax] = 0;
			arvtBound[ax] = 0;
			continue;
		}
		int axP1 = (ax+1) % 3;
		int axP2 = (ax+2) % 3;

//		vtCoef.Set(vtB[axP1], vtB[axP2]);
//		vtCoef.Rotate90CW();			// These two lines are 2x2 matrix inverse!
//		vtCoef /= vtAxB[ax];

		// vtAxB[ax] does not generally change sign
		double coefAP1 =  vtB[axP2] / vtAxB[ax];	// = vtA[axP1] / vtAP12.magSq() if vtA,vtB are perpendicular and in X,Y or Z plane
		double coefAP2 = -vtB[axP1] / vtAxB[ax];	// = vtA[axP2] / vtAP12.magSq() if vtA,vtB are perpendicular and in X,Y or Z plane
		ASSERT(fabs(dot(vtA, vtB)) <= 1e-8);		// check if perp - don't need to be!


/*		char signP1 = (char)sign(coefAP1);
		char signP2 = (char)sign(coefAP2);
		// check if vtBnd is always same sign - it can change!
		if (signP1 != 0 && vtBnd[axP1] != 0)
			ASSERT(signP1 == vtBnd[axP1]);
		if (signP2 != 0 && vtBnd[axP2] != 0)
			ASSERT(signP2 == vtBnd[axP2]);
*/
		vtBnd[axP1] = (char)sign(coefAP1);// sign of vtBnd can change between axis!!!
		vtBnd[axP2] = (char)sign(coefAP2);

		if (vtBnd[axP1] == 0)		// if coef Ma == 0 use min/max Mb bound
			vtBnd[axP1] = char((vtB[axP1] >= 0) ? signMbDef : -signMbDef);		// could set bound to 0 if (vtB[ax] == 0)
		if (vtBnd[axP2] == 0)		// if coef Ma == 0 use min/max Mb bound
			vtBnd[axP2] = char((vtB[axP2] >= 0) ? signMbDef : -signMbDef);		// could set bound to 0 if (vtB[ax] == 0)

		vtCorner[axP1] = (vtBnd[axP1] > 0) ? vtBoxMax[axP1] : vtBoxMin[axP1];
		vtCorner[axP2] = (vtBnd[axP2] > 0) ? vtBoxMax[axP2] : vtBoxMin[axP2];
		double MaEdge = coefAP1 * vtCorner[axP1] + coefAP2 * vtCorner[axP2];		// = dot(vtCoef, vtCorner) = dot(vtB.Rotate90CW(),  vtCorner) / vtAxB[ax]
		double MbEdge = (-vtA[axP2] * vtCorner[axP1] + vtA[axP1] * vtCorner[axP2]) / vtAxB[ax];			// = dot(vtA.Rotate90CCW(), vtCorner) / vtAxB[ax]
		double edgeVal = MaEdge * vtA[ax] + MbEdge * vtB[ax];

		arMa[ax] = MaEdge;
		arEdgeVal[ax] = edgeVal;
		vtBnd[ax] = 0;			// indicates edge used
		arvtBound[ax] = vtBnd;
		ASSERT(MaEdge > 0);
		if (MaEdge > Ma && edgeVal >= vtBoxMin[ax] && edgeVal <= vtBoxMax[ax])
		{
			axEdgeDir = ax;
			Ma = MaEdge;
			Mb = MbEdge;
			vtBound = vtBnd;
//			vtCorner[ax] = 0;
			vtCorner[ax] = edgeVal;		// to cross check!!
			vtBoundValue = vtCorner;
			ASSERT((vtCorner - MaEdge * vtA - MbEdge * vtB).MagSq() <= 1e-14);
		}
	}

	ASSERT(axEdgeDir != -1);
	ASSERT(Ma > 0);
	//vtBound[axBest] = 0;		// used by other functions (CPathTracker::GetAbreakaway) to indicate bounding edge
	// check only one bound is zero
	ASSERT((vtBound[0] == 0) + (vtBound[1] == 0) + (vtBound[2] == 0) == 1);
	return true;
}



/*
//	Method 2 for finding limit edge (edge with minimum Ma)	
	bool bMaMbSet = false;
	int axEdge = -1;
	CVector vtApl(0), vtBpl(0);		// used as 2D vectors, z stays 0
	CVector vtCornerpl(0);				// used as 2D vectors, z stays 0
	CVector vtMaMb(0), vtMaMbpl(0);	// used as 2D vectors, z stays 0
	CMatrix mxEqu(2, 2);		// (rows, cols) format
// Looking along each axis in turn, find edge with max vtA componemt
	for (int axView = 0; axView < 3; axView++)
	{
//		take only 2 axis (planar view)
		for (int a2=0, a3=0; a2 < 2; a2++, a3++)
		{
			if (a3 == axView) a3++;
			vtApl[a2] = vtA[a3];
			vtBpl[a2] = vtB[a3];
			vtCornerpl[a2] = vtCorner[a3];
		}

// Both Methods use this part
//		CornerComp = dot(vtAperpB, vtCorner);
//		Ma1 = CornerComp / AComp;		// if AComp != 0

		mxEqu.SetColumn(0, CMatrixWrap(vtApl, 2));
		mxEqu.SetColumn(1, CMatrixWrap(vtBpl, 2));
		if (mxEqu.Invert())
			vtMaMbpl = mxEqu * CMatrixWrap(vtCornerpl, 2);
		else
		{
//			vtMaMbpl = 0;			// no solution for this view
			vtMa[axView] = 0;
			continue;				// no solution for this view
		}
		vtMa[axView] = vtMaMbpl[0];

		if (vtMaMbpl[0] <= MaMax && (!bMaMbSet || (vtMaMbpl[0] > vtMaMb[0])))	// if within box edge AND (not set OR Ma is greater than previous Ma's)
		{

		//use MaMax if no other options
//			if (bMaMbSet && (vtMaMbpl[0] == vtMaMb[0]) && (vtMaMbpl[1] == vtMaMb[1]))	// equal to another axis, must be corner
//				axEdge = -2;
//			else
			axEdge = axView;
			vtMaMb = vtMaMbpl;
			bMaMbSet = true;
		}
	}	// for (int axView = 0; axView < 3; axView++)

	ASSERT(bMaMbSet);			// Couldn't find limit, probably V = 0
	vtBound[axEdge] = 0;
	Ma = vtMaMb[0];
	Mb = vtMaMb[1];
	return bMaMbSet;
*/


/*
Bound = 2 * (A >= 0) - 1;
signMax = Bound .* abs(Max);
signMax = signMax(:);
for i = 1:3
	sel = [1:3];
	sel(i) = [];
	Afact = A(sel);
	Bfact = B(sel);
	SimEqu = [Afact(:) Bfact(:)];
	if det(SimEqu) ~= 0
		MaMb = [MaMb [(inv(SimEqu) * signMax(sel)); i]];
	end
end

if MaMb == []
	error('Couldn''t find a limit, probably V = 0')
end

[Ma i] = min(MaMb(1,:));

// Ma = MaMb(1, i);
Mb = MaMb(2, i);

if sum(Ma == MaMb(1,:)) == 1	// If >=2 equal minimums, all axis are at bounds
	Bound(MaMb(3, i)) = 0;
else
	disp('Careful: Max normal accel is at corner of BoxLimit')
end


// check
res = [A(:) B(:)] * MaMb(1:2,i);
if abs(res) > Max(:) + 1e-8
	res
	error('Calculated result is out of box bounds')
end


*/


