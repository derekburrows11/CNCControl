// PathBuilder.h: interface for the CPathBuilder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHBUILDER_H__08361F80_73B1_11D8_86C3_0008A15E291C__INCLUDED_)
#define AFX_PATHBUILDER_H__08361F80_73B1_11D8_86C3_0008A15E291C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


//#include "Vector.h"
#include "PathDoc.h"

//class CPathDoc;



class CPathBuilder  
{
public:
	CPathBuilder();
	CPathBuilder(CPathDoc* pDoc);
	virtual ~CPathBuilder();

	void SetPathDoc(CPathDoc* pDoc) { m_pPath = pDoc; }
	const char* GetTitle() { return m_strTitle; }
	void Build();

	void HoleMatrixGeneral();
	void HoleMatrixServoCalibration();

	void BaseScrewsArray();
	void BaseVacuumHoseSingle();
	void BaseVacuumHoseArray();
	void ManifoldVacuumBoxArray();
	void ManifoldVacuumBoxSingle();
	void VacuumBoardStarArray();
	void VacuumBoardRampStarArray();
	void VacuumBoardTrenching();

	void DepthCheck();

	void Rectangle();
	void Rectangle(double dx, double dy);
	void Circle(double diameter);

	void Boatshed(int iSize);
	void Heart1(double width);
	void Heart2(double wInside, double wOutside);
	void Minaret();

	enum
	{
		SIZE_SMALL = 1,
		SIZE_MEDIUM,
		SIZE_LARGE,
	};

protected:
	void SetDefaults();
	void SetToolDiameterMM(double diaMM);
	void SetToolDiameterInch(double diaInch);


	enum
	{
		DIR_XNEG = 1,
		DIR_XPOS,
		DIR_YNEG,
		DIR_YPOS,
		DIR_ZNEG,
		DIR_ZPOS,
	};

//	void DoHole();
	void HoleDrill(double zTop, double zBottom);
	void HoleMillByZLevel(double diameter, double zTop, double zBottom, int nStartType = DIR_ZNEG, int nEndType = DIR_ZPOS);
	void HoleHelix(double diameter, double zTop, double zBottom, int nStartType = DIR_ZNEG, int nEndType = DIR_ZPOS);

	void HoleMatrixSquare();
	void HoleMatrixCurved();

	void SetToStartMatrixHole();
	bool MoveToNextMatrixHole();


	void GetHoleLoc();
	bool GetInitHoleLoc();
	bool GetNextHoleLoc();
	int CheckHoleLocExtents();
	bool CheckHoleLoc(CVector& vt);



	void SetFeedRate(double feedrate);
	double GetFeedRate() { return m_CurrPathProps.feedRate; }
	void MoveToZ(double z);
	void MoveToXY(CVector& vt);
	void MoveToXYZ(CVector& vt);
	void SmoothConnectZ(CVector& vtEnd, double zTop);
	void RadiusConnectZ(CVector& vtEnd, double zTop);
	void AddNode(CPathNode& nd) { m_pPath->AddPathNode(nd); }
	void AddEndNode(CPathNode& nd) { nd.type = PNT_ENDNODE; m_pPath->AddPathNode(nd); }
	CPathNode& GetNode(NODEREF nr) { return *m_pPath->GetPathNode(nr); }
	void RemoveLastNode() { m_pPath->RemoveLastNode(); }
	void RemoveNode(NODEREF nr) { m_pPath->RemoveNode(nr); }



protected:
	CPathDoc* m_pPath;
	CPathNode m_ndCurrentLoc;

	CString m_strTitle;

	double m_FRDrill;
	double m_FRRoute;
	double m_FRRetract;
	double m_FRRapid;


	double m_zClearance;
	double m_zTopDrill;
	double m_zBottomDrill;
	double m_zTopOfStock;
	double m_zBottomOfStock;
	double m_arZLevel[50];
	int m_numZLevels;

	double m_diaTool;
	double m_radTool;

	bool m_bAtMatrixStart;
	int m_iS;
	int m_iT;
	int m_iStepS;
	int m_iStepT;
//	int m_iSMin, m_iSMax;
//	int m_iTMin, m_iTMax;

	CVector m_vtExtentMin;
	CVector m_vtExtentMax;

	CVector m_vtStart;
	CVector m_vtEnd;
	CVector m_vtDelS;
	CVector m_vtDelT;

	CVector m_vtHoleLoc;
	CVector m_vtPosMachPathOrigForCalc;

	SPathProperties m_CurrPathProps;

};

#endif // !defined(AFX_PATHBUILDER_H__08361F80_73B1_11D8_86C3_0008A15E291C__INCLUDED_)
