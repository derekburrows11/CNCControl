// PathLoader.cpp: implementation of the CPathLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"

#include "PathDoc.h"
#include "PathLoader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


enum
{
	ERROR_StartPathWhileInPath = 1,
	ERROR_PathNumGTNumPaths,
	ERROR_EndPathWhileNotInPath,
	ERROR_EndPathMisMatch,
	ERROR_NodeBracketMismatch,
	ERROR_PathSyntax,
	ERROR_ExtraCharsIgnored,
	ERROR_TwoCoordWithoutConstAxis,
	ERROR_BadNodeCoords,
	ERROR_BadNodeSequence,
	ERROR_BadSegmentNum,
	ERROR_EndNodesMustBeAbsolute,
	ERROR_TooManyControlNodes,

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPathLoader::CPathLoader()
{
	m_pPathDoc = NULL;
}

CPathLoader::~CPathLoader()
{
}


void CPathLoader::Load(CArchive& ar)
{
	ASSERT(ar.m_pDocument->IsKindOf(RUNTIME_CLASS(CPathDoc)));
	m_pPathDoc = (CPathDoc*)ar.m_pDocument;

	m_numConseqControl = 0;
	m_iTotalErrors = 0;
	m_iNumberOfPaths = -1;
	m_iPathNumber = -1;
	m_iHighestPathNumber = 0;
	m_iSegmentNumber = 0;
	m_iPrevNodeType = PNT_ENDNODE;		// to set up start
	m_iFileLineNumber = 0;
	m_iConstAxis = -1;

	CFile* pFile = ar.GetFile();
	ASSERT(pFile->GetPosition() == 0);
	DWORD nFileSize = pFile->GetLength();


	CString strIn;
	while (ar.ReadString(strIn))
	{
		DWORD nFilePos = pFile->GetPosition();
		m_iFileLineNumber++;
		Line.SetToString(strIn.GetBuffer(0));
		Line.EatWS();
		if (isdigit((char)Line))	// starts with a digit
		{
			if (m_iPathNumber != -1)		// if in path description
			{
				int iSegNum;
				Line.GetInt(iSegNum);
				m_iLineType = 0;
				if (Line.ExtractString("ci"))
					m_iLineType = PNT_CONTROLNODE;
				else if (Line.ExtractString("cf"))
					m_iLineType = PNT_CONTROLNODE;
				else if (Line.ExtractString("nf"))
					m_iLineType = PNT_ENDNODE;
				else if (Line.ExtractString("ni"))		// occurs least
					m_iLineType = PNT_ENDNODE;
				else if (Line.ExtractString("sdp"))		// Set Drive Properties
					m_iLineType = PNT_SETDRIVEPROP;
				else		// not a segment instruction
					LogError(ERROR_PathSyntax);
				m_iLineSegmentNum = iSegNum;
				if (m_iLineType & PNTF_NODE)
				{
					if (ReadPoint())
					{
						Line.EatWS();
						if (!Line.AtEOL())
							LogError(ERROR_ExtraCharsIgnored);
						StoreNode();
					}
				}
			}
			else		// not in path description
				;
		}
		else		// line doesn't start with digit
		{
			if (Line.ExtractIString("start path"))
				TokenStartPath();
			else if (Line.ExtractIString("end path"))
				TokenEndPath();
			else if (Line.ExtractIString("number of segments"))
				TokenNumberOfSegments();
			else if (Line.ExtractIString("path bounding box"))
				TokenPathBoundingBox();
			else if (Line.ExtractIString("number of paths"))
				TokenNumberOfPaths();
			else if ((char)Line == '-')		// comment
				;			// ignore

		}			

	}	// while not EOF

}

void CPathLoader::LogError(int error)
{
	m_iTotalErrors++;
	m_iLastError = error;
	m_iLastErrorLine = m_iFileLineNumber;
	TRACE1("CPathLoader error: %i\n", error);
}

bool CPathLoader::ReadPoint()
{
	Line.EatWS();
	if (Line.Extract('('))
		m_iLineType |= PNTF_ABSOLUTE;
	else if (Line.Extract('['))
		m_iLineType |= PNTF_RELATIVE;
	else
		return false;
	double* v = m_adPoint;
	int numChars = -1;
	int res = sscanf(Line, "%lf%*[, ]%lf%n%*[, ]%lf%n", &v[0], &v[1], &numChars, &v[2], &numChars);
	if (res == 2)
	{
		switch (m_iConstAxis)
		{
		case -1: LogError(ERROR_TwoCoordWithoutConstAxis); break;
		case  0: v[2] = v[1]; v[1] = v[0]; break;
		case  1:	v[2] = v[1]; break;
		case  2: break;
		}
		if (m_iConstAxis != -1)
			v[m_iConstAxis] = m_fConstAxisValue;
	}
	else if (res == 3) ;
	else
	{
		LogError(ERROR_BadNodeCoords);
		return false;
	}
	Line += numChars;
	Line.EatWS();
	if (Line.Extract(')'))
	{
		if (!(m_iLineType & PNTF_ABSOLUTE))
			LogError(ERROR_NodeBracketMismatch);
	}
	else if (Line.Extract(']'))
	{
		if (!(m_iLineType & PNTF_RELATIVE))
			LogError(ERROR_NodeBracketMismatch);
	}
	return true;
}

void CPathLoader::StoreNode()
{
/*
	int nPrev = m_iPrevNodeType & NODE_LOCATION;
	int ExpSeg = -1;
	switch (m_iLineType & NODE_LOCATION)
	{
	case NODE_NI:
		if (nPrev != NODE_CI)
			if (nPrev == NODE_NF) ExpSeg = 2;
			else ExpSeg = 1;
		break;
	case NODE_CI:
		if (nPrev == NODE_NI) ExpSeg = 0;
		else if (nPrev == NODE_NF) ExpSeg = 1;
		break;
	case NODE_CF:
		if (nPrev == NODE_CI) ExpSeg = 0;
		break;
	case NODE_NF:
		if (nPrev != NODE_CI)
			if (nPrev == NODE_NF) ExpSeg = 1;
			else ExpSeg = 0;
		break;
	}
	if (ExpSeg == -1)
		LogError(ERROR_BadNodeSequence);
	if (m_iLineSegmentNum != m_iSegmentNumber + ExpSeg)
		LogError(ERROR_BadSegmentNum);
*/
	if (m_iLineType & PNTF_END)
		m_numConseqControl = 0;
	else if (m_iLineType & PNTF_CONTROL)
		if (++m_numConseqControl > 2)
			LogError(ERROR_TooManyControlNodes);
	if ((m_iLineType & PNTF_END) && !(m_iLineType & PNTF_ABSOLUTE))
		LogError(ERROR_EndNodesMustBeAbsolute);
	m_iSegmentNumber = m_iLineSegmentNum;
	m_iPrevNodeType = m_iLineType;

	CPathNode newPathNode;
	newPathNode.type = m_iLineType;
	newPathNode.x = m_adPoint[0];
	newPathNode.y = m_adPoint[1];
	newPathNode.z = m_adPoint[2];
	m_pPathDoc->AddPathNode(newPathNode);


}

void CPathLoader::TokenStartPath()
{
	if (m_iPathNumber != -1)
	{
		LogError(ERROR_StartPathWhileInPath);
		return;
	}
	Line.EatWS();
	int iNum;
	if (Line.GetInt(iNum))
		m_iPathNumber = iNum;
	else		// there was no number!
		m_iPathNumber = m_iHighestPathNumber + 1;
	if (m_iPathNumber > m_iNumberOfPaths && m_iNumberOfPaths != -1)
		LogError(ERROR_PathNumGTNumPaths);
	m_iHighestPathNumber = m_iPathNumber;
}

void CPathLoader::TokenEndPath()
{
	if (m_iPathNumber == -1)
	{
		LogError(ERROR_EndPathWhileNotInPath);
		return;
	}
	Line.EatWS();
	int iNum;
	if (Line.GetInt(iNum))
	{
		if (iNum != m_iPathNumber)
			LogError(ERROR_EndPathMisMatch);
	}
	m_iPathNumber = -1;
}

void CPathLoader::TokenNumberOfSegments()
{
}

void CPathLoader::TokenPathBoundingBox()
{
}

void CPathLoader::TokenNumberOfPaths()
{
}


