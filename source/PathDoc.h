#if !defined(AFX_PATHDOC_H__B0E03CA1_FD9F_11D4_8C1E_C239773B3A2C__INCLUDED_)
#define AFX_PATHDOC_H__B0E03CA1_FD9F_11D4_8C1E_C239773B3A2C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include <afxtempl.h>
#include "Vector.h"
//#include "Matrix.h"
#include "PathDataObjects.h"

#include "SettingsData.h"

class CPathSpeedDoc;


enum		// Path Node Type
{
	// Point nodes
	PNTF_NODE = 0x0100,
	PNTF_NODEHIDDEN = 0x0200,
	PNTF_END = 0x01,
	PNTF_CONTROL = 0x02,
	PNT_ENDNODE = PNTF_NODE | PNTF_END,
	PNT_CONTROLNODE = PNTF_NODE | PNTF_CONTROL,
	PNTF_ABSOLUTE = 0x04,
	PNTF_RELATIVE = 0x08,

	PNTF_STARTPASS = 0x10,
	PNTF_ENDPASS = 0x20,

	// Command nodes
	PNTF_COMMAND = 0x1000,		// All Command codes have this bit set (selection int in x)
	PNT_SETDRIVEPROP,				// Set Drive Properties -  s of segment for change in z=[0,1]
	PNT_SETFEEDRATE,				// Set Feed Rate - next rate in x, previous rate in y, -1 for rapid feedrate, s of segment for change in z=[0,1]
	PNT_SELECTTOOL,				// Select Tool - next tool number in x, previous in y, s of change in z
	PNT_PROGRAMNUMBER,			// NC program number in x
//	PNT_JOININGTAB,				// Leave a joining tab start rise up from s of segment in z
	PNT_BREAKPATH,					// Break path, restart from next node
};

enum		// Path Node Values
{
	FEEDRATE_RAPID = -1,
	FEEDRATE_DEFAULT = FEEDRATE_RAPID,

	TOOL_DEFAULT = 1,
};



enum PlaneType
{
	PT_NOTSET = 0,
	PT_PLANAR,
	PT_NONPLANAR,
};
enum Dir
{
	DIR_FREE = 0,
	DIR_MATCHED,
	DIR_SET,
};
enum Loc
{
	LOC_FREE = 0,
	LOC_RANGE,
	LOC_FIXED,
};
enum CurveChange		// curve change reasons
{
	CC_NOTSET = 0,
	CC_ENDOFPLANE,
	CC_NONPLANAR2PLANAR,
	CC_DIRCHANGEOVER,
	CC_CURVESTEPCHANGE,
	CC_LASTNODE,
};






class CPathNode : public CVector
{
public:
	int type;
	CPathNode() {}
	CPathNode(const CVector& vt) : TVector<double>(vt) {}
//	CPathNode(const CMatrix& vt) { ASSERT(vt.w * vt.h == 3); x = vt[0]; y = vt[1]; z = vt[2]; }
	CPathNode& operator=(const CVector& vt) { x = vt.x; y = vt.y; z = vt.z; return *this; }
//	CPathNode& operator=(const CVector2& pt) { x = pt.x; y = pt.y; z = 0; return *this; }

	bool IsPoint() { return (type & PNTF_NODE) != 0; }
	bool IsEndPoint() { return (type & PNT_ENDNODE) == PNT_ENDNODE; }
	bool IsControlPoint() { return (type & PNT_CONTROLNODE) == PNT_CONTROLNODE; }
	bool IsHiddenPoint() { return (type & PNTF_NODEHIDDEN) != 0; }
	bool IsAbsPoint() { return type & PNTF_NODE && !(type & PNTF_RELATIVE); }		// if absolute point
	bool IsRelPoint() { return type & PNTF_NODE && type & PNTF_RELATIVE; }		// if relative point
	bool IsCommand() { return (type & PNTF_COMMAND) != 0; }

	bool HidePoint()
	{
		if (!(type & PNTF_NODE))
			return false;
		type &= ~PNTF_NODE;
		type |= PNTF_NODEHIDDEN;
		return true;
	}
};


struct SCurveInfo
{
	NODEREF nrStart, nrEnd;
	int numSegs;
	enum Dir nStartDir, nEndDir;
	CVector vtStartDir, vtEndDir;
	CArray<int, int> FitBreakArray;
	void* pNodeSumsArray;
	void operator=(SCurveInfo& ci) { nrStart = ci.nrStart; nrEnd = ci.nrEnd;}
};

typedef CArray<CPathNode, CPathNode&> CPathNodeArray;
typedef CArray<SCurveInfo, SCurveInfo&> CCurveInfoArray;


/////////////////////////////////////////////////////////////////////////////
// CPathDoc document

class CPathDoc : public CDocument
{
public:
	CPathDoc();           // protected constructor used by dynamic creation
protected:
//	DECLARE_DYNCREATE(CPathDoc)		// may be needed for FileNew
	DECLARE_SERIAL(CPathDoc)


// Attributes
protected:
//	CString m_strPathFileName;

	int m_iPathNumInFile;

//	int m_iFilePathNumber;

	// Tool Attributes
	CString m_strToolName;
	CString m_strToolCode;
	double m_diaTool;

	bool m_bGotJoiningTabs;
	SJoiningTabData m_JoinTab;

	CPathNodeArray m_NodeArray;		// origional nodes from file
	CPathNode m_MinNodeValues, m_MaxNodeValues;
	CPathNode m_ModNode;			// used to temporaily convert relative node to absoulute

	DWORD m_iNumSegments;
//	DWORD m_iFirstListSegment;
//	DWORD m_iLastListSegment;
	
//	CArray<CPathRefMark, CPathRefMark&> m_DrivePropArray;
//	int m_iNumDriveProp;

	// path speed
	int m_iCountSpeedView;
	CPathSpeedDoc* m_pSpeedDoc;

public:
	CCurveInfoArray m_CurveInfoArray;
	CPathNodeArray m_FittedBezierArray;		// result of bezier curve fitting to segment curves

protected:
	int m_nrLastAddedToFittedArray;


// Operations
public:
	void ErasePath() { SetModifiedFlag(false); OnNewDocument(); }
	void FinaliseNewPath();

	void FindSpeeds();
	CPathSpeedDoc* GetSpeedDoc() { return m_pSpeedDoc; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPathDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	virtual void OnChangedViewList();
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL


// Implementation
public:
	virtual ~CPathDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void FindNodeBounds();
//	void AddDrivePropChange(CPathRefMark& driveProp);
	void JoiningTabs();


public:
	void SetToolName(char* strName);
	void SetToolCode(char* strCode);
	void SetToolDiameter(double diaTool);
	const CString& GetToolName() { return m_strToolName; }
	const CString& GetToolCode() { return m_strToolCode; }
	double GetToolDiameter() { return m_diaTool; }

	void AddPathNode(CPathNode& pathNode) { m_NodeArray.Add(pathNode); }
	void InsertNodeAt(NODEREF nr, CPathNode& pathNode) { m_NodeArray.InsertAt(nr, pathNode); }
	void InsertNodeAfter(NODEREF nr, CPathNode& pathNode) { m_NodeArray.InsertAt(nr+1, pathNode); }
	void RemoveLastNode() { m_NodeArray.RemoveAt(m_NodeArray.GetSize()-1); }
	void RemoveNode(NODEREF nr) { m_NodeArray.RemoveAt(nr); }

	void SetStartOfPass(NODEREF nr);
	void SetEndOfPass();


	DWORD GetNumNodes() { return (DWORD)m_NodeArray.GetSize(); }
	DWORD GetNumSegments() { return m_iNumSegments; }

	CPathNode& GetMinNodeValues() { return m_MinNodeValues; }
	CPathNode& GetMaxNodeValues() { return m_MaxNodeValues; }

	// functions returning command or point nodes
	CPathNode* GetFirstPathNode(NODEREF& nr);
	CPathNode* GetLastPathNode(NODEREF& nr);
	CPathNode* GetPathNode(NODEREF nr);
	CPathNode* GetNextPathNode(NODEREF& nr);
	CPathNode* GetPrevPathNode(NODEREF& nr);

	// functions returning point nodes
	CPathNode* GetFirstNode(NODEREF& nr);
	CPathNode* GetLastNode(NODEREF& nr);
	CPathNode* GetNode(NODEREF nr);
	CPathNode* GetNodeAbs(NODEREF nr);
	CPathNode* GetNextNode(NODEREF& nr);
	CPathNode* GetPrevNode(NODEREF& nr);
	CPathNode* GetNextEndNode(NODEREF& nr);
	CPathNode* GetPrevEndNode(NODEREF& nr);
	CPathNode* GetOffsetPointNodes(NODEREF& nr, int iOffset);	// skip iOffset Point(non command) nodes

	bool HideEndNode(NODEREF nr);

//	void ResetCommandNodeCount() { m_countCommandNodes = 0; }
//	int GetCommandNodeCount() { return m_countCommandNodes; }

	bool GetStartPos(CVector& vtPos);
	bool GetEndPos(CVector& vtPos);
	bool GetInitSegPolys(SSegPolys& polys);
	bool GetFinalSegPolys(SSegPolys& polys);
	bool GetSegPolysAt(SSegPolys& polys, int seg);
	bool GetNextSegPolys(SSegPolys& polys);
	bool GetPrevSegPolys(SSegPolys& polys);
//	bool MoveToSeg(SSegPolys& polys, SEGREF seg);				// tracks path property changes
	bool MoveToLocForFoward(SSegPolys& polys, CPathLoc& loc);	// tracks path property changes
	bool MoveToLocForBackward(SSegPolys& polys, CPathLoc& loc);		// tracks path property changes
	bool MoveBackDist(NODEREF& nr, double& s, double& dist, double distNear = 0);
	int ConsecutiveSegNodes(NODEREF nr);


	bool FindLocFirstSegPathPropChange(SSegPolys& polys);
	bool FindLocLastSegPathPropChange(SSegPolys& polys);
	bool FindLocNextSegPathPropChange(SSegPolys& polys);
	bool FindLocPrevSegPathPropChange(SSegPolys& polys);

//	void SetDrivePropStart(SDriveProperties& dp) { dp.ref = -1; }	// not finished!!
//	void SetDrivePropEnd(SDriveProperties& dp) { dp.ref = 0; }		// not finished!!

protected:
	void GetSegFromNodes(SSegPolys& polys, CPathNode *arpNodes[], int numNodes);

	void SetNextPathProp(SSegPolys& polys);
	void SetPrevPathProp(SSegPolys& polys);




	// Generated message map functions
protected:
	//{{AFX_MSG(CPathDoc)
	afx_msg void OnPathFindSpeeds();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnPathSendStart();
	afx_msg void OnUpdatePathSendStart(CCmdUI* pCmdUI);
	afx_msg void OnPathDoBoatshed1() { OnPathDoBoatshed(1); }
	afx_msg void OnPathDoBoatshed2() { OnPathDoBoatshed(2); }
	afx_msg void OnPathDoBoatshed3() { OnPathDoBoatshed(3); }
	afx_msg void OnPathDoRectangle();
	afx_msg void OnPathDoCircle();
	afx_msg void OnPathDoHeart();
	afx_msg void OnPathDoMinaret();
	afx_msg void OnPathHoleMatrix();
	afx_msg void OnPathHoleMatrixServoCalibration();
	afx_msg void OnBaseScrewsArray();
	afx_msg void OnBaseVacuumHoseSingle();
	afx_msg void OnBaseVacuumHoseArray();
	afx_msg void OnManifoldVacuumBoxArray();
	afx_msg void OnManifoldVacuumBoxSingle();
	afx_msg void OnVacuumBoardTrenching();
	afx_msg void OnVacuumBoardStarArray();
	afx_msg void OnDepthCheck();
	//}}AFX_MSG
	afx_msg void OnPathDoBoatshed(int iSize);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATHDOC_H__B0E03CA1_FD9F_11D4_8C1E_C239773B3A2C__INCLUDED_)
