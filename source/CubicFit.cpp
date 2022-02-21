// CubicFit.cpp: implementation of the CCubicFit class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"

#include <math.h>

//#include "cnccontrol.h"
#include "VectMath.h"

#include "CubicFit.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/*
	double x, y, xPow;

	Num++;
	SumYXp[0] += y;
	SumXp[1]  += (xPow = x);
	SumYXp[1] += xPow * y;
	SumXp[2]  += (xPow *= x);
	SumYXp[2] += xPow * y;
	SumXp[3]  += (xPow *= x);
	SumYXp[3] += xPow * y;
	SumXp[4]  += (xPow *= x);
	SumXp[5]  += (xPow *= x);
	SumXp[6]  += (xPow * x);
	SumY2 += y * y;		// needed to calc residual error (sum of squares)

------------

	Best cubic fit
	Y = a0 + a1.x + a2.x^2 + a3.x^3

	Sum of residuals squared = Sr
		Sr uses all the sums S(x^0..6) plus S(y^2)
	Sr = Sum((y - Y)^2)		where y is actual value and Y is cubic value

	Sr = S(y^2) + a0^2.S(x^0) + a1^2.S(x^2) + a2^2.S(x^4) + a3^2.S(x^6)
		+ 2{-a0.S(x^0.y) - a1.S(x^1.y) - a2.S(x^2.y) - a3.S(x^3.y) }
		+ 2{ a0.a1.S(x^1) + a0.a2.S(x^2) + (a0.a3+a1.a2).S(x^3) + a1.a3.S(x^4) + a2.a3.S(x^5) }

		S(...) = sum(...)
		note S(x^0) = N	number of points

	dSr/da0     [ S(x^0)  S(x^1)  S(x^2)  S(x^3) ]   a0     S(x^0.y)
	dSr/da1 = 2*[ S(x^1)  S(x^2)  S(x^3)  S(x^4) ] * a1 - 2*S(x^1.y)
	dSr/da2     [ S(x^2)  S(x^3)  S(x^4)  S(x^5) ]   a2     S(x^2.y)
	dSr/da3     [ S(x^3)  S(x^4)  S(x^5)  S(x^6) ]   a3     S(x^3.y)

	For minimum Sr all dSr/da are 0.  Therefor:
	[ S(x^0)  S(x^1)  S(x^2)  S(x^3) ]   a0   S(x^0.y)
	[ S(x^1)  S(x^2)  S(x^3)  S(x^4) ] * a1 = S(x^1.y)
	[ S(x^2)  S(x^3)  S(x^4)  S(x^5) ]   a2   S(x^2.y)
	[ S(x^3)  S(x^4)  S(x^5)  S(x^6) ]   a3   S(x^3.y)

	--> vtdSra = 2 * mxS * vta  -  2 * vtSy
	->  mxS * vta  =  vtSy

-------------------------
	With end boundary conditions


	Initial Pos and Vel fixed and initial x = 0:
	a0 = P0 & a1 = V0

	[ S(x^4)  S(x^5) ] * a2  =  S(x^2.y) - [ S(x^2)  S(x^3) ] * a0
	[ S(x^5)  S(x^6) ]   a3     S(x^3.y)   [ S(x^3)  S(x^4) ]   a1

	If initial x != 0, but x = xa
	[ 1        xa       xa^2     xa^3 ]   a0    Pos(xa)
	[ 0        1       2xa      3xa^2 ] * a1 =  Vel(xa)
	                                      a2
	                                      a3
	mxBCs * vta  =  vtBCs

	eliminate a0, a1 (solve for these!)
	step 1:
	[ 1        0       -xa^2   -2xa^3 ]   a0    Pos(xa) - xa*Vel(xa)		//  -xa * row2
	[ 0        1       2xa      3xa^2 ] * a1 =  Vel(xa)
	                                      a2
	                                      a3
	dSr/da2 = dSra/da2 + dSra/da0 * da0/da2 + dSra/da1 * da1/da2 = 0		// from all dependent params
	-->
	dSr/da2   [ da0/da2  da1/da2    1    0 ]   dSra/da0     0
	dSr/da3 = [ da0/da3  da1/da3    0    1 ] * dSra/da1  =  0
	                                           dSra/da2
	                                           dSra/da3

	-> vtdSrRed = mxda * vtdSra = 0

	          [  xa^2   -2xa     1    0 ]
	-> mxda = [ 2xa^3   -3xa^2   0    1 ]
	     
	  mxda * vtdSra = 0
	& vtdSra = 2 * mxS * vta  -  2 * vtSy

	mxda * mxS * vta  =  mxda * vtSy		// 2 equ's		(note: mxda doesn't have an inverse - not square!)
	     mxBCs * vta  =  vtBCs				// 2 equ's







--------------------------
	Optimise knot of two polys with continuous pos and vel
	  P(a0 a1 a2 a3)    P(b0 b1 b2 b3)
	|-----------------|-----------------|
	xa						xb

	assuming x starts at 0 for poly B !!:
  b0 = a0 + a1.xb + a2.xb^2 + a3.xb^3		continuous pos
  b1 = a1 + 2a2.xb + 3a3.xb^2					continuous vel

  for any other starting x for poly B:
  0 = a0-b0 + (a1-b1).xb + (a2-b2).xb^2 + (a3-b3).xb^3		continuous pos
  0 = (a1-b1) + 2(a2-b2).xb + 3(a3-b3).xb^2						continuous vel


		First set of sums is over poly A
	Sr = S(y^2) + a0^2.S(x^0) + a1^2.S(x^2) + a2^2.S(x^4) + a3^2.S(x^6)
		+ 2{-a0.S(x^0.y) - a1.S(x^1.y) - a2.S(x^2.y) - a3.S(x^3.y) }
		+ 2{ a0.a1.S(x^1) + a0.a2.S(x^2) + (a0.a3+a1.a2).S(x^3) + a1.a3.S(x^4) + a2.a3.S(x^5) }

		Following sums are over poly B
		+ S(y^2) + b0^2.S(x^0) + b1^2.S(x^2) + b2^2.S(x^4) + b3^2.S(x^6)
		+ 2{-b0.S(x^0.y) - b1.S(x^1.y) - b2.S(x^2.y) - b3.S(x^3.y) }
		+ 2{ b0.b1.S(x^1) + b0.b2.S(x^2) + (b0.b3+b1.b2).S(x^3) + b1.b3.S(x^4) + b2.b3.S(x^5) }





	Initial pos & vel of poly a is set
	pos & vel between polys a & b is continous

	Find minimum of Sr = Sra + Srb
	dSra/da0   [ Sa(x^0)  Sa(x^1)  Sa(x^2)  Sa(x^3) ]   a0   Sa(x^0.y)
	dSra/da1 =2[ Sa(x^1)  Sa(x^2)  Sa(x^3)  Sa(x^4) ] * a1 -2Sa(x^1.y)
	dSra/da2   [ Sa(x^2)  Sa(x^3)  Sa(x^4)  Sa(x^5) ]   a2   Sa(x^2.y)
	dSra/da3   [ Sa(x^3)  Sa(x^4)  Sa(x^5)  Sa(x^6) ]   a3   Sa(x^3.y)

	dSrb/db0   [ Sb(x^0)  Sb(x^1)  Sb(x^2)  Sb(x^3) ]   b0   Sb(x^0.y)
	dSrb/db1 =2[ Sb(x^1)  Sb(x^2)  Sb(x^3)  Sb(x^4) ] * b1 -2Sb(x^1.y)
	dSrb/db2   [ Sb(x^2)  Sb(x^3)  Sb(x^4)  Sb(x^5) ]   b2   Sb(x^2.y)
	dSrb/db3   [ Sb(x^3)  Sb(x^4)  Sb(x^5)  Sb(x^6) ]   b3   Sb(x^3.y)

	-->
	dSra/da0   [ Sa(x^0)  Sa(x^1)  Sa(x^2)  Sa(x^3)  0        0        0        0       ]   a0   Sa(x^0.y)
	dSra/da1 =2[ Sa(x^1)  Sa(x^2)  Sa(x^3)  Sa(x^4)  0        0        0        0       ] * a1 -2Sa(x^1.y)
	dSra/da2   [ Sa(x^2)  Sa(x^3)  Sa(x^4)  Sa(x^5)  0        0        0        0       ]   a2   Sa(x^2.y)
	dSra/da3   [ Sa(x^3)  Sa(x^4)  Sa(x^5)  Sa(x^6)  0        0        0        0       ]   a3   Sa(x^3.y)
	dSrb/db0   [ 0        0        0        0        Sb(x^0)  Sb(x^1)  Sb(x^2)  Sb(x^3) ]   b0   Sb(x^0.y)
	dSrb/db1   [ 0        0        0        0        Sb(x^1)  Sb(x^2)  Sb(x^3)  Sb(x^4) ]   b1   Sb(x^1.y)
	dSrb/db2   [ 0        0        0        0        Sb(x^2)  Sb(x^3)  Sb(x^4)  Sb(x^5) ]   b2   Sb(x^2.y)
	dSrb/db3   [ 0        0        0        0        Sb(x^3)  Sb(x^4)  Sb(x^5)  Sb(x^6) ]   b3   Sb(x^3.y)
	-> vtdSrab = 2 * mxSab * vtab  -  2 * vtSaby

	At xb:
	Apos(xb)   [ 1  xb  xb^2  xb^3  ]   a0
	Avel(xb) = [ 0   1  2xb   3xb^2 ] * a1
	                                    a2
	                                    a3

	Bpos(xb)   [ 1  xb  xb^2  xb^3  ]   b0
	Bvel(xb) = [ 0   1  2xb   3xb^2 ] * b1
	                                    b2
	                                    b3


boundary condition equations:
	[ 1        xb       xb^2     xb^3    -1       -xb      -xb^2    -xb^3  ]   a..    0				// Pos(xb) cont.
	[ 0        1       2xb      3xb^2     0       -1      -2xb     -3xb^2  ] * a.. =  0				// Vel(xb) cont.
	[ 1        xa       xa^2     xa^3     0        0        0        0     ]   b..    Pos(xa)		// Pos(xa)
	[ 0        1       2xa      3xa^2     0        0        0        0     ]   b..    Vel(xa)		// Vel(xa)
	-> mxBCs * vtab = vtBCs		// 4 equ's

	eliminate a0, a1, b0, b1 (solve for these!)
	step 1:
	[ 1        0 xb^2-2xa*xb xb^3-3xb*xa^2 -1       -xb      -xb^2    -xb^3    ]   a..    0 - xb*Vel(xa)				// -xb * row4
	[ 0        0   2(xb-xa)  3(xb^2-xa^2)   0       -1      -2xb     -3xb^2    ] * a.. =  0 - Vel(xa)					//  -1 * row4
	[ 1        0       -xa^2   -2xa^3       0        0        0        0       ]   b..    Pos(xa) - xa*Vel(xa)		// -xa * row4
	[ 0        1       2xa      3xa^2       0        0        0        0       ]   b..    Vel(xa)
	step 2:
	[ 0        0   (xb-xa)^2 xb^3-3xb*xa^2+2xa^3 -1       -xb      -xb^2    -xb^3    ]   a..    (xa-xb)*Vel(xa) - Pos(xa)	//  -1 * row3
	[ 0        0   2(xb-xa)   3(xb^2-xa^2)        0       -1      -2xb     -3xb^2    ] * a.. =  -Vel(xa)
	[ 1        0       -xa^2   -2xa^3             0        0        0        0       ]   b..    Pos(xa) - xa*Vel(xa)
	[ 0        1       2xa      3xa^2             0        0        0        0       ]   b..    Vel(xa)
	step 3:
	[ 0        0   xa^2-xb^2  2xa^3-2xb^3    -1        0       xb^2     2xb^3    ]   a..    xa*Vel(xa) - Pos(xa)	// -xb * row2
	[ 0        0   2xb-2xa    3xb^2-3xa^2     0       -1      -2xb     -3xb^2    ] * a.. =  -Vel(xa)
	[ 1        0       -xa^2   -2xa^3         0        0        0        0       ]   b..    Pos(xa) - xa*Vel(xa)
	[ 0        1       2xa      3xa^2         0        0        0        0       ]   b..    Vel(xa)

	solution:
	a0   [      xa^2        2xa^3     0        0         1     -xa ] * [ a2  a3  b2  b3  Pos(xa)  Vel(xa) ]'
	a1 = [    -2xa         -3xa^2     0        0         0      1  ]
	b0   [ xa^2-xb^2  2xa^3-2xb^3     xb^2    2xb^3      1     -xa ]
	b1   [ 2xb-2xa    3xb^2-3xa^2   -2xb     -3xb^2      0      1  ]



	dSr/da2 = dSra/da2 + dSra/da0 * da0/da2 + dSra/da1 * da1/da2 + dSrb/db0 * db0/da2 + dSrb/db1 * db1/da2 = 0		// from all dependent params
	-->
	dSr/da2   [ da0/da2  da1/da2    1    0   db0/da2  db1/da2   0    0 ]   dSra/da0     0
	dSr/da3 = [ da0/da3  da1/da3    0    1   db0/da3  db1/da3   0    0 ] * dSra/da1  =  0
	dSr/db2   [ da0/db2  da1/db2    0    0   db0/db2  db1/db2   1    0 ]   dSra/da2     0
	dSr/db3   [ da0/db3  da1/db3    0    0   db0/db3  db1/db3   0    1 ]   dSra/da3     0
	                                                                       dSrb/db0
	                                                                       dSrb/db1
	                                                                       dSrb/db2
	                                                                       dSrb/db3
	-> vtdSrRed = mxdab * vtdSrab = 0

	           [  xa^2   -2xa     1    0     xa^2-xb^2     2xb-2xa     0    0 ]
	-> mxdab = [ 2xa^3   -3xa^2   0    1   2xa^3-2xb^3   3xb^2-3xa^2   0    0 ]
	           [  0        0      0    0          xb^2        -2xb     1    0 ]
	           [  0        0      0    0         2xb^3        -3xb^2   0    1 ]


	  mxdab * vtdSrab = 0
	& vtdSrab = 2*mxSab * vtab  -  2*vtSaby

	mxdab * mxSab * vtab  =  mxdab * vtSaby		// 4 equ's		(note: mxdab doesn't have an inverse - not square!)
	        mxBCs * vtab  =  vtBCs					// 4 equ's

	------------------------------

	vtab = [ a0  a1  a2  a3  b0  b1  b2  b3 ]'

	        [  xa^2   -2xa     1    0     xa^2-xb^2     2xb-2xa     0    0 ]
	mxdab = [ 2xa^3   -3xa^2   0    1   2xa^3-2xb^3   3xb^2-3xa^2   0    0 ]
	        [  0        0      0    0          xb^2        -2xb     1    0 ]
	        [  0        0      0    0         2xb^3        -3xb^2   0    1 ]

	        [ Sa(x^0)  Sa(x^1)  Sa(x^2)  Sa(x^3)  0        0        0        0       ]
	mxSab = [ Sa(x^1)  Sa(x^2)  Sa(x^3)  Sa(x^4)  0        0        0        0       ]
	        [ Sa(x^2)  Sa(x^3)  Sa(x^4)  Sa(x^5)  0        0        0        0       ]
	        [ Sa(x^3)  Sa(x^4)  Sa(x^5)  Sa(x^6)  0        0        0        0       ]
	        [ 0        0        0        0        Sb(x^0)  Sb(x^1)  Sb(x^2)  Sb(x^3) ]
	        [ 0        0        0        0        Sb(x^1)  Sb(x^2)  Sb(x^3)  Sb(x^4) ]
	        [ 0        0        0        0        Sb(x^2)  Sb(x^3)  Sb(x^4)  Sb(x^5) ]
	        [ 0        0        0        0        Sb(x^3)  Sb(x^4)  Sb(x^5)  Sb(x^6) ]

	         [ Sa(x^0.y) ]
	vtSaby = [ Sa(x^1.y) ]
	         [ Sa(x^2.y) ]
	         [ Sa(x^3.y) ]
	         [ Sb(x^0.y) ]
	         [ Sb(x^1.y) ]
	         [ Sb(x^2.y) ]
	         [ Sb(x^3.y) ]

	        [ 1        xb       xb^2     xb^3    -1       -xb      -xb^2    -xb^3 ]
	mxBCs = [ 0        1       2xb      3xb^2     0       -1      -2xb     -3xb^2 ]
	        [ 1        xa       xa^2     xa^3     0        0        0        0    ]
	        [ 0        1       2xa      3xa^2     0        0        0        0    ]

	        [     0   ]
	vtBCs = [     0   ]
	        [ Pos(xa) ]
	        [ Vel(xa) ]

	----------------------------------



  




**************************************

	a0 + a1.xb + a2.xb^2 + a3.xb^3 = val(xb)
	a0 = val(xb) - a1.xb - a2.xb^2 - a3.xb^3

	da1/da0 = -1/xb
	da2/da0 = -1/xb^2
	da3/da0 = -1/xb^3

	da0/da1 = -xb
	da0/da2 = -xb^2
	da0/da3 = -xb^3

	dSr'/da1 = dSr/da1 + dSr/da0 * da0/da1
	eliminate a0:
	dSr/da0   [ S(x^0)  S(x^1)  S(x^2)  S(x^3) ]   a0   S(x^0.y)
	dSr/da1 = [ S(x^1)  S(x^2)  S(x^3)  S(x^4) ] * a1 - S(x^1.y)
	dSr/da2   [ S(x^2)  S(x^3)  S(x^4)  S(x^5) ]   a2   S(x^2.y)
	dSr/da3   [ S(x^3)  S(x^4)  S(x^5)  S(x^6) ]   a3   S(x^3.y)

	  [         0            0            0            0   ]   a0
	- [ S(x^0).xb    S(x^1).xb    S(x^2).xb    S(x^3).xb   ] * a1
	  [ S(x^0).xb^2  S(x^1).xb^2  S(x^2).xb^2  S(x^3).xb^2 ]   a2
	  [ S(x^0).xb^3  S(x^1).xb^3  S(x^2).xb^3  S(x^3).xb^3 ]   a3

  
	 
	Sr = S(y^2) + (val - a1.xb - a2.xb^2 - a3.xb^3)^2.S(x^0) + a1^2.S(x^2) + a2^2.S(x^4) + a3^2.S(x^6)
		+ 2{-(val - a1.xb - a2.xb^2 - a3.xb^3).S(x^0.y) - a1.S(x^1.y) - a2.S(x^2.y) - a3.S(x^3.y) }
		+ 2{ a0.a1.S(x^1) + a0.a2.S(x^2) + (a0.a3+a1.a2).S(x^3) + a1.a3.S(x^4) + a2.a3.S(x^5) }


	[      0       0       0       0 ]   [         1           xb           xb^2         xb^3 ]   a0   -val(xb)
	[ S(x^1)  S(x^2)  S(x^3)  S(x^4) ] - [ S(x^0).xb    S(x^1).xb    S(x^2).xb    S(x^3).xb   ] * a1 = S(x^1.y)
	[ S(x^2)  S(x^3)  S(x^4)  S(x^5) ]   [ S(x^0).xb^2  S(x^1).xb^2  S(x^2).xb^2  S(x^3).xb^2 ]   a2   S(x^2.y)
	[ S(x^3)  S(x^4)  S(x^5)  S(x^6) ]   [ S(x^0).xb^3  S(x^1).xb^3  S(x^2).xb^3  S(x^3).xb^3 ]   a3   S(x^3.y)


*/






//////////////////////////////////
// SPolySegInfo functions
//////////////////////////////////

SPolySegInfo::SPolySegInfo()
{
	Reset();
}

void SPolySegInfo::Reset()
{
	nNumPoints = 0;
	Sr = Sabs = 0;
	nSumRefInit = nSumRefFinal = 0;
	bPolySet = false;
}

double SPolySegInfo::ValueAt(double x)
{
	double xPow, y;
	y  = coef[0];
	y += coef[1] * (xPow = x);
	y += coef[2] * (xPow *= x);
	y += coef[3] * (xPow *= x);
	return y;
}

void SPolySegInfo::AddPoint(SPointData& pt)
{
	if (nNumPoints++ == 0)
		Xinit = pt.x;
	Sr += pt.yErrorSq;
	Sabs += fabs(pt.yError);
	Xfinal = pt.x;
}

void SPolySegInfo::SetPolyFrom(SPolySegInfo& src)
{
	for (int i = 0; i < sizeof(coef)/sizeof(*coef); i++)
		coef[i] = src.coef[i];
	bPolySet = src.bPolySet;
}

void SPolySegInfo::CalcPointError(SPointData& pt)
{
	pt.yPoly = ValueAt(pt.x);
	pt.yError = pt.yPoly - pt.y;
	pt.yErrorSq = pt.yError * pt.yError;
}


//////////////////////////////////
// SPointPowerData functions
//////////////////////////////////

void SPointPowerData::Zero()
{
	num = 0;
	for (int i = 0; i < 7; i++)
		Xp[i] = 0;
	for (i = 0; i < 4; i++)
		YXp[i] = 0;
	Y2 = 0;
}

void SPointPowerData::SumUpPoint(const SPointPowerData& prevSum, SfPoint& pt)
{
	double xPow;
	double x = pt.x, y = pt.y;		// create local copies	
	num = prevSum.num + 1;
	Xp[0]  = prevSum.Xp[0] + 1;
	YXp[0] = prevSum.YXp[0] + y;
	Xp[1]  = prevSum.Xp[1] + (xPow = x);
	YXp[1] = prevSum.YXp[1] + (xPow * y);
	Xp[2]  = prevSum.Xp[2] + (xPow *= x);
	YXp[2] = prevSum.YXp[2] + (xPow * y);
	Xp[3]  = prevSum.Xp[3] + (xPow *= x);
	YXp[3] = prevSum.YXp[3] + (xPow * y);
	Xp[4]  = prevSum.Xp[4] + (xPow *= x);
	Xp[5]  = prevSum.Xp[5] + (xPow *= x);
	Xp[6]  = prevSum.Xp[6] + (xPow * x);
	Y2 = prevSum.Y2 + (y * y);		// needed to calc residual error (sum of squares)
	this->x = x;		// these just for checking
	this->y = y;
}


//////////////////////////////////
// CPointSumData functions
//////////////////////////////////

CPointSumData::CPointSumData()
{
	m_iStart = 0;
	m_iEnd = 0;
	m_iLength = 0;
	m_iBufferSize = sizeof(m_SumPoints) / sizeof(m_SumPoints[0]);
	m_iStartRef = 0;
}

int CPointSumData::AddPoint(SfPoint& pt)
{
	assert(m_iLength < m_iBufferSize);

	if (m_iLength == 0)			// set first point
	{
		m_SumPoints[m_iEnd].Zero();
		m_SumPoints[m_iEnd].SumUpPoint(m_SumPoints[m_iEnd], pt);
	}
	else
	{
		int iNewEnd = m_iEnd + 1;
		if (iNewEnd >= m_iBufferSize)
			iNewEnd -= m_iBufferSize;
		m_SumPoints[iNewEnd].SumUpPoint(m_SumPoints[m_iEnd], pt);
		m_iEnd = iNewEnd;
	}
	m_iLength++;
	return 1;
}

int CPointSumData::GetSumsFor(int iInit, int iFinal, SPointPowerData& sumPoints)
{
	if (iInit == 0 && iFinal == -1)
		sumPoints = m_SumPoints[m_iEnd];
	else
		assert(0);
	return 1;
}



//////////////////////////////////////////////////////////////////////
// CCubicFit functions
//////////////////////////////////////////////////////////////////////

enum
{
	FITSTAT_NOPOINTS = 0,
	FITSTAT_NOPOLY,
	FITSTAT_CURRSET,
	FITSTAT_REDUCESPAN,
};

CCubicFit::CCubicFit()
{
	Init();
}

CCubicFit::~CCubicFit()
{

}

void CCubicFit::Init()
{
	m_nFitStatus = 0;
	m_TolY = 1e-4;
	m_TolFirstSr = 1e-12;	// squared value!

	m_nFitStatus = FITSTAT_NOPOINTS;
	m_SuggestedInc = 0;
	m_nReductions = 0;

	m_CurrPt.Zero();
}

//////////////////////////////////////////////////////////////////////


// Main entry function for adding points to poly
int CCubicFit::NextPoint(double x, double y)
{
	m_CurrPt.x = x;
	m_CurrPt.y = y;

	if (m_nFitStatus == FITSTAT_CURRSET)
		TryPoint();
	else if (m_nFitStatus <= FITSTAT_NOPOLY)		// no initial poly is established yet
	{
		if (m_nFitStatus == FITSTAT_NOPOINTS)
		{
			InitFromFirstPoint();
			m_nFitStatus = FITSTAT_NOPOLY;
		}
		AddFitPoint();
		if (m_ExtrapoPoly.NumPoints() == 4)		// calc first poly attempt once enough points
		{
			FitPolyToPointsContPV(m_ExtrapoPoly);
			m_CurrPoly.SetPolyFrom(m_ExtrapoPoly);
			GetPolyErrors(m_CurrPoly);		// for testing
			double Sr = GetSumResidual(m_CurrPoly);
			if (Sr <= m_TolFirstSr)			// Sr is squared value
			{
				m_nFitStatus = FITSTAT_CURRSET;
			}
			else
			{
				m_nFitStatus = FITSTAT_REDUCESPAN;
				m_SuggestedInc /= 10;
				m_nReductions++;
			}
		}
	}
	else
		assert(0);


	return 1;
}






/*
	an approx poly has been established

	next point to test
	get error from poly
	if error is acceptable
		check vel and accel are acceptable		
		sum up error and error^2
		do next point
	else
		mark current point and go on to best fit up until here

	
	Redo poly to include new points
	store:
		Sr of new points
		Sr of current points
	
*/

void CCubicFit::InitFromFirstPoint()
{
	m_PrevPoly.derFinal[0] = m_CurrPt.y;	// pos
	m_PrevPoly.derFinal[1] = 0;				// vel
}

int CCubicFit::TryPoint()
{
// Current poly is set already
	int iPointResult = 0;

	m_ExtrapoPoly.CalcPointError(m_CurrPt);
	if (fabs(m_CurrPt.yError) <= m_TolY)
	{
		// add this point to current set   
		AddFitPoint();
		iPointResult = 1;
	}
	else
	{
	// refit poly to points up until this then check error again

		if (fabs(m_CurrPt.yError) <= m_TolY)
		{
			// add this point to current poly segment   
			AddFitPoint();
			iPointResult = 1;
		}
		else
		{
		// start a new poly segment with this point
		// if current segment hasn't many points, go back and do finer increments


		}
	}
	return iPointResult;
}

void CCubicFit::AddFitPoint()
{
	// add point and x.y powers to array of sums
	m_PointSumData.AddPoint(m_CurrPt);
	// add point details to Extrapolated poly object
	m_ExtrapoPoly.AddPoint(m_CurrPt);

}


double CCubicFit::GetSumResidual(SPolySegInfo& poly)
{
/*	Sr = Sum((y - Y)^2)		where y is actual value and Y is cubic value
	Sr = S(y^2) + a0^2.S(x^0) + a1^2.S(x^2) + a2^2.S(x^4) + a3^2.S(x^6)
		+ 2{-a0.S(x^0.y) - a1.S(x^1.y) - a2.S(x^2.y) - a3.S(x^3.y) }
		+ 2{ a0.a1.S(x^1) + a0.a2.S(x^2) + (a0.a3+a1.a2).S(x^3) + a1.a3.S(x^4) + a2.a3.S(x^5) }
*/
	SPointPowerData sumPoints;
	m_PointSumData.GetSumsFor(0, -1, sumPoints);		// based on poly range
	double* SumXp = sumPoints.Xp;
	double* SumYXp = sumPoints.YXp;
	double* a = poly.coef;
	double a0 = a[0], a1 = a[1], a2 = a[2], a3 = a[3];		// faster??
	double Sr;
	Sr = a0*a1*SumXp[1] + a0*a2*SumXp[2] + (a0*a3+a1*a2)*SumXp[3] + a1*a3*SumXp[4] + a2*a3*SumXp[5];
	Sr -= a0*SumYXp[0] + a1*SumYXp[1] + a2*SumYXp[2] + a3*SumYXp[3];
	Sr *= 2;
	Sr += a0*a0*SumXp[0] + a1*a1*SumXp[2] + a2*a2*SumXp[4] + a3*a3*SumXp[6];
	Sr += sumPoints.Y2;
	return Sr;
}

void CCubicFit::GetPolyErrors(SPolySegInfo& poly)
{
	SPointPowerData* arPD = m_PointSumData.m_SumPoints;
	double SumRes = 0;
	for (int i = m_PointSumData.m_iStart; i <= m_PointSumData.m_iEnd; i++)
	{
		SPointPowerData& pd = arPD[i];
		pd.yPoly = poly.ValueAt(pd.x);
		pd.yError = pd.yPoly - pd.y;
		pd.yErrorSq = pd.yError * pd.yError;
		SumRes += pd.yErrorSq;
	}
	arPD[i].yErrorSq = SumRes;		// store it!
}

// Fit poly to current points - not continuous from previous poly
void CCubicFit::FitPolyToPoints(SPolySegInfo& poly)
{
	SPointPowerData sumPoints;
	m_PointSumData.GetSumsFor(0, -1, sumPoints);
	// fill matrix for poly soltion
/*	int mxIdx = 0;
	for (int j = 0; j < 4; j++)			// row
	{
		int xpIdx = j;
		for (int k = 0; k < 4; k++)		// column
			matrix[mxIdx++] = SumXp[xpIdx++];
	}
*/
	// LUFullSymSolve uses a vector instead of matrix for a fully sym matrix
	LUFullSymSolve(4, sumPoints.Xp, poly.coef, sumPoints.YXp);		// solves matrix equ, result in m_arFitPoly
}

// Fit poly to current points with Pos and Vel continuous from previous poly
void CCubicFit::FitPolyToPointsContPV(SPolySegInfo& poly)
{
	SPointPowerData SumPts;
	m_PointSumData.GetSumsFor(0, -1, SumPts);
	double* SumXp = SumPts.Xp;
	double* SumYXp = SumPts.YXp;

	double det, b[2];
	double* fitPoly = poly.coef;
	double Xa = 1.234;		// initial x

	if (Xa == 0)		// xa is x location of initial pos & vel
	{
/*
		If initial pos and vel are at x=0 then equ's are simplified -> a0 & a1 known
		[ S(x^4)  S(x^5) ] * a2  =  S(x^2.y) - [ S(x^2)  S(x^3) ] * a0
		[ S(x^5)  S(x^6) ]   a3     S(x^3.y)   [ S(x^3)  S(x^4) ]   a1
*/
		// get determinent of 2x2 matrix
		det = SumXp[4]*SumXp[6] - SumXp[5]*SumXp[5];
		if (det == 0)
			assert(0);
		det = 1 / det;

		fitPoly[0] = m_PrevPoly.derFinal[0];	// derivative[0] is just value!
		fitPoly[1] = m_PrevPoly.derFinal[1];
		// m_arFitPoly[0,1] are already set
		b[0] = SumYXp[2] - SumXp[2]*fitPoly[0] - SumXp[3]*fitPoly[1];
		b[1] = SumYXp[3] - SumXp[3]*fitPoly[0] - SumXp[4]*fitPoly[1];
		fitPoly[2] = det * ( SumXp[6]*b[0] - SumXp[5]*b[1]);
		fitPoly[3] = det * (-SumXp[5]*b[0] + SumXp[4]*b[1]);
		return;
	}

/*
	If initial x != 0, but x = xa
	[ 1        xa       xa^2     xa^3 ]   a0    Pos(xa)
	[ 0        1       2xa      3xa^2 ] * a1 =  Vel(xa)
	                                      a2
	                                      a3
	mxBCs * vta  =  vtBCs

	eliminate a0, a1 (solve for these!)
	step 1:
	[ 1        0       -xa^2   -2xa^3 ]   a0    Pos(xa) - xa*Vel(xa)		//  -xa * row2
	[ 0        1       2xa      3xa^2 ] * a1 =  Vel(xa)
	                                      a2
	                                      a3
	dSr/da2 = dSra/da2 + dSra/da0 * da0/da2 + dSra/da1 * da1/da2 = 0		// from all dependent params
	-->
	dSr/da2   [ da0/da2  da1/da2    1    0 ]   dSra/da0     0
	dSr/da3 = [ da0/da3  da1/da3    0    1 ] * dSra/da1  =  0
	                                           dSra/da2
	                                           dSra/da3

	-> vtdSrRed = mxda * vtdSra = 0

	          [  xa^2   -2xa     1    0 ]
	-> mxda = [ 2xa^3   -3xa^2   0    1 ]
	     
	  mxda * vtdSr = 0
	& vtdSra = mxS * vta  -  vtSy

	mxda * mxS * vta  =  mxda * vtSy		// 2 equ's		(note: mxda doesn't have an inverse - not square!)
	     mxBCs * vta  =  vtBCs				// 2 equ's
*/

	CMatrix mxda(2,4), mxS(4,4), mxBCs(2,4);
	CMatrix vtSy(4), vtBCs(2), vtPolys(4);

	double XaP;
	// set matrix 'mxda'
	mxda.elem(0, 1) = -2*Xa;
	XaP = Xa*Xa;
	mxda.elem(0, 0) = XaP;
	mxda.elem(1, 1) = -3*XaP;
	XaP *= Xa;
	mxda.elem(1, 0) = 2*XaP;
	mxda.elem(0, 2) = 1;
	mxda.elem(1, 3) = 1;
	mxda.elem(0, 3) = 0;
	mxda.elem(1, 2) = 0;

	// set matrix 'mxS'
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++)
			mxS.elem(r, c) = SumPts.Xp[r+c];
	// set vector 'vtSy'
	for (r = 0; r < 4; r++)
		vtSy[r] = SumPts.YXp[r];
	// set matrix 'mxBCs'
	XaP = 1;
	for (int c = 0; ; )
	{
		mxBCs.elem(0, c) = XaP;
		if (++c == 4) break;
		mxBCs.elem(1, c) = c * XaP;
		XaP *= Xa;
	}
	mxBCs.elem(1, 0) = 0;

	// set vector 'vtBCs'
//	vtBCs[0] = Pos(Xa);
//	vtBCs[1] = Vel(Xa);


/*
	vta = [ a0  a1  a2  a3 ]'

	       [  xa^2   -2xa     1    0 ]
	mxda = [ 2xa^3   -3xa^2   0    1 ]

	      [ Sa(x^0)  Sa(x^1)  Sa(x^2)  Sa(x^3) ]
	mxS = [ Sa(x^1)  Sa(x^2)  Sa(x^3)  Sa(x^4) ]
	      [ Sa(x^2)  Sa(x^3)  Sa(x^4)  Sa(x^5) ]
	      [ Sa(x^3)  Sa(x^4)  Sa(x^5)  Sa(x^6) ]

	       [ S(x^0.y) ]
	vtSy = [ S(x^1.y) ]
	       [ S(x^2.y) ]
	       [ S(x^3.y) ]

	        [ 1        xa       xa^2     xa^3 ]
	mxBCs = [ 0        1       2xa      3xa^2 ]

	vtBCs = [ Pos(xa) ]
	        [ Vel(xa) ]

	------------------------------
	mxda * mxS * vta  =  mxda * vtSy		// 2 equ's		(note: mxda doesn't have an inverse - not square!)
	     mxBCs * vta  =  vtBCs				// 2 equ's

*/
	CMatrix mxAllEqus(4,4);
	CMatrix vtAllVals(4);

	mxAllEqus.Prod(mxda, mxS);
	vtAllVals.Prod(mxda, vtSy);
	mxAllEqus.SetPart(2, 0, mxBCs);
	vtAllVals.SetPart(2, 0, vtBCs);

	mxAllEqus.LUSolve(vtPolys.GetArray(), vtAllVals.GetArray());
//	vtPolys.Prod(msAllEqusSolve, vtAllVals);




}

// Fit two polys to current and extrapolated points with continuous Pos and Vel at initial and middle knot
void CCubicFit::FitDblPolyToPointsContPV(SPolySegInfo& poly)
{
	SPointPowerData SumPtsA, SumPtsB;
	m_PointSumData.GetSumsFor(0, -1, SumPtsA);
	m_PointSumData.GetSumsFor(0, -1, SumPtsB);	// set correct ranges

	double* fitPoly = poly.coef;


	CMatrix mxdab(4,8), mxSab(8,8), mxBCs(4,8);
	CMatrix vtSaby(8), vtBCs(4), vtPolys(8);

	double Xa, XaP, Xb, XbP;
	// set matrix 'mxdab'
//	Xa = ;
//	Xb = ;
	mxdab = 0;
	mxdab.elem(0  ,1  ) = -2*Xa;
	mxdab.elem(0+2,1+4) = -2*Xb;
	mxdab.elem(0  ,1+4) = -2*(Xa-Xb);
	XaP = Xa*Xa;
	XbP = Xb*Xb;
	mxdab.elem(0  ,0  ) = XaP;
	mxdab.elem(0+2,0+4) = XbP;
	mxdab.elem(0  ,0+4) = (XaP-XbP);
	mxdab.elem(1  ,1  ) = -3*XaP;
	mxdab.elem(1+2,1+4) = -3*XbP;
	mxdab.elem(1  ,1+4) = -3*(XaP-XbP);
	XaP *= Xa;
	XbP *= Xb;
	mxdab.elem(1  ,0  ) = 2*XaP;
	mxdab.elem(1+2,0+4) = 2*XbP;
	mxdab.elem(1  ,0+4) = 2*(XaP-XbP);
	mxdab.elem(0  ,2  ) = 1;
	mxdab.elem(1  ,3  ) = 1;
	mxdab.elem(0+2,2+4) = 1;
	mxdab.elem(1+2,3+4) = 1;

	// set matrix 'mxSab'
	mxSab = 0;
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++)
		{
			mxSab.elem(r, c) = SumPtsA.Xp[r+c];
			mxSab.elem(r+4, c+4) = SumPtsB.Xp[r+c];
		}
	// set vector 'vtSaby'
	for (r = 0; r < 4; r++)
	{
		vtSaby[r] = SumPtsA.YXp[r];
		vtSaby[r+4] = SumPtsB.YXp[r];
	}
	// set matrix 'mxBCs'
	XaP = XbP = 1;
	for (int c = 0; ; )
	{
		mxBCs.elem(0, c+0) = XbP;
		mxBCs.elem(2, c+0) = XaP;
		mxBCs.elem(0, c+4) = -XbP;
		mxBCs.elem(2, c+4) = 0;
		mxBCs.elem(3, c+4) = 0;
		if (++c == 4) break;
		mxBCs.elem(1, c+0) = c * XbP;
		mxBCs.elem(3, c+0) = c * XaP;
		mxBCs.elem(1, c+4) = -c * XbP;
		XaP *= Xa;
		XbP *= Xb;
	}
	mxBCs.elem(1, 0) = 0;
	mxBCs.elem(3, 0) = 0;
	mxBCs.elem(1, 4) = 0;

	// set vector 'vtBCs'
	vtBCs[0] = 0;
	vtBCs[1] = 0;
//	vtBCs[2] = Pos(xa);
//	vtBCs[3] = Vel(xa);


/*
	vtab = [ a0  a1  a2  a3  b0  b1  b2  b3 ]'

	        [  xa^2   -2xa     1    0     xa^2-xb^2     2xb-2xa     0    0 ]
	mxdab = [ 2xa^3   -3xa^2   0    1   2xa^3-2xb^3   3xb^2-3xa^2   0    0 ]
	        [  0        0      0    0          xb^2        -2xb     1    0 ]
	        [  0        0      0    0         2xb^3        -3xb^2   0    1 ]

	        [ Sa(x^0)  Sa(x^1)  Sa(x^2)  Sa(x^3)  0        0        0        0       ]
	mxSab = [ Sa(x^1)  Sa(x^2)  Sa(x^3)  Sa(x^4)  0        0        0        0       ]
	        [ Sa(x^2)  Sa(x^3)  Sa(x^4)  Sa(x^5)  0        0        0        0       ]
	        [ Sa(x^3)  Sa(x^4)  Sa(x^5)  Sa(x^6)  0        0        0        0       ]
	        [ 0        0        0        0        Sb(x^0)  Sb(x^1)  Sb(x^2)  Sb(x^3) ]
	        [ 0        0        0        0        Sb(x^1)  Sb(x^2)  Sb(x^3)  Sb(x^4) ]
	        [ 0        0        0        0        Sb(x^2)  Sb(x^3)  Sb(x^4)  Sb(x^5) ]
	        [ 0        0        0        0        Sb(x^3)  Sb(x^4)  Sb(x^5)  Sb(x^6) ]

	         [ Sa(x^0.y) ]
	vtSaby = [ Sa(x^1.y) ]
	         [ Sa(x^2.y) ]
	         [ Sa(x^3.y) ]
	         [ Sb(x^0.y) ]
	         [ Sb(x^1.y) ]
	         [ Sb(x^2.y) ]
	         [ Sb(x^3.y) ]

	        [ 1        xb       xb^2     xb^3    -1       -xb      -xb^2    -xb^3 ]
	mxBCs = [ 0        1       2xb      3xb^2     0       -1      -2xb     -3xb^2 ]
	        [ 1        xa       xa^2     xa^3     0        0        0        0    ]
	        [ 0        1       2xa      3xa^2     0        0        0        0    ]

	        [     0   ]
	vtBCs = [     0   ]
	        [ Pos(xa) ]
	        [ Vel(xa) ]

	------------------------------
	mxdab * mxSab * vtab  =  mxdab * vtSaby		// 4 equ's		(note: mxdab doesn't have an inverse - not square!)
	        mxBCs * vtab  =  vtBCs					// 4 equ's

*/
	CMatrix mxAllEqus(8,8);
	CMatrix vtAllVals(8);

	mxAllEqus.Prod(mxdab, mxSab);
	vtAllVals.Prod(mxdab, vtSaby);
	mxAllEqus.SetPart(4, 0, mxBCs);
	vtAllVals.SetPart(4, 0, vtBCs);

	mxAllEqus.LUSolve(vtPolys.GetArray(), vtAllVals.GetArray());
//	vtPolys.Prod(msAllEqusSolve, vtAllVals);




}


/*
	Reference: LU-Factorization, Crout's or Doolittle's method, Kreyszig p.1011
	Should work well without the need for pivoting on a symmetric, positive definite matrix!
	for matrix A, vector b and unknow vector x
		mxA * vtX = vtVal
		find L, U such that:  mxA = L*U
		where L is a lower triangular with diagonals of 1 and U is upper triangular
			mxA * vtX = LU * vtX = vtVal
		->	Ly = vtVal	where U * vtX = y
		L and U are be combined in LU to save memory!
		LU diagonal belongs to U.  L's diagonal is 1's
*/
void TMatrix<double>::LUSolve(double* vtX, double* vtVal)
{
	assert(w == h);
	int j, k, n, s, smax;		// j-row index, k-column index
	double dSum;
	const int maxN = 8;
	double LU[maxN*maxN], Uinv[maxN];	// reserve space for up to 8 square matrix without having to 'new' memory
	double Y[maxN];
	n = w;		// order of poly
	assert(n <= maxN);
	bool bIsLower;

	for (j = 0; j < n; j++)			// row
	{
		bIsLower = true;
		for (k = 0; k < n; k++)		// column
		{
			if (k == j)		// on diagonal element
				bIsLower = false;
			smax = bIsLower ? k : j;
			dSum = elem(j, k);
			for (s = 0; s < smax; s++)
				dSum -= LU[j*n + s] * LU[s*n + k];	// L[j, s] * U[s, k]
			if (bIsLower)
				LU[j*n + k] = dSum * Uinv[k];		// to lower
			else
			{
				LU[j*n + k] = dSum;		// using upper
				if (k == j)
					Uinv[j] = 1.0 / dSum;	// do div's for diagonal upper's only once, used again in back substitution
			}
		}
	}
	// now find	Ly = vtVal
	for (j = 0; j < n; j++)		// row
	{
		dSum = vtVal[j];
		for (k = 0; k < j; k++)
			dSum -= LU[j*n + k] * Y[k];	// using lower
		Y[j] = dSum;
	}
	// now find	Ux = y
	for (j = n-1; j >= 0; j--)		// row
	{
		dSum = Y[j];
		for (k = j+1; k < n; k++)
			dSum -= LU[j*n + k] * vtX[k];	// using upper
		vtX[j] = dSum * Uinv[j];			// inverse stored earlier
	}
}

/*
	LUFullSymSolve uses code shortcuts for applicable to fully symmetrical matricies
	as with least squares curve fitting
	where element Ajk = vectA[j+k]
	for a square matrix m*n vector length is (m+n-1)
*/
bool LUFullSymSolve(int n, const double* vectA, double* x, const double* b)
{
	int j, k, s, smax;
	double dSum;
	bool bIsLower;
	double LU[4*4], Uinv[4], y[4];	// reserve space for up to cubic polys
	if (n > 4)
	{
		assert(n <= 4);		// Order for LU solve is > 4 - need to increase array sizes
		return false;
	}

	for (j = 0; j < n; j++)			// row
	{
		bIsLower = true;
		for (k = 0; k < n; k++)		// column
		{
			if (k == j)		// on diagonal element
				bIsLower = false;
			smax = bIsLower ? k : j;
			dSum = vectA[j+k];
			for (s = 0; s < smax; s++)
				dSum -= LU[j*n + s] * LU[s*n + k];	// L[j, s] * U[s, k]
			if (bIsLower)
				LU[j*n + k] = dSum * Uinv[k];		// to lower
			else
			{
				LU[j*n + k] = dSum;		// using upper
				if (k == j)
					if (dSum == 0)
					{
						assert(0);		// Zero on diagonal for LU Solve!
						return false;
					}
					else
						Uinv[j] = 1.0 / dSum;	// do div's for diagonal upper's only once, used again in back substitution
			}
		}
	}
	// now find	Ly = b
	for (j = 0; j < n; j++)		// row
	{
		dSum = b[j];
		for (k = 0; k < j; k++)
			dSum -= LU[j*n + k] * y[k];	// using lower
		y[j] = dSum;
	}
	// now find	Ux = y
	for (j = n-1; j >= 0; j--)		// row
	{
		dSum = y[j];
		for (k = j+1; k < n; k++)
			dSum -= LU[j*n + k] * x[k];	// using upper
		x[j] = dSum * Uinv[j];			// inverse stored earlier
	}
	return true;
}
