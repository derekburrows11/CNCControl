// PathLoader.h: interface for the CPathLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHLOADER_H__09EA53A3_0262_11D5_8C1E_F2004A730B2C__INCLUDED_)
#define AFX_PATHLOADER_H__09EA53A3_0262_11D5_8C1E_F2004A730B2C__INCLUDED_

#include "StrUtils.h"

//#include "PathDoc.h"
class CPathDoc;

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CPathLoader  
{
public:
	CPathLoader();
	virtual ~CPathLoader();
	void Load(CArchive& ar);

protected:
	CPathDoc* m_pPathDoc;
	int m_numConseqControl;
	int m_iNumberOfPaths;
	int m_iPathNumber;
	int m_iHighestPathNumber;
	int m_iLineSegmentNum;
	int m_iSegmentNumber;
	int m_iLineType;
	int m_iPrevNodeType;
	int m_iFileLineNumber;
	double m_adPoint[3];
	int m_iConstAxis;
	double m_fConstAxisValue;
	CInterpStr Line;

	int m_iTotalErrors;
	int m_iLastError;
	int m_iLastErrorLine;

protected:
	void LogError(int error);
	bool ReadPoint();
	void StoreNode();
	void TokenStartPath();
	void TokenEndPath();
	void TokenNumberOfSegments();
	void TokenPathBoundingBox();
	void TokenNumberOfPaths();


};

#endif // !defined(AFX_PATHLOADER_H__09EA53A3_0262_11D5_8C1E_F2004A730B2C__INCLUDED_)
