// PathBuilder.cpp: implementation of the CPathBuilder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"

#include "PolyFunc.h"
#include "PosConverter.h"
#include "DimensionsDlg.h"

#include "PathBuilder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPathBuilder::CPathBuilder()
{
	m_pPath = NULL;

	m_diaTool = 0;
	m_radTool = 0;
	SetDefaults();
}

CPathBuilder::CPathBuilder(CPathDoc* pDoc)
{
	m_pPath = pDoc;

	m_diaTool = 0;
	m_radTool = 0;
	SetDefaults();
}

CPathBuilder::~CPathBuilder()
{
}


void CPathBuilder::Build()
{
}

void CPathBuilder::SetToolDiameterMM(double diaMM)
{
	m_diaTool = diaMM;				// mm value
	m_radTool = 0.5 * m_diaTool;
}

void CPathBuilder::SetToolDiameterInch(double diaInch)
{
	m_diaTool = diaInch * 25.4;	// Inch value
	m_radTool = 0.5 * m_diaTool;
}

void CPathBuilder::SetDefaults()
{
//	SetToolDiameterMM(18.0);			// 18mm tool
//	SetToolDiameterInch(3.0 / 16);	// 3/16" tool
	SetToolDiameterInch(1.0 / 4);		// 1/4" tool

	m_zClearance = 16;

	m_numZLevels = 2;
	m_arZLevel[0] = 6;
	m_arZLevel[1] = 0;
	m_arZLevel[2] = 0;
	
	m_vtStart = CVector(0, 0, 50);
	m_vtEnd = m_vtStart;

	m_vtPosMachPathOrigForCalc = 0;


	m_FRDrill = 10;
	m_FRRoute = 20;
	m_FRRetract = 25;
	m_FRRapid = FEEDRATE_RAPID;

	m_CurrPathProps.feedRate = FEEDRATE_DEFAULT;
	m_CurrPathProps.iTool = TOOL_DEFAULT;

	m_bAtMatrixStart = true;
	m_vtHoleLoc = 0;

}

void CPathBuilder::Minaret()
{
	SetDefaults();

	double width;
	// units are 72nd of inch
	double units2mm = 25.4 / 72.0;
	double bound[] = { 245.05115, 220.88041, 951.30085, 1435.02350 };

	double data[] = {
		245.05115, 220.88041,
		951.30085, 220.88041,
		951.30085, 948.47981,
		951.30085, 1029.31313, 905.09641, 1085.29002, 846.16101, 1118.15376,
		711.80731, 1193.07317, 684.57288, 1229.85269, 651.83357, 1292.90343,
		628.56794, 1337.70898, 610.48998, 1386.68315, 598.17600, 1435.02350,

		585.86202, 1386.68315, 567.78406, 1337.70898, 544.51843, 1292.90343,
		511.77912, 1229.85269, 484.54469, 1193.07288, 350.19099, 1118.15376,
		291.25587, 1085.29002, 245.05115, 1031.36513, 245.05115, 950.53153,
		245.05115, 220.88098,
	};

	CVect2 vtMin(bound[0], bound[1]);
	CVect2 vtMax(bound[2], bound[3]);

	const int iNumVals = sizeof(data) / sizeof(data[0]);
	const int iNumNodes = iNumVals / 2;
	ASSERT(iNumVals % 2 == 0);
	CVect2 arDataScaled[iNumNodes];

	double scale = width / (vtMax.x - vtMin.x);	// scale to width
	double radTool = m_radTool;

	CPathNode nd;
	CVect2 vtPnt;

	for (int i =  0; i < iNumNodes; i++)
	{
		vtPnt.Set(data[2*i], data[2*i+1]);
		arDataScaled[i] = (vtPnt - vtMin) * units2mm;
	}


	CVect2* arBez = &arDataScaled[2];
	CVect2 arBezOffset[10];



	CVect2 arToolPath[80];
	int iTP = 0;
	arToolPath[iTP++] = arDataScaled[0] + CVect2(-radTool, -radTool);
	arToolPath[iTP++] = arDataScaled[1] + CVect2(+radTool, -radTool);
	OffsetBezierEndDeriv(&arToolPath[iTP++], &arDataScaled[2], -radTool);
	iTP += 2;
	OffsetBezierEndDeriv(&arToolPath[iTP++], &arDataScaled[2+3], -radTool);
	iTP += 2;
	OffsetBezierEndDeriv(&arToolPath[iTP++], &arDataScaled[2+6], -radTool);
	iTP += 2;

	CVect2 ndEnd = arToolPath[iTP-0];
	CVect2 ndControl = arToolPath[iTP-1];
	CVect2 apex = arDataScaled[11];		// apex
	CVect2 vtS = ndEnd - ndControl;
	double h = (apex.x - ndEnd.x) / vtS.x * vtS.y;
	iTP++;
	arToolPath[iTP++].Set(apex.x, ndEnd.y + h);
	arToolPath[iTP++].Set(apex.x, ndEnd.y + h);		// same

	OffsetBezierEndDeriv(&arToolPath[iTP++], &arDataScaled[11], -radTool);
	iTP += 2;
	OffsetBezierEndDeriv(&arToolPath[iTP++], &arDataScaled[11+3], -radTool);
	iTP += 2;
	OffsetBezierEndDeriv(&arToolPath[iTP++], &arDataScaled[11+6], -radTool);
	iTP += 2;

	iTP++;
	arToolPath[iTP++] = arDataScaled[21] + CVect2(-radTool, -radTool);





	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);			// starts above 0,0
	// first point
	nd = arToolPath[0];
	nd.z = m_zClearance;
	MoveToXYZ(nd);

	SetFeedRate(m_FRRoute);

	for (int iZLevel = 0; iZLevel < m_numZLevels; iZLevel++)
	{
		double zLevel = m_arZLevel[iZLevel];

		iTP = 0;
		nd.type = PNT_ENDNODE;
		nd = arToolPath[iTP++];
		nd.z = zLevel;
		AddNode(nd);
		nd = arToolPath[iTP++];
		nd.z = zLevel;
		AddNode(nd);
		nd = arToolPath[iTP++];
		nd.z = zLevel;
		AddNode(nd);
		for (i = 0; i < 7; i++)		// num of beziers
		{
			nd.type = PNT_CONTROLNODE;
			nd = arToolPath[iTP++];
			nd.z = zLevel;
			AddNode(nd);
			nd.type = PNT_CONTROLNODE;
			nd = arToolPath[iTP++];
			nd.z = zLevel;
			AddNode(nd);
			nd.type = PNT_ENDNODE;
			nd = arToolPath[iTP++];
			nd.z = zLevel;
			AddNode(nd);
		}

		nd = arToolPath[iTP++];
		nd.z = zLevel;
		AddNode(nd);
	}

	SetFeedRate(m_FRRetract);
	nd.z = m_zClearance;
	AddNode(nd);
	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);

}

void CPathBuilder::Heart1(double width)
{
	SetDefaults();
	// units are 72nd of inch
	double bound[] = { 676.37735, 102.11131, 1227.26976, 627.34649 };

	double data[] = {
		951.82356, 102.11131,
		921.84463, 167.62337, 850.24205, 221.51452, 801.93827, 264.07899,
		758.04690, 302.75546, 717.12964, 341.56431, 688.31830, 411.55427,
		651.24624, 501.61153, 705.44863, 603.32995, 782.60202, 622.83543,
		845.38970, 638.70888, 913.14312, 611.22954, 951.82356, 563.85043,
		990.50400, 611.22954, 1058.25742, 638.70888, 1121.04510, 622.83543,
		1198.19849, 603.32995, 1252.40088, 501.61153, 1215.32882, 411.55427,
		1186.51748, 341.56431, 1145.60022, 302.75546, 1101.70885, 264.07899,
		1053.40507, 221.51452, 981.80249, 167.62337, 951.82356, 102.11131,
	};

	CVect2 vtMin(bound[0], bound[1]);
	CVect2 vtMax(bound[2], bound[3]);

	int iNumVals = sizeof(data) / sizeof(data[0]);
	int iNumNodes = iNumVals / 2;
	ASSERT(iNumVals % 2 == 0);
	ASSERT(iNumNodes % 3 == 1);
	int iNumEndNodes = (iNumNodes+2) / 3;

	double scale = width / (vtMax.x - vtMin.x);	// scale to width
	CPathNode nd;
	CVect2 vtPnt;


	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);			// starts above 0,0
	// first point
	vtPnt.Set(data[0], data[1]);
	nd = (vtPnt - vtMin) * scale;
	nd.z = m_zClearance;
	MoveToXYZ(nd);

	SetFeedRate(m_FRRoute);

	for (int iZLevel = 0; iZLevel < m_numZLevels; iZLevel++)
	{
		double zLevel = m_arZLevel[iZLevel];
		for (int i = 0; i < iNumNodes; i++)
		{
			vtPnt.Set(data[2*i], data[2*i+1]);
			nd = (vtPnt - vtMin) * scale;
			nd.z = zLevel;
			if (i%3 == 0)
				nd.type = PNT_ENDNODE;
			else
				nd.type = PNT_CONTROLNODE;
			AddNode(nd);
		}
	}

	SetFeedRate(m_FRRetract);
	nd.z = m_zClearance;
	AddNode(nd);
	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);

}

void CPathBuilder::Heart2(double wInside, double wOutside)
{
	SetDefaults();

	// units are 72nd of inch
	double bound[] = { 676.37735, 102.11131, 1227.26976, 627.34649 };

	double data[] = {
		951.82356, 102.11131,
		921.84463, 167.62337, 850.24205, 221.51452, 801.93827, 264.07899,
		758.04690, 302.75546, 717.12964, 341.56431, 688.31830, 411.55427,
		651.24624, 501.61153, 705.44863, 603.32995, 782.60202, 622.83543,
		845.38970, 638.70888, 913.14312, 611.22954, 951.82356, 563.85043,
		990.50400, 611.22954, 1058.25742, 638.70888, 1121.04510, 622.83543,
		1198.19849, 603.32995, 1252.40088, 501.61153, 1215.32882, 411.55427,
		1186.51748, 341.56431, 1145.60022, 302.75546, 1101.70885, 264.07899,
		1053.40507, 221.51452, 981.80249, 167.62337, 951.82356, 102.11131,
	};

	CVect2 vtMin(bound[0], bound[1]);
	CVect2 vtMax(bound[2], bound[3]);
	CVect2 vtCenter = (vtMax + vtMin) / 2;

	int iNumVals = sizeof(data) / sizeof(data[0]);
	int iNumNodes = iNumVals / 2;
	ASSERT(iNumVals % 2 == 0);
	ASSERT(iNumNodes % 3 == 1);
	int iNumEndNodes = (iNumNodes+2) / 3;


	double scaleI = wInside / (vtMax.x - vtMin.x);	// scale to width
	double scaleO = wOutside / (vtMax.x - vtMin.x);	// scale to width
	CVect2 vtPntCI = (vtCenter - vtMin) * scaleI;
	CVect2 vtPntCO = (vtCenter - vtMin) * scaleO;
	CVect2 vtIadjust = vtPntCO - vtPntCI;

	double scale = scaleI;

	CPathNode nd;
	CVect2 vtPnt;


	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);			// starts above 0,0
	// first point
	// inside
	vtPnt.Set(data[0], data[1]);
	nd = (vtPnt - vtMin) * scale + vtIadjust;
	nd.z = m_zClearance;
	MoveToXYZ(nd);

	SetFeedRate(m_FRRoute);

	for (int iZLevel = 0; iZLevel < m_numZLevels; iZLevel++)
	{
		double zLevel = m_arZLevel[iZLevel];
		for (int i = 0; i < iNumNodes; i++)
		{
			vtPnt.Set(data[2*i], data[2*i+1]);
			nd = (vtPnt - vtMin) * scale + vtIadjust;
			nd.z = zLevel;
			if (i%3 == 0)
				nd.type = PNT_ENDNODE;
			else
				nd.type = PNT_CONTROLNODE;
			AddNode(nd);
		}
	}
	SetFeedRate(m_FRRetract);
	nd.z = m_zClearance;
	AddNode(nd);

	// move to outside
	scale = scaleO;
	vtPnt.Set(data[0], data[1]);
	nd = (vtPnt - vtMin) * scale;
	nd.z = m_zClearance;
	MoveToXYZ(nd);

	for (iZLevel = 0; iZLevel < m_numZLevels; iZLevel++)
	{
		double zLevel = m_arZLevel[iZLevel];
		for (int i = 0; i < iNumNodes; i++)
		{
			vtPnt.Set(data[2*i], data[2*i+1]);
			nd = (vtPnt - vtMin) * scale;
			nd.z = zLevel;
			if (i%3 == 0)
				nd.type = PNT_ENDNODE;
			else
				nd.type = PNT_CONTROLNODE;
			AddNode(nd);
		}
	}


	SetFeedRate(m_FRRetract);
	nd.z = m_zClearance;
	AddNode(nd);
	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);

}



void CPathBuilder::Boatshed(int iSize)
{
	SetDefaults();

	double zSupport = 1;
	double wSupport = 5;

	double BaseWidth  = 0;		// base rectangle width
	double BaseHeight = 0;		// base rectangle height
	double RoofWidth  = 0;		// eave overhang
	double RoofHeight = 0;		// from apex to roof base
	switch (iSize)
	{
	case SIZE_SMALL:
		BaseWidth  = 110;
		BaseHeight = 148;
		RoofWidth  = 150;
		RoofHeight = 78;
		break;
	case SIZE_MEDIUM:
		BaseWidth  = 133;
		BaseHeight = 169;
		RoofWidth  = 185;
		RoofHeight = 93;
		break;
	case SIZE_LARGE:
		BaseWidth  = 155;
		BaseHeight = 190;
		RoofWidth  = 225;
		RoofHeight = 115;
		break;
	default:
		ASSERT(0);
		return;
	}
	double Eave = 0.5 * (RoofWidth - BaseWidth);
	double a = RoofHeight;
	double b = Eave + 0.5*BaseWidth;
	double radTool = m_radTool;

	const int numNodes = 7;
	CVector nd[numNodes];
	CVector vtRef = 0;
	// shape origion is bottom left of bounding rectangle
	// first is bottom left
	nd[0].x = vtRef.x + Eave - radTool;
	nd[0].y = vtRef.y + -radTool;
	nd[1].x = nd[0].x + BaseWidth + 2*radTool;
	nd[1].y = nd[0].y;
	nd[2].x = nd[1].x;
	nd[2].y = nd[1].y + BaseHeight;
	nd[3].x = nd[2].x + Eave - radTool + radTool * (b+sqrt(a*a + b*b))/a;
	nd[3].y = nd[2].y;
	nd[4].x = vtRef.x + Eave + 0.5*BaseWidth;
	nd[4].y = vtRef.y + BaseHeight + RoofHeight + radTool * sqrt(a*a + b*b)/b;
	nd[5].x = vtRef.x + -radTool * (b+sqrt(a*a + b*b))/a;
	nd[5].y = nd[3].y;
	nd[6].x = nd[0].x;
	nd[6].y = nd[2].y;

	CVector ndSup[4];
	ndSup[1].x = nd[2].x;
	ndSup[1].y = nd[2].y - 10;
	ndSup[0].x = ndSup[1].x;
	ndSup[0].y = ndSup[1].y - wSupport - 2*radTool;

	ndSup[2].x = nd[6].x;
	ndSup[2].y = ndSup[1].y;
	ndSup[3].x = ndSup[2].x;
	ndSup[3].y = ndSup[0].y;


	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);			// starts above 0,0
	nd[0].z = m_zClearance;
	MoveToXYZ(nd[0]);

	for (int iZLevel = 0; iZLevel < m_numZLevels; iZLevel++)
	{
		double z = m_arZLevel[iZLevel];
		SetFeedRate(m_FRDrill);
		MoveToZ(z);
		SetFeedRate(m_FRRoute);

		if (iZLevel != m_numZLevels-1)
		{
			for (int i = 1; i < numNodes; i++)
				MoveToXY(nd[i]);
		}
		else
		{
			MoveToXY(nd[1]);
			MoveToXY(ndSup[0]);
			MoveToZ(zSupport);
			MoveToXY(ndSup[1]);
			MoveToZ(z);
			for (int i = 2; i <= 6; i++)
				MoveToXY(nd[i]);
			MoveToXY(ndSup[2]);
			MoveToZ(zSupport);
			MoveToXY(ndSup[3]);
			MoveToZ(z);
		}
		MoveToXY(nd[0]);
	}


	SetFeedRate(m_FRRetract);
	MoveToZ(m_zClearance);
	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);			// ends above 0,0


}

void CPathBuilder::Rectangle(double dx, double dy )
{
	SetDefaults();

	double radTool = m_radTool;
	const int numNodes = 4;
	CVector nd[numNodes];
	CVector vtRef = 0;
	// shape origion is bottom left of bounding rectangle
	// first is bottom left
	nd[0].x = vtRef.x - radTool;
	nd[0].y = vtRef.y - radTool;
	nd[1].x = vtRef.x + dx + radTool;
	nd[1].y = nd[0].y;
	nd[2].x = nd[1].x;
	nd[2].y = vtRef.y + dy + radTool;
	nd[3].x = nd[0].x;
	nd[3].y = nd[2].y;


	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);			// starts above 0,0
	nd[0].z = m_zClearance;
	MoveToXYZ(nd[0]);

	for (int iZLevel = 0; iZLevel < m_numZLevels; iZLevel++)
	{
		double z = m_arZLevel[iZLevel];
		SetFeedRate(m_FRDrill);
		MoveToZ(z);
		SetFeedRate(m_FRRoute);

		for (int i = 1; i < numNodes; i++)
			MoveToXY(nd[i]);

		MoveToXY(nd[0]);
	}

	SetFeedRate(m_FRRetract);
	MoveToZ(m_zClearance);
	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);			// ends above 0,0
}

void CPathBuilder::Circle(double diaCircle)
{
	SetDefaults();


	// control node distance from end nodes = radius * 4/3 * (1 - cos(angArc/2)) / sin(angArc/2)   -> radius * 0.5523 for 90deg
	// each bezier arc is 45deg
	double radCircle = 0.5 * diaCircle;
	double diameter = diaCircle + 2 * m_radTool;
	double radius = 0.5 * diameter;
	double angSeg = 45 * deg2rad;
	int iNumSegs = 8;
	double cnodeDist = 4.0/3 * radius * (1-cos(0.5*angSeg)) / sin(0.5*angSeg);	// this gives bezier that touches arc midway

	CVector vtCenter(radCircle, radCircle, 0);
	CVector vtRef(0);

	CPathNode vtE, vtCi, vtCf;
	vtE.type = PNT_ENDNODE;
	vtCi.type = vtCf.type = PNT_CONTROLNODE;


	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);			// starts above 0,0
	vtCenter.z = m_zClearance;

	for (int iZLevel = 0; iZLevel < m_numZLevels+1; iZLevel++)
	{
		double angE = -90 * deg2rad;			// end node angle
		for (int iSeg = 0; iSeg < iNumSegs; iSeg++, angE += angSeg)	// do iNumSegs-1 loops, 'iSeg' value not used
		{
			double cosA = cos(angE);
			double sinA = sin(angE);
			vtE.Set(cosA*radius, sinA*radius, 0);
			vtCi.Set(-sinA*cnodeDist, cosA*cnodeDist, 0);		// rotate 90
			vtCi.Rotate180(vtCf);
			if (iSeg == 1)
				if (iZLevel == m_numZLevels)
					iSeg = iNumSegs;					// force end, don't change z
				else
					vtCenter.z = m_arZLevel[iZLevel];
			vtE += vtCenter;
			vtCi += vtE;
			vtCf += vtE;
			if (iSeg == 0 && iZLevel == 0)
			{
				AddNode(vtE);
				SetFeedRate(m_FRRoute);
			}
			else
			{
				AddNode(vtCf);
				AddNode(vtE);
			}
			AddNode(vtCi);
		}
	}

	RemoveLastNode();
	SetFeedRate(m_FRRetract);
	vtE.z = m_zClearance;
	AddNode(vtE);
	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);

}

void CPathBuilder::HoleMatrixGeneral()
{
	ASSERT(m_pPath != NULL);
	SetDefaults();

// parameters
	m_vtDelS = CVector(500, 0, 0);
	m_vtDelT = CVector(0, 260, 0);

	m_vtExtentMin = CVector(170, 30, 0);
	m_vtExtentMax = CVector(1800, 840, 0);

	m_zBottomDrill = -3;						// abs
	m_zTopDrill = 3;
	m_zClearance = m_zTopDrill + 5;

	HoleMatrixCurved();
}

void CPathBuilder::HoleMatrixServoCalibration()
{
	// use same locations as PosConverter
/*
	double xCenter = 920;
	double arXLocs[4] = { -750, -250, 250, 750 };
	double yCenter = 420;
	double arYLocs[4] = { -390, -130, 130, 390 };
	double zCenter = 0;
	double arZLocs[2] = { 60, 300 };
*/
	ASSERT(m_pPath != NULL);
	SetDefaults();

// parameters
	m_vtDelS = CVector(500, 0, 0);
	m_vtDelT = CVector(0, 260, 0);

	m_vtExtentMin = CVector(170, 30, 0);
	m_vtExtentMax = CVector(1800, 840, 0);

	m_vtStart.x = m_vtExtentMin.x;
	m_vtStart.y = m_vtExtentMin.y;
	m_vtEnd = m_vtStart;

	m_vtPosMachPathOrigForCalc.Set(0, 0, 60);

	m_zBottomDrill = -3;						// abs
	m_zTopDrill = 3;
	m_zClearance = m_zTopDrill + 5;

	HoleMatrixCurved();
}

void CPathBuilder::BaseScrewsArray()
{
	ASSERT(m_pPath != NULL);
	SetDefaults();
	SetToolDiameterInch(1.0 / 4);		// 1/4" tool

// parameters
	m_vtDelS = CVector(200, 0, 0);
	m_vtDelT = CVector(0, 200, 0);

	m_vtExtentMin = CVector(0, 0, 0);
	m_vtExtentMax = CVector(1600, 800, 0);


	m_vtPosMachPathOrigForCalc.Set(120, 20, 60);

	m_zTopDrill = 18.5;							// abs
	m_zClearance = m_zTopDrill + 5;		// abs

	MoveToXYZ(m_vtStart);

	SetToStartMatrixHole();
	while (MoveToNextMatrixHole())
	{
//		HoleDrill(m_zTopDrill, m_zTopDrill - 1.5);
//		HoleMillByZLevel(18, m_zTopDrill, 11.5, DIR_ZNEG, DIR_ZPOS);

		HoleMillByZLevel(18, m_zTopDrill, 11.5, DIR_ZNEG, DIR_YPOS);
		HoleHelix(10, 11.5, -0.5, DIR_YPOS, DIR_ZPOS);
	}

	MoveToXYZ(m_vtEnd);
}

void CPathBuilder::BaseVacuumHoseSingle()
{
	ASSERT(m_pPath != NULL);
	SetDefaults();
	SetToolDiameterInch(1.0 / 4);		// 1/4" tool

// parameters

	m_zTopDrill = 18.5;						// abs
	double zBaseUpper = 5;
	m_zClearance = m_zTopDrill + 5;		// abs

	CDimensionsDlg dlg;
	dlg.m_strHeading = "Base Vacuum Hose Single Hole";
	dlg.m_strVal1 = "Upper diameter";
	dlg.m_strVal2 = "Lower diameter";
	dlg.m_dia1 = 20.8;
	dlg.m_dia2 = 14;
	dlg.DoModal();

	double diaUpper1 = dlg.m_dia1;
	double diaLower1 = dlg.m_dia2;
	double zBaseUpper1 = zBaseUpper;
	double diaUpper2 = diaUpper1;
	double zBaseUpper2 = zBaseUpper1;
	if (dlg.m_bFinishCut)
	{
		diaUpper1 -= 2 * dlg.m_finishRadius;
		zBaseUpper1 += dlg.m_finishDepth;
	}

	MoveToXYZ(m_vtStart);

	HoleMillByZLevel(diaUpper1, m_zTopDrill, zBaseUpper1, DIR_ZNEG, DIR_YPOS);
	HoleHelix(diaLower1, zBaseUpper1, -0.5, DIR_YPOS, DIR_ZPOS);
	if (dlg.m_bFinishCut)
		HoleHelix(diaUpper2, zBaseUpper2, zBaseUpper2, DIR_YPOS, DIR_ZPOS);

	MoveToXYZ(m_vtEnd);
}

void CPathBuilder::BaseVacuumHoseArray()
{
	ASSERT(m_pPath != NULL);
	SetDefaults();
	SetToolDiameterInch(1.0 / 4);		// 1/4" tool

// parameters
	m_vtDelS = CVector(200, 0, 0);
	m_vtDelT = CVector(0, 200, 0);

	// start at (162.0  126.0) if machine span = [1836, 852]
	m_vtExtentMin = CVector(0, 0, 0);
	m_vtExtentMax = CVector(1600, 600, 0);


	m_zTopDrill = 18.5;						// abs
	double zBaseUpper = 5;
	m_zClearance = m_zTopDrill + 10;		// abs !!!! allow for screw heads !!!!!

	CDimensionsDlg dlg;
	dlg.m_strHeading = "Base Vacuum Hose Array of Holes";
	dlg.m_strVal1 = "Upper diameter";
	dlg.m_strVal2 = "Lower diameter";
	dlg.m_dia1 = 20.8;
	dlg.m_dia2 = 14;
	dlg.m_bFinishCut = true;
	dlg.DoModal();

	double diaUpper1 = dlg.m_dia1;
	double diaLower1 = dlg.m_dia2;
	double zBaseUpper1 = zBaseUpper;
	double diaUpper2 = diaUpper1;
	double zBaseUpper2 = zBaseUpper1;
	if (dlg.m_bFinishCut)
	{
		diaUpper1 -= 2 * dlg.m_finishRadius;
		zBaseUpper1 += dlg.m_finishDepth;
	}


	MoveToXYZ(m_vtStart);

	SetToStartMatrixHole();
	while (MoveToNextMatrixHole())
	{
		HoleMillByZLevel(diaUpper1, m_zTopDrill, zBaseUpper1, DIR_ZNEG, DIR_YPOS);
		HoleHelix(diaLower1, zBaseUpper1, -0.5, DIR_YPOS, DIR_ZPOS);
		if (dlg.m_bFinishCut)
			HoleHelix(diaUpper2, zBaseUpper2, zBaseUpper2, DIR_YPOS, DIR_ZPOS);
	}

	MoveToXYZ(m_vtEnd);
}

void CPathBuilder::HoleDrill(double zTop, double zBottom)
{
	NODEREF nr;
	CPathNode& ndLast = *m_pPath->GetLastNode(nr);
	if (fabs(zTop - ndLast.z) > 0.1 || !ndLast.IsEndPoint())
		MoveToZ(zTop);				// was high feed rate
	SetFeedRate(m_FRDrill);
	MoveToZ(zBottom);
	SetFeedRate(m_FRRetract);
	MoveToZ(zTop);
	return;
}
	
void CPathBuilder::HoleMillByZLevel(double diameter, double zTop, double zBottom, int nStartType, int nEndType)
{	
	double zDepth = zTop - zBottom;
	ASSERT(zDepth >= 0);

	double radOuter = 0.5 * diameter - m_radTool;
	if (fabs(radOuter) <= 1e-3)	// hole is tool size - just drill
	{
		//DrillHole();
		m_vtHoleLoc.z = zTop;
		MoveToXYZ(m_vtHoleLoc);
		SetFeedRate(m_FRDrill);
		MoveToZ(zTop - zDepth);
		SetFeedRate(m_FRRetract);
		MoveToZ(zTop);
		return;
	}
	if (radOuter < 0)					// hole smaller than tool - can't do it!
		return;
	if (radOuter <= m_radTool)		// hole <= 2x tool - spiral down
	{
		HoleHelix(diameter, zTop, zDepth, nEndType);
		return;
	}

	
	// hole > 2x tool so mill each depth

	double XYStep = 0.5 * (2 * m_radTool);
	double radInner = XYStep / 2;
	double radDiff = radOuter - radInner;
	int numRad = 2 + int(radDiff / XYStep - 0.1);		// allow at bit over
	double radiusStage = radDiff / (numRad-1);

	double depthStageMax = 7.1;

	int numDepths = 1 + int(zDepth / depthStageMax - 0.1);		// allow at bit over
	double depthStage = zDepth / numDepths;



	//GetBezierArcControlNodeRadiusRatio(90 * deg2rad);
	double cnodeRatio = GetBezier90ArcControlNodeRadiusRatio();

	double angSeg = -90 * deg2rad;
	int numSegs = 4;
	double angInit = 0;

	CVector vtCentre = m_vtHoleLoc;
	CPathNode ndEnd, ndCi, ndCf;
	ndEnd.type = PNT_ENDNODE;
	ndCi.type = ndCf.type = PNT_CONTROLNODE;

	double radius;
	double cnodeDist;
	double zLevel = zTop;

	// do entry move
	if (nStartType == DIR_ZNEG)
	{
		// starts above top centre moving(0,0,-1)
		vtCentre.z = zLevel;
		ndEnd = vtCentre;
		NODEREF nr;
		CPathNode& ndLast = *m_pPath->GetLastNode(nr);
		if (fabs(ndEnd.z - ndLast.z) > 0.1 || !ndLast.IsEndPoint())
			AddNode(ndEnd);		// was high feed rate
		SetFeedRate(m_FRRoute);
		ndCi = vtCentre;
		ndCi.z -= depthStage * cnodeRatio;
	}
	else if (nStartType == DIR_YPOS)
	{
		// starts at top centre moving(0,1,0)
		SetFeedRate(m_FRRoute);
		vtCentre.z = zLevel;
		ndCi = vtCentre;
		ndCi.y += radInner * cnodeRatio;
	}
	else
		ASSERT(0);

	AddNode(ndCi);

	for (int iDepth = 1; iDepth <= numDepths; iDepth++)
	{
		zLevel = zTop - iDepth * depthStage;
		vtCentre.z = zLevel;
		int iSegInit = -1;			// start 90 deg earlier for first radius
		for (int iRadius = 0; iRadius < numRad; iRadius++)
		{
			radius = radInner + iRadius * radiusStage;
			cnodeDist = cnodeRatio * radius;
			int iSegFinal = numSegs - 1;
			if (iRadius == numRad - 1)			// finish 90 later on last radius
				iSegFinal++;
			for (int iSeg = iSegInit; iSeg <= iSegFinal; iSeg++)		// do numSeg-1 loops
			{
				double angE = angInit + angSeg * iSeg;		// end node angle
				double cosA = cos(angE);
				double sinA = sin(angE);
				ndEnd.Set(cosA*radius, sinA*radius, 0);
				ndCi.Set(sinA*cnodeDist, -cosA*cnodeDist, 0);		// rotate -90
				ndCi.Neg(ndCf);
				ndEnd += vtCentre;
				ndCi += ndEnd;
				ndCf += ndEnd;
				AddNode(ndCf);
				AddNode(ndEnd);
				AddNode(ndCi);
			}
			iSegInit = 0;
		}

		if (iDepth != numDepths)
		{
			// same zLevel, seg = 2, iRad = 0
			ndCf = vtCentre;
			ndCf.x -= radInner;
			ndCf.y -= radius * cnodeRatio;
			ndEnd = vtCentre;
			ndEnd.x -= radInner;
			ndCi = vtCentre;
			ndCi.x -= radInner;
			ndCi.y += radInner * cnodeRatio;
			AddNode(ndCf);
			AddNode(ndEnd);
			AddNode(ndCi);
		}
	}	// for (int iDepth...)

	// finish off
	if (nEndType == DIR_ZPOS && zDepth < 0.5)		// can't do if too shallow
		nEndType = DIR_YPOS;
	if (nEndType == DIR_ZPOS)	// last depth - do final move
	{
		// retract to top centre moving(0,0,1)
		vtCentre.z = zTop;
		ndCf = vtCentre;
		ndCf.z -= zDepth * cnodeRatio;
		ndEnd = vtCentre;
		AddNode(ndCf);
		AddNode(ndEnd);
	}
	else if (nEndType == DIR_YPOS)
	{
		// move to bottom centre moving(0,1,0)
		ndCf = vtCentre;
		ndCf.y -= radius * cnodeRatio;
		ndEnd = vtCentre;
		AddNode(ndCf);
		AddNode(ndEnd);
	}
	else
		ASSERT(0);

}

void CPathBuilder::HoleHelix(double diameter, double zTop, double zBottom, int nStartType, int nEndType)
{	
	double radOuter = 0.5 * diameter - m_radTool;
	if (fabs(radOuter) <= 1e-3)	// hole is tool size - just drill
	{
		//DrillHole();
		return;
	}
	if (radOuter < 0)					// hole smaller than tool - can't do it!
		return;
	if ((radOuter - 1.5*m_radTool) > 0)		// hole > 2x tool so mill each depth
	{
		// MillHole();
//		return;
	}
	// hole <= 2x tool - spiral down

	double zDepth = zTop - zBottom;
	ASSERT(zDepth >= 0);

	double depthStageMax = 6.1;

	int numDepths = 1 + int((zDepth - 0.51) / depthStageMax);		// allow at bit over
	double depthStage = zDepth / numDepths;


	//GetBezierArcControlNodeRadiusRatio(90 * deg2rad);
	double cnodeRatio = GetBezier90ArcControlNodeRadiusRatio();

	double angSeg = -90 * deg2rad;
	int numSegs = 4;
	double angInit = 0;

	double depthSeg = depthStage / numSegs;
	double cnodeZDist = depthSeg / 3;


	CVector vtCentre = m_vtHoleLoc;
	CPathNode ndEnd, ndCi, ndCf;
	ndEnd.type = PNT_ENDNODE;
	ndCi.type = ndCf.type = PNT_CONTROLNODE;

	double radius = radOuter;
	double cnodeDist = cnodeRatio * radius;
	double zLevel = zTop;


	// do entry move
	if (nStartType == DIR_ZNEG)
	{
		// start above top centre moving(0,0,-1)
		vtCentre.z = zLevel;
		ndEnd = vtCentre;
		NODEREF nr;
		CPathNode& ndLast = *m_pPath->GetLastNode(nr);
		if (fabs(ndEnd.z - ndLast.z) > 0.1 || !ndLast.IsEndPoint())
			AddNode(ndEnd);		// was high feed rate
		SetFeedRate(m_FRRoute);

		ndCi = vtCentre;
		ndCi.z -= depthStage * cnodeRatio;
	}
	else if (nStartType == DIR_YPOS)
	{
		// starts at top centre moving(0,1,0)
		vtCentre.z = zLevel;
	//	ndEnd = vtCentre;
	//	AddNode(ndEnd);		// was high feed rate
		SetFeedRate(m_FRRoute);

		ndCi = vtCentre;
		ndCi.y += cnodeDist;
	}
	else
		ASSERT(0);

	AddNode(ndCi);

	for (int iDepth = 0; iDepth <= numDepths; iDepth++)
	{
		zLevel = zTop - 0.5 * depthSeg - iDepth * depthStage;
		int iSegInit = 0;
		int iSegFinal = numSegs - 1;

		if (iDepth == numDepths)
		{
			zLevel += 0.5 * depthSeg;		// step down only 1/2 usual seg dist to flatten off on bottom
			depthSeg = 0;
			cnodeZDist = 0;
			iSegFinal++;				// finish 90 later on last depth
		}
		vtCentre.z = zLevel;
		for (int iSeg = iSegInit; iSeg <= iSegFinal; iSeg++)		// do numSeg-1 loops
		{
			double angE = angInit + angSeg * iSeg;		// end node angle
			double cosA = cos(angE);
			double sinA = sin(angE);
			ndEnd.Set(cosA*radius, sinA*radius, 0);
			ndCi.Set(sinA*cnodeDist, -cosA*cnodeDist, -cnodeZDist);		// rotate -90
			ndCi.Neg(ndCf);
			ndEnd += vtCentre;
			ndCi += ndEnd;
			ndCf += ndEnd;
			AddNode(ndCf);
			AddNode(ndEnd);
			AddNode(ndCi);
			zLevel -= depthSeg;
			vtCentre.z = zLevel;
		}

	}	// for (int iDepth...)

	if (nEndType == DIR_ZPOS && zDepth < 0.5)		// can't do if too shallow
		nEndType = DIR_YPOS;
	if (nEndType == DIR_ZPOS)	// last depth - do final move
	{
		// retract to top centre moving(0,0,1)
		vtCentre.z = zTop;
		ndCf = vtCentre;
		ndCf.z -= zDepth * cnodeRatio;
		ndEnd = vtCentre;
		AddNode(ndCf);
		AddNode(ndEnd);
	}
	else if (nEndType == DIR_YPOS)
	{
		// move to centre same at bottom moving(0,1,0)
		ndCf = vtCentre;
		ndCf.y -= cnodeDist;
		ndEnd = vtCentre;
		AddNode(ndCf);
		AddNode(ndEnd);
	}
	else
		ASSERT(0);

}


void CPathBuilder::HoleMatrixSquare()
{
	// Feed rates:
	// Clearance -> TopDrill -> BottomDrill -> TopDrill -> Clearance
	//         FRHigh      FRDrill        FRRetract   FRHigh

	if (!GetInitHoleLoc())
		return;						// no valid holes!

	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);
	m_vtHoleLoc.z = m_zClearance;
	MoveToXYZ(m_vtHoleLoc);

	for (;;)
	{
		MoveToZ(m_zTopDrill);		// high feed rate
		SetFeedRate(m_FRDrill);
		MoveToZ(m_zBottomDrill);
		SetFeedRate(m_FRRetract);
		MoveToZ(m_zTopDrill);
		SetFeedRate(m_FRRapid);
		MoveToZ(m_zClearance);
		if (!GetNextHoleLoc())
			break;
		MoveToXY(m_vtHoleLoc);
	}

	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);
}

void CPathBuilder::SetToStartMatrixHole()
{
	m_bAtMatrixStart = true;
}

bool CPathBuilder::MoveToNextMatrixHole()
{
	if (m_bAtMatrixStart)
	{
		m_bAtMatrixStart = false;
		if (!GetInitHoleLoc())
			return false;						// no valid holes!

		SetFeedRate(m_FRRapid);
		double zVal = m_vtHoleLoc.z;
		m_vtHoleLoc.z = m_zClearance;
		MoveToXYZ(m_vtHoleLoc);
		m_vtHoleLoc.z = zVal;
		MoveToZ(m_zTopDrill);
		return true;
	}
	else
	{
		SetFeedRate(m_FRRapid);
		NODEREF nr;
		CPathNode& ndLast = *m_pPath->GetLastNode(nr);
		if (fabs(m_zTopDrill - ndLast.z) > 0.1 || !ndLast.IsEndPoint())
			MoveToZ(m_zTopDrill);
		// finished drill, z at m_zTopDrill

		if (!GetNextHoleLoc())
		{
			MoveToZ(m_zClearance);
			return false;				// no more holes
		}
	}
	double zVal = m_vtHoleLoc.z;
	m_vtHoleLoc.z = m_zTopDrill;
	RadiusConnectZ(m_vtHoleLoc, m_zClearance);
	m_vtHoleLoc.z = zVal;

// ready for drill, z at m_zTopDrill
	return true;
}

void CPathBuilder::HoleMatrixCurved()
{
	// Feed rates:
	// Clearance -> TopDrill -> BottomDrill -> TopDrill -> Clearance
	//         FRHigh      FRDrill        FRRetract   FRHigh

	CVector vtPosServo;
	ofstream os("Hole Locations.txt");

	if (!GetInitHoleLoc())
		return;						// no valid holes!

	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);
	double zVal = m_vtHoleLoc.z;
	m_vtHoleLoc.z = m_zClearance;
	MoveToXYZ(m_vtHoleLoc);
	m_vtHoleLoc.z = zVal;

	MoveToZ(m_zTopDrill);
	for (;;)
	{
		g_PosConvert.GetPosServoFromPosHead(m_vtHoleLoc + m_vtPosMachPathOrigForCalc, vtPosServo);
		os << "   " << m_vtHoleLoc + m_vtPosMachPathOrigForCalc << endl;
		os << "-->" << vtPosServo << endl << endl;

		SetFeedRate(m_FRDrill);			// high feed rate
		MoveToZ(m_zBottomDrill);
		SetFeedRate(m_FRRetract);
		MoveToZ(m_zTopDrill);
		if (!GetNextHoleLoc())
			break;
		SetFeedRate(m_FRRapid);
		zVal = m_vtHoleLoc.z;
		m_vtHoleLoc.z = m_zTopDrill;
		RadiusConnectZ(m_vtHoleLoc, m_zClearance);
		m_vtHoleLoc.z = zVal;
	}

	SetFeedRate(m_FRRapid);
	MoveToZ(m_zClearance);
	MoveToXYZ(m_vtEnd);
}


bool CPathBuilder::GetInitHoleLoc()
{
	m_iS = 0;
	m_iT = 0;
	m_iStepS = 1;
	m_iStepT = 1;
	m_iS -= m_iStepS;
	return GetNextHoleLoc();
}

bool CPathBuilder::GetNextHoleLoc()
{
	// scans back and foward checking for OK locations
	for (;;)
	{
		m_iS += m_iStepS;
		GetHoleLoc();
		if (CheckHoleLocExtents() != 0)
		{
			m_iStepS = - m_iStepS;
			m_iS += m_iStepS;			// bring back to allowed pos
			m_iT += m_iStepT;
			GetHoleLoc();
			if (CheckHoleLocExtents() != 0)
				return false;
		}
		if (CheckHoleLoc(m_vtHoleLoc))
			break;
	}
	return true;
}
/*
bool CPathBuilder::GetNextHoleLocOld()
{
	// scans back and foward checking for OK locations
	for (;;)
	{
		m_iS += m_iStepS;
		if (m_iS < m_iSMin || m_iS > m_iSMax)
		{
			m_iStepS = - m_iStepS;
			m_iS += m_iStepS;			// bring back to allowed pos
			m_iT += m_iStepT;
			if (m_iT < m_iTMin || m_iT > m_iTMax)
				return false;
		}
		if (GetHoleLoc())
			return true;
	}
}
*/
void CPathBuilder::GetHoleLoc()
{
	m_vtHoleLoc = m_vtExtentMin + (m_vtDelS * m_iS) + (m_vtDelT * m_iT);
}

int CPathBuilder::CheckHoleLocExtents()
{
	int i = 0;
	if (m_vtHoleLoc.x < m_vtExtentMin.x-0.1 || m_vtHoleLoc.x > m_vtExtentMax.x+0.1)
		i++;
	if (m_vtHoleLoc.y < m_vtExtentMin.y-0.1 || m_vtHoleLoc.y > m_vtExtentMax.y+0.1)
		i++;
	return i;
}


bool CPathBuilder::CheckHoleLoc(CVector& vt)
{
	return true;
	if (vt.y < 100 || vt.x > 1500)
		return true;
	return false;
	return true;

	double radius = 200;
	double radiusSq = radius*radius;
	if (vt.x*vt.x + vt.y*vt.y <= radiusSq)
		return true;

	return false;
}

void CPathBuilder::VacuumBoardTrenching()
{
	SetDefaults();
	m_FRRoute = 25;

/////////////////////////////////////////////////////
// parameters
	CVect2 vtGrid = 50;


	CVect2 vtExtentMin(0, 0);
	CVect2 vtExtentMax(1800, 850);


	// z = 0 is bottom of stock
	double dThickness = 12;
	double dClearance = 4;
	double dHoleOverShoot = 1;
	



/////////////////////////////////////////////////////
// calculated parameters

	double zHoleBottom = -dHoleOverShoot;
	double zClearance = dThickness + dClearance;

	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);

	SetFeedRate(m_FRRoute);



	int x, y, dx, dy;
	x = 0;
	y = 0;
	dx = 1;
	dy = -1;

	CPoint ptMin(0, 0);
	CPoint ptMax(7, 4);

if (0)
	for (;;)
	{
		//fn(x,y);


		x += dx;
		y += dy;
		bool bXOut = (x < ptMin.x || x > ptMax.x);
		bool bYOut = (y < ptMin.y || y > ptMax.y);
		if (bXOut && bYOut)
		{
			dx = -dx;
			dy = -dy;
			x += dx;		// back track
			y += dy;

		}
		else if (bYOut)
		{
			dy = -dy;
			y += dy;			// y now same as last
			x += dx;
			dx = -dx;
			bXOut = (x < ptMin.x || x > ptMax.x);
			if (bXOut)
			{
				x += dx;		// should be OK now!
				y += dy;
			}
		}
		else if (bXOut)
		{
			dx = -dx;
			x += dx;			// x now same as last
			y += dy;
			dy = -dy;
			bYOut = (y < ptMin.y || y > ptMax.y);
			if (bYOut)
			{
				x += dx;
				y += dy;		// should be OK now!
			}
		}
		//else		// point OK
	}


	CPoint szSpan = ptMax - ptMin;
	int numPoints = (szSpan.x * szSpan.y);

	for (int i = 0; i <= numPoints; i++)
	{
		m_vtHoleLoc.Set(x*vtGrid.x, y*vtGrid.y, 0);
		MoveToXYZ(m_vtHoleLoc);

		x += dx;
		y += dy;
		bool bXOut = (x < ptMin.x || x > ptMax.x);
		bool bYOut = (y < ptMin.y || y > ptMax.y);
		if (bXOut)
		{
			dx = -dx;
			x += 2*dx;
		}
		if (bYOut)
		{
			dy = -dy;
			y += 2*dy;
		}
	}

	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);

}

void CPathBuilder::VacuumBoardStarArray()
{
	SetDefaults();
	m_FRRoute = 25;

/////////////////////////////////////////////////////
// parameters
	CVect2 vtGrid = 50;

	CVect2 vtStarWidth = 30;
	CVect2 vtStarDepth = 2;
	CVect2 vtSlopeWidth = 10;

	CVect2 vtExtentMin(0, 0);
	CVect2 vtExtentMax(1800, 850);


	// z = 0 is bottom of stock
	double dThickness = 12;
	double dClearance = 4;
	double dHoleOverShoot = 1;
	
	double ratioControlSlope = 0.4;		// <= 0.5



/////////////////////////////////////////////////////
// calculated parameters

	double zHoleBottom = -dHoleOverShoot;
	CVect2 vtzRampBottom = dThickness - vtStarDepth;
	CVect2 vtSlopeControl = vtSlopeWidth * ratioControlSlope;
	double zClearance = dThickness + dClearance;


/////////////////////////////////////////////////////

	CPathNode nd[20];
	CPathNode ndAdd[20];
	CPathNode ndEnd[4];
	CPathNode ndEndAdd[4];
	CPathNode ndExt[8];
	CPathNode ndExtAdd[8];
	CPathNode ndExtEnd[4];
	CPathNode ndExtEndAdd[4];





// Start

	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);			// starts above 0,0
	double FRPrev = GetFeedRate();


	int i;
	int axLine = 0;
	int axPerp = 1 - axLine;

	
	// corner end nodes without extension - first node is away from line
	i = 0;
	int numEndNodes = i;
	ASSERT(i <= sizeof(ndEnd)/sizeof(*ndEnd));


	// corner end nodes with extension - first node is away from line
	i = 0;
/*	ndExtEnd[i].type = PNT_SETFEEDRATE;
	ndExtEnd[i].y = m_FRRoute;
	ndExtEnd[i].x = m_FRRapid;
	ndExtEnd[i].z = 0;
	i++;
*/	ndExtEnd[i].type = PNT_ENDNODE;
	ndExtEnd[i] = 0;
	ndExtEnd[i].z = zClearance;
	i++;
	int numExtEndNodes = i;
	ASSERT(i <= sizeof(ndExtEnd)/sizeof(*ndExtEnd));


	// extension nodes - first node is outside edge
	i = 0;
	int numExtNodes = i;
	ASSERT(i <= sizeof(ndExt)/sizeof(*ndExt));


	
	// star nodes relative to hole - first node is min value
	i = 0;
	nd[i].type = PNT_ENDNODE;
	nd[i] = - vtStarWidth - vtSlopeWidth;
	nd[i].z = zClearance;
	i++;
	nd[i].type = PNT_SETFEEDRATE;
	nd[i].y = FRPrev;
	nd[i].x = FRPrev = m_FRRoute;
	nd[i].z = 0;
	i++;
	nd[i].type = PNT_CONTROLNODE;
	nd[i] = - vtStarWidth - vtSlopeWidth + vtSlopeControl;
	nd[i].z = zClearance;
	i++;
	nd[i].type = PNT_CONTROLNODE;
	nd[i] = - vtStarWidth - vtSlopeControl;
	nd[i].z = vtzRampBottom[axLine];
	i++;
	nd[i].type = PNT_ENDNODE;
	nd[i] = - vtStarWidth;
	nd[i].z = vtzRampBottom[axLine];
	i++;
	nd[i].type = PNT_ENDNODE;
	nd[i] = 0;
	nd[i].z = vtzRampBottom[axLine];
	int idxHalfStarInit = i;
	i++;
	int idxRampDrillInit = i;
	nd[i].type = PNT_SETFEEDRATE;		// for drill only
	nd[i].y = FRPrev;
	nd[i].x = FRPrev = m_FRDrill;
	nd[i].z = 0;
	i++;
	nd[i].type = PNT_ENDNODE;			// for drill only
	nd[i] = 0;
	nd[i].z = zHoleBottom;
	i++;
	nd[i].type = PNT_SETFEEDRATE;		// for drill only
	nd[i].y = FRPrev;
	nd[i].x = FRPrev = m_FRRoute;
	nd[i].z = 0;
	i++;
	nd[i].type = PNT_ENDNODE;			// for drill only
	nd[i] = 0;
	nd[i].z = vtzRampBottom[axLine];
	int idxHalfStarFinal = i;
	i++;
	int idxRampDrillFinal = i;
	nd[i].type = PNT_ENDNODE;
	nd[i] = vtStarWidth;
	nd[i].z = vtzRampBottom[axLine];
	i++;
	nd[i].type = PNT_CONTROLNODE;
	nd[i] = vtStarWidth + vtSlopeControl;
	nd[i].z = vtzRampBottom[axLine];
	i++;
	nd[i].type = PNT_CONTROLNODE;
	nd[i] = vtStarWidth + vtSlopeWidth - vtSlopeControl;
	nd[i].z = zClearance;
	i++;
	nd[i].type = PNT_ENDNODE;
	nd[i] = vtStarWidth + vtSlopeWidth;
	nd[i].z = zClearance;
	i++;
	nd[i].type = PNT_SETFEEDRATE;
	nd[i].y = FRPrev;
	nd[i].x = FRPrev = m_FRRapid;
	nd[i].z = 0;
	i++;
	
	ASSERT(i <= sizeof(nd)/sizeof(*nd));
	int numRampNodes = i;


	for (i = 0; i < numEndNodes; i++)
		ndEndAdd[i] = ndEnd[i];
	for (i = 0; i < numExtNodes; i++)
		ndExtAdd[i] = ndExt[i];
	for (i = 0; i < numExtEndNodes; i++)
		ndExtEndAdd[i] = ndExtEnd[i];


	int iHole = 0;
	int iLineDir = 1;
	int iLine = 0;
	int iPerpDir = 1;

//	CVect2 vtGridLoc;

for (int iPass = 0;; iPass++)
{
	for (;;)
	{
		int nDoEven = (iLine & 1) == 0;		// do even holes on an even line, odd holes on an odd line

		double posPerpBase = vtExtentMin[axPerp] + iLine * vtGrid[axPerp];
		double posPerpMin = posPerpBase - vtGrid[axPerp];
		double posPerpMax = posPerpBase + vtGrid[axPerp];
//		vtGridLoc[axPerp] = posPerpBase;

		// check if last line
		if (posPerpBase > vtExtentMax[axPerp]+1 || posPerpBase < vtExtentMin[axPerp]-1)
		{
			iLine -= iPerpDir;	// set back to valid value
			break;
		}

		for (i = 0; i < numRampNodes; i++)
		{
			ndAdd[i] = nd[i];
			if (nd[i].IsPoint())
				ndAdd[i][axPerp] = posPerpBase;
		}

		bool bStartLine = true;
		for (;; iHole += iLineDir)		// loop along line
		{
			double posLineBase = vtExtentMin[axLine] + iHole * vtGrid[axLine];
			double posLineMin = posLineBase - vtGrid[axLine];
			double posLineMax = posLineBase + vtGrid[axLine];
//			vtGridLoc[axLine] = posLineBase;

			bool bSkip = (iHole & 1) == nDoEven;
			// check if end of line
			if (posLineMax > vtExtentMax[axLine]+1 || posLineMin < vtExtentMin[axLine]-1)	// allow 1mm for tolerance
			{
				if (bStartLine)	// start of line
				{
					if (bSkip)			// just add corner
						for (i = 0; i < numEndNodes; i++)		// add corner first if start of line
						{
							ndEndAdd[i][axLine] = posLineBase;
							ndEndAdd[i][axPerp] = posPerpBase - iPerpDir * ndEnd[i][axPerp];
							AddNode(ndEndAdd[i]);
						}
					else				// add extension after corner if required
					{
						for (i = 0; i < numExtEndNodes; i++)		// add corner first if start of line
						{
							ndExtEndAdd[i][axLine] = posLineBase;
							ndExtEndAdd[i][axPerp] = posPerpBase - iPerpDir * ndExtEnd[i][axPerp];
							AddNode(ndExtEndAdd[i]);
						}
						AddNode(nd[1]);		// Feedrate Route from Rapid
						for (i = idxHalfStarInit; i < numRampNodes; i++)
						{
							if (nd[i].IsPoint())
								ndAdd[i][axLine] = posLineBase + iLineDir * nd[i][axLine];
							AddNode(ndAdd[i]);
						}
/*						
						for (i = 0; i < numExtNodes; i++)
						{
							ndExtAdd[i][axLine] = posLineBase + iLineDir * ndExt[i][axLine];
							ndExtAdd[i][axPerp] = posPerpBase;
							AddNode(ndExtAdd[i]);
						}
*/
					}
					continue;
				}
				else		// end of line
				{
					if (!bSkip)			// add extension before corner if required
					{
/*
						for (i = numExtNodes-1; i >= 0; i--)
						{
							ndExtAdd[i][axLine] = posLineBase - iLineDir * ndExt[i][axLine];
							ndExtAdd[i][axPerp] = posPerpBase;
							AddNode(ndExtAdd[i]);
						}
*/
						for (i = 0; i <= idxHalfStarFinal; i++)
						{
							if (nd[i].IsPoint())
								ndAdd[i][axLine] = posLineBase + iLineDir * nd[i][axLine];
							AddNode(ndAdd[i]);
						}
						for (i = numExtEndNodes-1; i >= 0; i--)		// add corner after if end of line
						{
							ndExtEndAdd[i][axLine] = posLineBase;
							ndExtEndAdd[i][axPerp] = posPerpBase + iPerpDir * ndExtEnd[i][axPerp];
							AddNode(ndExtEndAdd[i]);
						}
						AddNode(nd[numRampNodes-1]);		// Feedrate Rapid from Route
					}
					else			// just add corner
						for (i = numEndNodes-1; i >= 0; i--)		// add corner after if end of line
						{
							ndEndAdd[i][axLine] = posLineBase;
							ndEndAdd[i][axPerp] = posPerpBase + iPerpDir * ndEnd[i][axPerp];
							AddNode(ndEndAdd[i]);
						}
					break;
				}
			}
			else if (bSkip)		// position OK
				continue;


			bStartLine = false;
			for (i = 0; i < numRampNodes; i++)
			{
				if (nd[i].IsPoint())
					ndAdd[i][axLine] = posLineBase + iLineDir * nd[i][axLine];
				AddNode(ndAdd[i]);
			}
		}	// for all holes on line

		iLine += iPerpDir;
		iLineDir = -iLineDir;
	}	// for all lines of one direction

	if (iPass == 1)
		break;

	// remove drill only nodes from ramp nodes
	int iDest = idxRampDrillInit;
	int iSrc = idxRampDrillFinal;
	int numRemove = iSrc - iDest;
	for (; iSrc < numRampNodes; iSrc++, iDest++)
		nd[iDest] = nd[iSrc];
	numRampNodes = iDest;
	idxRampDrillFinal = idxRampDrillInit;
	idxHalfStarFinal -= numRemove;		// needed for half ramps, now with no drill

	// swap axis
	int iTmp;
	iTmp = axLine;
	axLine = axPerp;
	axPerp = iTmp;

	iTmp = iPerpDir;
	iPerpDir = iLineDir;
	iLineDir = -iTmp;

	iTmp = iLine;
	iLine = iHole;
	iHole = iTmp;

}

	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);

}


void CPathBuilder::VacuumBoardRampStarArray()
{
	SetDefaults();
	m_FRRoute = 25;

/////////////////////////////////////////////////////
// parameters
	CVect2 vtGrid = 50;
	CVect2 vtRampLength = 35;
	CVect2 vtRampLengthExtra = 5;
	CVect2 vtRampDepth = 5;

	CVect2 vtExtentMin(0, 0);
	CVect2 vtExtentMax(1500, 800);


	double dThickness = 12;
	double dClearance = 4;
	double dHoleOverShoot = 1;
	
	double ratioControlTop = 0.3;		// <= 0.5
	CVect2 vtRadExtLine = 30;



/////////////////////////////////////////////////////
// calculated parameters

	double zHoleBottom = -dHoleOverShoot;
	CVect2 vtzRampBottom = dThickness - vtRampDepth;
	CVect2 vtzRampTop    = dThickness + (vtRampDepth/vtRampLength) * vtRampLengthExtra;
	CVect2 vtRampWidth = vtRampLength + vtRampLengthExtra;

	CVect2 vtTopCurveWidth = 2 * (vtGrid - vtRampWidth);
	CVect2 vtControlRelTop = vtTopCurveWidth * ratioControlTop;				// relative to end node
	CVect2 vtzControlRelTop = vtControlRelTop * (vtRampDepth/vtRampLength);		// relative to end node
	CVect2 vtzControlTop = vtzRampTop + vtzControlRelTop;

	double zClearance = dThickness + dClearance;


/////////////////////////////////////////////////////

	CPathNode nd[20];
	CPathNode ndAdd[20];
	CPathNode ndEnd[4];
	CPathNode ndEndAdd[4];
	CPathNode ndExt[8];
	CPathNode ndExtAdd[8];
	CPathNode ndExtEnd[4];
	CPathNode ndExtEndAdd[4];





// Start

	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtStart);			// starts above 0,0
	double FRPrev = GetFeedRate();


	int i;
	int axLine = 0;
	int axPerp = 1 - axLine;

	
	// corner end nodes without extension - first node is away from line
	i = 0;
	ndEnd[i].type = PNT_ENDNODE;
	ndEnd[i] = 0.5 * vtTopCurveWidth;
	ndEnd[i].z = zClearance;
	i++;
	ndEnd[i].type = PNT_CONTROLNODE;
	ndEnd[i] = vtTopCurveWidth * (0.5 - ratioControlTop);
	ndEnd[i].z = zClearance;
	i++;
	ASSERT(i <= sizeof(ndEnd)/sizeof(*ndEnd));
	int numEndNodes = i;


	// corner end nodes with extension - first node is away from line
	i = 0;
	ndExtEnd[i].type = PNT_ENDNODE;
	ndExtEnd[i] = vtRadExtLine;
	ndExtEnd[i].z = zClearance;
	i++;
	ndExtEnd[i].type = PNT_CONTROLNODE;
	ndExtEnd[i] = vtRadExtLine * (1 - 2*ratioControlTop);
	ndExtEnd[i].z = zClearance;
	i++;
	ASSERT(i <= sizeof(ndExtEnd)/sizeof(*ndExtEnd));
	int numExtEndNodes = i;


	// extension nodes - first node is outside edge
	i = 0;
	ndExt[i].type = PNT_CONTROLNODE;
	ndExt[i] = vtRadExtLine * (1 - 2*ratioControlTop);
	ndExt[i].z = zClearance;
	i++;
	ndExt[i].type = PNT_ENDNODE;
	ndExt[i] = vtRadExtLine;
	ndExt[i].z = zClearance;
	i++;
	ndExt[i].type = PNT_ENDNODE;
	ndExt[i] = vtRampWidth;
	ndExt[i].z = zClearance;
	i++;
	ndExt[i].type = PNT_CONTROLNODE;
	ndExt[i] = vtRampWidth + vtControlRelTop;
	ndExt[i].z = zClearance;
	i++;
	ASSERT(i <= sizeof(ndExt)/sizeof(*ndExt));
	int numExtNodes = i;



	
	// star nodes relative to hole - first node is min value
	i = 0;
	nd[i].type = PNT_CONTROLNODE;
	nd[i] = - vtRampWidth - vtControlRelTop;
	nd[i].z = vtzControlTop[axLine];
	i++;
	nd[i].type = PNT_ENDNODE;
	nd[i] = - vtRampWidth;
	nd[i].z = vtzRampTop[axLine];
	i++;
	nd[i].type = PNT_SETFEEDRATE;
	nd[i].y = FRPrev;
	nd[i].x = FRPrev = m_FRRoute;
	nd[i].z = 0;
	i++;
	nd[i].type = PNT_ENDNODE;
	nd[i] = 0;
	nd[i].z = vtzRampBottom[axLine];
	i++;
	int idxRampDrillInit = i;
	nd[i].type = PNT_SETFEEDRATE;		// for drill only
	nd[i].y = FRPrev;
	nd[i].x = FRPrev = m_FRDrill;
	nd[i].z = 0;
	i++;
	nd[i].type = PNT_ENDNODE;			// for drill only
	nd[i] = 0;
	nd[i].z = zHoleBottom;
	i++;
	nd[i].type = PNT_SETFEEDRATE;		// for drill only
	nd[i].y = FRPrev;
	nd[i].x = FRPrev = m_FRRoute;
	nd[i].z = 0;
	i++;
	nd[i].type = PNT_ENDNODE;			// for drill only
	nd[i] = 0;
	nd[i].z = vtzRampBottom[axLine];
	i++;
	int idxRampDrillFinal = i;
	nd[i].type = PNT_ENDNODE;
	nd[i] = vtRampWidth;
	nd[i].z = vtzRampTop[axLine];
	i++;
	nd[i].type = PNT_SETFEEDRATE;
	nd[i].y = FRPrev;
	nd[i].x = FRPrev = m_FRRapid;
	nd[i].z = 0;
	i++;
	nd[i].type = PNT_CONTROLNODE;
	nd[i] = vtRampWidth + vtControlRelTop;
	nd[i].z = vtzControlTop[axLine];
	i++;
	ASSERT(i <= sizeof(nd)/sizeof(*nd));
	int numRampNodes = i;


	for (i = 0; i < numEndNodes; i++)
		ndEndAdd[i] = ndEnd[i];
	for (i = 0; i < numExtNodes; i++)
		ndExtAdd[i] = ndExt[i];
	for (i = 0; i < numExtEndNodes; i++)
		ndExtEndAdd[i] = ndExtEnd[i];


	int iHole = 0;
	int iLineDir = 1;
	int iLine = 0;
	int iPerpDir = 1;

//	CVect2 vtGridLoc;

for (int iPass = 0;; iPass++)
{
	for (;;)
	{
		iLine += iPerpDir;
		int nDoEven = (iLine & 1) == 0;		// do even holes on an even line, odd holes on an odd line

		double posPerpBase = vtExtentMin[axPerp] + iLine * vtGrid[axPerp];
		double posPerpMin = posPerpBase - vtGrid[axPerp];
		double posPerpMax = posPerpBase + vtGrid[axPerp];
//		vtGridLoc[axPerp] = posPerpBase;

		// check if last line
		if (posPerpMax > vtExtentMax[axPerp]+1 || posPerpMin < vtExtentMin[axPerp]-1)
			break;

		for (i = 0; i < numRampNodes; i++)
		{
			ndAdd[i] = nd[i];
			if (nd[i].IsPoint())
				ndAdd[i][axPerp] = posPerpBase;
		}

		bool bStartLine = true;
		for (;; iHole += iLineDir)		// loop along line
		{
			double posLineBase = vtExtentMin[axLine] + iHole * vtGrid[axLine];
			double posLineMin = posLineBase - vtGrid[axLine];
			double posLineMax = posLineBase + vtGrid[axLine];
//			vtGridLoc[axLine] = posLineBase;

			bool bSkip = (iHole & 1) == nDoEven;
			// check if end of line
			if (posLineMax > vtExtentMax[axLine]+1 || posLineMin < vtExtentMin[axLine]-1)	// allow 1mm for tolerance
			{
				if (bStartLine)	// start of line
				{
					if (bSkip)			// just add corner
						for (i = 0; i < numEndNodes; i++)		// add corner first if start of line
						{
							ndEndAdd[i][axLine] = posLineBase;
							ndEndAdd[i][axPerp] = posPerpBase - iPerpDir * ndEnd[i][axPerp];
							AddNode(ndEndAdd[i]);
						}
					else				// add extension after corner if required
					{
						for (i = 0; i < numExtEndNodes; i++)		// add corner first if start of line
						{
							ndExtEndAdd[i][axLine] = posLineBase;
							ndExtEndAdd[i][axPerp] = posPerpBase - iPerpDir * ndExtEnd[i][axPerp];
							AddNode(ndExtEndAdd[i]);
						}
						for (i = 0; i < numExtNodes; i++)
						{
							ndExtAdd[i][axLine] = posLineBase + iLineDir * ndExt[i][axLine];
							ndExtAdd[i][axPerp] = posPerpBase;
							AddNode(ndExtAdd[i]);
						}
					}
					continue;
				}
				else		// end of line
				{
					if (!bSkip)			// add extension before corner if required
					{
						for (i = numExtNodes-1; i >= 0; i--)
						{
							ndExtAdd[i][axLine] = posLineBase - iLineDir * ndExt[i][axLine];
							ndExtAdd[i][axPerp] = posPerpBase;
							AddNode(ndExtAdd[i]);
						}
						for (i = numExtEndNodes-1; i >= 0; i--)		// add corner after if end of line
						{
							ndExtEndAdd[i][axLine] = posLineBase;
							ndExtEndAdd[i][axPerp] = posPerpBase + iPerpDir * ndExtEnd[i][axPerp];
							AddNode(ndExtEndAdd[i]);
						}
					}
					else			// just add corner
						for (i = numEndNodes-1; i >= 0; i--)		// add corner after if end of line
						{
							ndEndAdd[i][axLine] = posLineBase;
							ndEndAdd[i][axPerp] = posPerpBase + iPerpDir * ndEnd[i][axPerp];
							AddNode(ndEndAdd[i]);
						}
					break;
				}
			}
			else if (bSkip)		// position OK
				continue;


			bStartLine = false;
			for (i = 0; i < numRampNodes; i++)
			{
				if (nd[i].IsPoint())
					ndAdd[i][axLine] = posLineBase + iLineDir * nd[i][axLine];
				AddNode(ndAdd[i]);
			}
		}	// for all holes on line

		iLineDir = -iLineDir;
	}	// for all lines of one direction

	if (iPass == 1)
		break;

	// remove drill only nodes from ramp nodes
	int iDest = idxRampDrillInit;
	int iSrc = idxRampDrillFinal;
	for (; iSrc < numRampNodes; iSrc++, iDest++)
		nd[iDest] = nd[iSrc];
	numRampNodes = iDest;

	// swap axis
	int iTmp;
	iTmp = axLine;
	axLine = axPerp;
	axPerp = iTmp;

	iTmp = iPerpDir;
	iPerpDir = iLineDir;
	iLineDir = -iTmp;

	iTmp = iLine;
	iLine = iHole;
	iHole = iTmp;

}

	SetFeedRate(m_FRRapid);
	MoveToXYZ(m_vtEnd);

}



void CPathBuilder::SetFeedRate(double feedrate)
{
	CPathNode nd;
	nd.type = PNT_SETFEEDRATE;
	nd.x = feedrate;
	nd.y = m_CurrPathProps.feedRate;
	nd.z = 0;
	m_CurrPathProps.feedRate = feedrate;
	AddNode(nd);
}

void CPathBuilder::MoveToZ(double z)
{
	m_ndCurrentLoc.z = z;
	m_ndCurrentLoc.type = PNT_ENDNODE;
	AddNode(m_ndCurrentLoc);
}

void CPathBuilder::MoveToXY(CVector& vt)
{
	m_ndCurrentLoc.x = vt.x;
	m_ndCurrentLoc.y = vt.y;
	m_ndCurrentLoc.type = PNT_ENDNODE;
	AddNode(m_ndCurrentLoc);
}

void CPathBuilder::MoveToXYZ(CVector& vt)
{
	m_ndCurrentLoc = vt;
	m_ndCurrentLoc.type = PNT_ENDNODE;
	AddNode(m_ndCurrentLoc);
}

void CPathBuilder::SmoothConnectZ(CVector& vtEnd, double zTop)
{
/*
	Connect from current node in current direction to new node vt vertically with 2 beziers
	curves upwards to max height of z
	start and end of curve is vertical, join of the 2 beziers is horizontal
	control nodes distances from associated end nodes:
		0.5 bezier height
		1.0 bezier width for no curve at connecting ends
		0.8 bezier width for rounder curve
*/
	CPathNode ndMid, ndC1, ndC2;
	ndMid.type = PNT_ENDNODE;
	ndC1.type = ndC2.type = PNT_CONTROLNODE;

	ndMid = (m_ndCurrentLoc + vtEnd) / 2;		// average position
	ndMid.z = zTop;

	double height = zTop - m_ndCurrentLoc.z;
	CVector vtWidth = ndMid - m_ndCurrentLoc;
	vtWidth.z = 0;
	ndC1 = CVector(m_ndCurrentLoc);
	ndC1.z += 0.5 * height;
	ndC2 = ndMid - 0.8 * vtWidth; 
	AddNode(ndC1);
	AddNode(ndC2);
	AddNode(ndMid);

	height = zTop - vtEnd.z;
	ndC1 = ndMid + 0.8 * vtWidth; 
	ndC2 = vtEnd;
	ndC2.z += 0.5 * height;
	m_ndCurrentLoc = vtEnd;
	AddNode(ndC1);
	AddNode(ndC2);
	AddNode(m_ndCurrentLoc);

}

void CPathBuilder::RadiusConnectZ(CVector& vtEnd, double zTop)
{
/*
	Connect from current node in current direction to new node vt vertically with bezier-line-bezier
	curves upwards to max height of z
	start and end of curve is vertical, join of the 2 beziers is horizontal
	control nodes distances from associated end nodes:
		0.5 bezier height
		1.0 bezier width for no curve at connecting ends
		0.8 bezier width for rounder curve
*/
	double ratioControl = 2 * 0.3;
	CPathNode ndMid, ndC1, ndC2;
	ndMid.type = PNT_ENDNODE;
	ndC1.type = ndC2.type = PNT_CONTROLNODE;


//	ndMid = (m_ndCurrentLoc + vtEnd) / 2;		// average position
//	ndMid.z = zTop;

	CVector vtChangeDir = vtEnd - m_ndCurrentLoc;
	vtChangeDir.z = 0;
	vtChangeDir.Unit();
	double radius;

	radius = zTop - m_ndCurrentLoc.z;		// radius of first curve
	ndMid = m_ndCurrentLoc + vtChangeDir * radius;
	ndMid.z = zTop;
	ndC1 = CVector(m_ndCurrentLoc);
	ndC1.z += ratioControl * radius;
	ndC2 = ndMid - vtChangeDir * (radius * ratioControl); 
	AddNode(ndC1);
	AddNode(ndC2);
	AddNode(ndMid);

	radius = zTop - vtEnd.z;		// radius of second curve
	ndMid = vtEnd - vtChangeDir * radius;
	ndMid.z = zTop;
	ndC1 = ndMid + vtChangeDir * (radius * ratioControl); 
	ndC2 = vtEnd;
	ndC2.z += ratioControl * radius;
	m_ndCurrentLoc = vtEnd;
	AddNode(ndMid);
	AddNode(ndC1);
	AddNode(ndC2);
	AddNode(m_ndCurrentLoc);

}



