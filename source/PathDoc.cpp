// PathDoc.cpp : implementation file
//

#include "stdafx.h"
#include "CNCControl.h"
#include "CNCControlApp.h"		// only for template pointers
#include "PathLoader.h"
#include "PathSpeedDoc.h"
#include "PathBuilder.h"
#include "PolyFunc.h"

#include "PathDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif






/////////////////////////////////////////////////////////////////////////////
// CPathDoc

//IMPLEMENT_DYNCREATE(CPathDoc, CDocument)
IMPLEMENT_SERIAL(CPathDoc, CDocument, 1)


BEGIN_MESSAGE_MAP(CPathDoc, CDocument)
	//{{AFX_MSG_MAP(CPathDoc)
	ON_COMMAND(ID_PATH_FINDSPEEDS, OnPathFindSpeeds)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_PATH_SENDSTART, OnPathSendStart)
	ON_UPDATE_COMMAND_UI(ID_PATH_SENDSTART, OnUpdatePathSendStart)
	ON_COMMAND(ID_PATH_HOLEMATRIX, OnPathHoleMatrix)
	ON_COMMAND(ID_PATH_DOSTARHOLEMATRIX, OnVacuumBoardStarArray)
	ON_COMMAND(ID_PATH_VACUUMBOARDTRENCHING, OnVacuumBoardTrenching)
	ON_COMMAND(ID_PATH_HOLEMATRIXSERVOCAL, OnPathHoleMatrixServoCalibration)
	ON_COMMAND(ID_PATH_HOLEMATRIXBASESCREWS, OnBaseScrewsArray)
	ON_COMMAND(ID_PATH_BASEVACUUMHOSESINGLE, OnBaseVacuumHoseSingle)
	ON_COMMAND(ID_PATH_BASEVACUUMHOSEARRAY, OnBaseVacuumHoseArray)
	ON_COMMAND(ID_PATH_MANIFOLDVACUUMBOXARRAY, OnManifoldVacuumBoxArray)
	ON_COMMAND(ID_PATH_MANIFOLDVACUUMBOXSINGLE, OnManifoldVacuumBoxSingle)
	ON_COMMAND(ID_PATH_DEPTHCHECK, OnDepthCheck)
	ON_COMMAND(ID_PATH_DOBOATSHED1, OnPathDoBoatshed1)
	ON_COMMAND(ID_PATH_DOBOATSHED2, OnPathDoBoatshed2)
	ON_COMMAND(ID_PATH_DOBOATSHED3, OnPathDoBoatshed3)
	ON_COMMAND(ID_PATH_DORECTANGLE, OnPathDoRectangle)
	ON_COMMAND(ID_PATH_DOCIRCLE, OnPathDoCircle)
	ON_COMMAND(ID_PATH_DOHEART, OnPathDoHeart)
	ON_COMMAND(ID_PATH_DOMINARET, OnPathDoMinaret)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CPathDoc::CPathDoc()
{
	m_pSpeedDoc = NULL;
}

CPathDoc::~CPathDoc()
{
}


/////////////////////////////////////////////////////////////////////////////
// CPathDoc message handlers

BOOL CPathDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}


BOOL CPathDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
//	CString strTitle = GetTitle();		// title set to file name when this function returns!
//	SetModifiedFlag(false);			// is by default - to disable ID_FILE_SAVE
	return TRUE;
}

void CPathDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsModified());
}

void CPathDoc::DeleteContents() 
{
	m_NodeArray.SetSize(0, 2048);		// can't set with an initial allocation!
//	m_NodeArray.SetSize(1);		// doesn't deallocate memory
//	GetNumNodes() = 0;
	m_iNumSegments = 0;
	m_MinNodeValues = 0;
	m_MaxNodeValues = 0;

	m_bGotJoiningTabs = false;
	m_JoinTab = g_Settings.PathOptions.m_JoinTab;

	m_diaTool = 0;

	m_iCountSpeedView = 0;
//	m_DrivePropArray.SetSize(256, 256);
//	m_iNumDriveProp = 0;

	m_pSpeedDoc = NULL;
	
	CDocument::DeleteContents();	// does nothing
}

/////////////////////////////////////////////////////////////////////////////
// CPathDoc diagnostics

#ifdef _DEBUG
void CPathDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPathDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPathDoc serialization


void CPathDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		CPathLoader newPath;
		newPath.Load(ar);
		FindNodeBounds();
	}
}



/////////////////////////////////////////////////////////////////////////////
// CPathDoc commands


void CPathDoc::SetToolName(char* strName)
{
	m_strToolName = strName;
}

void CPathDoc::SetToolCode(char* strCode)
{
	m_strToolCode = strCode;
}

void CPathDoc::SetToolDiameter(double diaTool)
{
	m_diaTool = diaTool;
}





/*
void CPathDoc::AddDrivePropChange(CPathRefMark& driveProp)
{
	m_DrivePropArray.SetAtGrow(m_iNumDriveProp, driveProp);	// stores 'Set Drive Properties'
	m_iNumDriveProp++;
}
*/
void CPathDoc::FindNodeBounds()
{
	// also find number of path segments
	// and check for coincident nodes - either end/end or end/control
	// notify and remove following coincident end/end nodes
	// notify coincident end/control nodes
	NODEREF nr;
	CPathNode* pNode = GetFirstNode(nr);
	CPathNode* pPrevNode;
	m_iNumSegments = 0;
	if (!pNode)
	{
		m_MinNodeValues = CVector(0,0,0);
		m_MaxNodeValues = CVector(0,0,0);
		return;
	}
	pNode = GetNodeAbs(nr);		// first node will be and endnode!
	m_MinNodeValues = *pNode;
	m_MaxNodeValues = *pNode;
	while (GetNextNode(nr))
	{
		pPrevNode = pNode;
		pNode = GetNodeAbs(nr);
		if (!pNode->IsPoint())
			continue;
		m_MinNodeValues.Min(*pNode);
		m_MaxNodeValues.Max(*pNode);
		if (pNode->IsEndPoint())		// num segs is num endnodes - 1
			m_iNumSegments++;

		if (((*pNode - *pPrevNode).SumAbs() <= 2e-6) && (pNode->IsEndPoint() || pPrevNode->IsEndPoint()))
			if (pNode->IsEndPoint() && pPrevNode->IsEndPoint())
			{
				LOGMESSAGE1("Coincident (<=2e-6) End Node %i Removed", nr);
				RemoveNode(nr);
				nr--;			// move back one
				pNode = pPrevNode;
				m_iNumSegments--;			// was an endpoint!
			}
			else
				LOGERROR1("Coincident End/Control Node %i", nr);
	}
}



///////////////////////////////////////
// node functions
///////////////////////////////////////

// CPathNode's can be command's, not point nodes
// Get????PathNode(nr)	gets next node in list, command or point
// Get????Node(nr)		gets next point node in list

CPathNode* CPathDoc::GetFirstPathNode(NODEREF& nr)
{
	nr = 0;
	if (m_NodeArray.GetSize() == 0)
		return NULL;
	return &m_NodeArray.ElementAt(nr);
}

CPathNode* CPathDoc::GetLastPathNode(NODEREF& nr)
{
	nr = m_NodeArray.GetSize() - 1;
	if (nr < 0)
		return NULL;
	return &m_NodeArray.ElementAt(nr);
}

CPathNode* CPathDoc::GetPathNode(NODEREF nr)
{
	if (nr >= m_NodeArray.GetSize() || nr < 0)
		return NULL;
	return &m_NodeArray.ElementAt(nr);
}

CPathNode* CPathDoc::GetNextPathNode(NODEREF& nr)
{
	if (nr + 1 >= m_NodeArray.GetSize())
		return NULL;
	return &m_NodeArray.ElementAt(++nr);
}

CPathNode* CPathDoc::GetPrevPathNode(NODEREF& nr)
{
	if (nr <= 0)
		return NULL;
	return &m_NodeArray.ElementAt(--nr);
}


// Functions returning 'point' nodes

CPathNode* CPathDoc::GetFirstNode(NODEREF& nr)
{
	nr = 0;
	if (m_NodeArray.GetSize() == 0)
		return NULL;
	CPathNode* pNode = &m_NodeArray.ElementAt(nr);
	if (pNode->type & PNTF_NODE)
		return pNode;
	return GetNextNode(nr);
}

CPathNode* CPathDoc::GetLastNode(NODEREF& nr)
{
	nr = m_NodeArray.GetSize() - 1;
	if (nr < 0)
		return NULL;
	CPathNode* pNode = &m_NodeArray.ElementAt(nr);
	if (pNode->type & PNTF_NODE)
		return pNode;
	return GetPrevNode(nr);
}

CPathNode* CPathDoc::GetNode(NODEREF nr)
{
	if (nr >= m_NodeArray.GetSize())
		return NULL;
	CPathNode* pNode = &m_NodeArray.ElementAt(nr);
	ASSERT(pNode->type & PNTF_NODE);		// check it's a 'point' node
	return pNode;
}

CPathNode* CPathDoc::GetNextNode(NODEREF& nr)
{
	NODEREF nrOrig = nr;
	int sizeArray = m_NodeArray.GetSize();
	for (;;)
	{
		if (++nr >= sizeArray)
		{
			nr = nrOrig;		// don't change nr!
			return NULL;
		}
		CPathNode* pNode = &m_NodeArray.ElementAt(nr);
		if (pNode->type & PNTF_NODE)
			return pNode;
	}
}

CPathNode* CPathDoc::GetNextEndNode(NODEREF& nr)
{
	NODEREF nrOrig = nr;
	int sizeArray = m_NodeArray.GetSize();
	for (;;)
	{
		if (++nr >= sizeArray)
		{
			nr = nrOrig;		// don't change nr!
			return NULL;
		}
		CPathNode* pNode = &m_NodeArray.ElementAt(nr);
		if (pNode->IsEndPoint())
			return pNode;
	}
}

int CPathDoc::ConsecutiveSegNodes(NODEREF nr)
{
	ASSERT(m_NodeArray[nr].IsEndPoint());
	int degree = 1;
	int sizeArray = m_NodeArray.GetSize();
	for (;;)
	{
		if (++nr >= sizeArray)
			return 0;
		CPathNode* pNode = &m_NodeArray.ElementAt(nr);
		if (pNode->IsEndPoint())
			return degree;
		if (!pNode->IsPoint())
			return 0;
		ASSERT(++degree <= 3);
	}
}

CPathNode* CPathDoc::GetPrevNode(NODEREF& nr)
{
	NODEREF nrOrig = nr;
	for (;;)
	{
		if (--nr < 0)
		{
			nr = nrOrig;		// don't change nr!
			return NULL;
		}
		CPathNode* pNode = &m_NodeArray.ElementAt(nr);
		if (pNode->type & PNTF_NODE)
			return pNode;
	}
}

CPathNode* CPathDoc::GetPrevEndNode(NODEREF& nr)
{
	NODEREF nrOrig = nr;
	for (;;)
	{
		if (--nr < 0)
		{
			nr = nrOrig;		// don't change nr!
			return NULL;
		}
		CPathNode* pNode = &m_NodeArray.ElementAt(nr);
		if (pNode->IsEndPoint())
			return pNode;
	}
}

CPathNode* CPathDoc::GetOffsetPointNodes(NODEREF& nr, int iOffset)	// skip iOffset Point(non command) nodes
{
	NODEREF nrOrig = nr;
	int iDir = (iOffset > 0) ? 1 : -1;
	if (iOffset == 0)
		iDir = 0;
	NODEREF nrMax = m_NodeArray.GetSize() - 1;
	for (;;)
	{
		nr += iDir;
		if (nr < 0 || nr > nrMax)
		{
			nr = nrOrig;		// don't change nr!
			return NULL;
		}
		CPathNode* pNode = &m_NodeArray.ElementAt(nr);
		if (pNode->type & PNTF_NODE)
			if ((iOffset -= iDir) == 0)
				return pNode;
	}
}

CPathNode* CPathDoc::GetNodeAbs(NODEREF nr)
{
	if (nr >= m_NodeArray.GetSize())
		return NULL;
	CPathNode* pNode = &m_NodeArray.ElementAt(nr);
	int nType = pNode->type;
	ASSERT(nType & PNTF_NODE);
	if (!(nType & PNTF_RELATIVE))
		return pNode;
	ASSERT(nType & PNTF_CONTROL);

	NODEREF nrOrig = nr;
	CPathNode* pNodeAbs = GetPrevNode(nr);	// check if node before is absolute
	if (pNodeAbs->type & PNTF_RELATIVE)
		pNodeAbs = GetNextNode(nrOrig);				// or else node after should be!
	ASSERT(!(pNodeAbs->type & PNTF_RELATIVE));

	m_ModNode = *pNodeAbs + *pNode;	// Add relative on to absolute
	m_ModNode.type = nType;
	m_ModNode.type &= ~PNTF_RELATIVE;
	m_ModNode.type |= PNTF_ABSOLUTE;
	return &m_ModNode;
}

void CPathDoc::SetStartOfPass(NODEREF nr)
{
	ASSERT(nr >= 0);
	CPathNode* pNode = &m_NodeArray.ElementAt(nr);
	ASSERT(pNode->type & PNTF_NODE);
	pNode->type |= PNTF_STARTPASS;
}

void CPathDoc::SetEndOfPass()			// sets end of pass at last node
{
	NODEREF nr;
	nr = m_NodeArray.GetSize() - 1;
	ASSERT(nr >= 0);
	CPathNode* pNode = &m_NodeArray.ElementAt(nr);
	ASSERT(pNode->type & PNTF_NODE);
	pNode->type |= PNTF_ENDPASS;
}


bool CPathDoc::HideEndNode(NODEREF nr)
{
	CPathNode& node = m_NodeArray.ElementAt(nr);
	ASSERT(node.IsEndPoint());
	return node.HidePoint();
}


///////////////////////////////////////
// segment functions
///////////////////////////////////////

bool CPathDoc::GetStartPos(CVector& vtPos)
{
	NODEREF nr;
	CPathNode* pNode = GetFirstNode(nr);
	if (!pNode)
		return false;
	ASSERT(pNode->type & PNTF_END);
	vtPos = *pNode;
	return true;
}

bool CPathDoc::GetEndPos(CVector& vtPos)
{
	NODEREF nr;
	CPathNode* pNode = GetLastNode(nr);
	if (!pNode)
		return false;
	ASSERT(pNode->type & PNTF_END);
	vtPos = *pNode;
	return true;
}

bool CPathDoc::MoveBackDist(NODEREF& nr, double& s, double& dist, double distNear)
{
	// stops a node if dist is within distNear, s & dist negative if before node
	// move dist, just use straight dist between end nodes for now!!
	CPathNode* pNode0 = GetNode(nr);
	ASSERT(pNode0->type & PNTF_END);
	ASSERT(s >= 0 && s <= 1);
	CPathNode* pNode1;
	double dist1 = dist;
	if (s > 0)
	{
		NODEREF nr0 = nr;
		pNode1 = GetNextEndNode(nr);
		nr = nr0;
		if (pNode1 == NULL)
			return false;
		double distSeg = (*pNode1 - *pNode0).Mag();	// pNode0/1 reversed
		dist1 -= s * distSeg;
		if (dist1 <= distNear)
		{
			dist = -dist1;
			s = dist / distSeg;
			return true;
		}
	}
	for (;;)
	{
		pNode1 = GetPrevEndNode(nr);
		if (pNode1 == NULL)
			return false;
		double distSeg = (*pNode1 - *pNode0).Mag();
		dist1 -= distSeg;
		if (dist1 <= distNear)
		{
			dist = -dist1;
			s = dist / distSeg;
			return true;
		}
		pNode0 = pNode1;
	}
}

bool CPathDoc::GetInitSegPolys(SSegPolys& polys)
{
	CPathNode* pNode = GetFirstNode(polys.nrFinal);
	if (!pNode)
		return false;
	ASSERT(pNode->type & PNTF_END);		// first end node
	polys.seg = -1;						// will be inc to 0
	return GetNextSegPolys(polys);
}

bool CPathDoc::GetFinalSegPolys(SSegPolys& polys)
{
	CPathNode* pNode = GetLastNode(polys.nrInit);
	if (!pNode)
		return false;
	ASSERT(pNode->type & PNTF_END);		// last end node
	polys.seg = GetNumSegments();			// will be dec be 1
	return GetPrevSegPolys(polys);
}

bool CPathDoc::GetSegPolysAt(SSegPolys& polys, int seg)
{
	SEGREF& segCurr = polys.seg;
	NODEREF& nrInit = polys.nrInit;
	NODEREF& nrFinal = polys.nrFinal;
	if (seg == segCurr)
	{
		nrFinal = nrInit;
		segCurr--;
		return GetNextSegPolys(polys);
	}
	CPathNode* pNode;
	// get distance from Init, Current & Final positions, then choose closest
	int iFromInit = seg;
	int iFromCurr = seg - segCurr;
	int iFromFinal = GetNumSegments() - 1 - seg;
	if (iFromInit <= iFromFinal)
	{
		if (iFromInit < abs(iFromCurr))
		{
			pNode = GetFirstNode(nrFinal);
			if (!pNode)
				return false;							// no node list!
			ASSERT(pNode->type & PNTF_END);		// first end node
			segCurr = -1;
		}
	}
	else
		if (iFromFinal < abs(iFromCurr))
		{
			pNode = GetLastNode(nrInit);
			if (!pNode)
				return false;							// no node list!
			ASSERT(pNode->type & PNTF_END);		// last end node
			segCurr = GetNumSegments();
		}

	if (segCurr < seg)				// scan foward through segments
	{
		while (segCurr < seg - 1)				// scan foward through segments
		{
			pNode = GetNextNode(nrFinal); 		// only returns PNTF_NODE types
			if (!pNode)
				return false;
			if (pNode->type & PNTF_END)			// get next end node
				segCurr++;
			else
				ASSERT(pNode->type & PNTF_CONTROL);		// must be a control node
		}
		GetNextSegPolys(polys);
	}
	else if (segCurr > seg)				// scan back through segments
	{
		while (segCurr > seg + 1)				// scan back through segments
		{
			pNode = GetPrevNode(nrInit); 		// only returns PNTF_NODE types
			if (!pNode)
				return false;
			if (pNode->type & PNTF_END)			// get previous end node
				segCurr--;
			else
				ASSERT(pNode->type & PNTF_CONTROL);		// must be a control node
		}
		GetPrevSegPolys(polys);
	}
	else
		ASSERT(0);			// shouldn't happen!
	return true;
}

void CPathDoc::SetNextPathProp(SSegPolys& polys)
{
	CPathNode* pNode = GetPathNode(polys.nrPathPropChange);
	int type = pNode->type;
	ASSERT(type & PNTF_COMMAND);
	if (type == PNT_SETFEEDRATE)
		polys.pathProps.feedRate = pNode->x;
	else if (type == PNT_SELECTTOOL)
		polys.pathProps.iTool = (int)pNode->x;
	else
		ASSERT(0);
}

void CPathDoc::SetPrevPathProp(SSegPolys& polys)
{
	CPathNode* pNode = GetPathNode(polys.nrPathPropChange);
	int type = pNode->type;
	ASSERT(type & PNTF_COMMAND);
	if (type == PNT_SETFEEDRATE)
		polys.pathProps.feedRate = pNode->y;
	else if (type == PNT_SELECTTOOL)
		polys.pathProps.iTool = (int)pNode->y;
	else
		ASSERT(0);
}

bool CPathDoc::MoveToLocForFoward(SSegPolys& polys, CPathLoc& loc)	// tracks path property changes
{
	SEGREF& segCurr = polys.seg;
	NODEREF& nrInit = polys.nrInit;
	NODEREF& nrFinal = polys.nrFinal;
	SEGREF& seg = loc.seg;
	CPathNode* pNode;
	if (segCurr < seg)				// scan foward through segments
	{
		for (;;)
		{
			pNode = GetNextPathNode(nrInit);
			if (!pNode)
				return false;
			int type = pNode->type;
			if (type & PNTF_NODE)
			{
				if (type & PNTF_END)			// get next end node
				{
					if (++segCurr >= seg)
						break;
				}
				else
					ASSERT(type & PNTF_CONTROL);		// must be a control node
			}
			else if (type & PNTF_COMMAND)
			{
				if (type == PNT_SETFEEDRATE)
					polys.pathProps.feedRate = pNode->x;
				else if (type == PNT_SELECTTOOL)
					polys.pathProps.iTool = (int)pNode->x;
			}
		}
		nrFinal = nrInit;				// start scan from start of seg to get all path prop changes
		segCurr--;						// to keep it on track
	}
	else	// if (segCurr >= seg)			// scan back through segments
	{
		for (;;)
		{
			pNode = GetPrevPathNode(nrFinal);
			if (!pNode)
				return false;
			int type = pNode->type;
			if (type & PNTF_NODE)
			{
				if (type & PNTF_END)			// get next end node
				{
					if (--segCurr < seg)
						break;
				}
				else
					ASSERT(type & PNTF_CONTROL);		// must be a control node
			}
			else if (type & PNTF_COMMAND)
			{
				if (type == PNT_SETFEEDRATE)
					polys.pathProps.feedRate = pNode->y;
				else if (type == PNT_SELECTTOOL)
					polys.pathProps.iTool = (int)pNode->y;
			}
		}
	}
	// polys is set to seg before required seg, nrFinal is the valid one
	ASSERT(segCurr == seg-1);
	GetNextSegPolys(polys);
	while (polys.locPathPropChange <= loc)		// advance past any changes to loc
	{
		SetNextPathProp(polys);
		FindLocNextSegPathPropChange(polys);
	}
	return true;
}

bool CPathDoc::MoveToLocForBackward(SSegPolys& polys, CPathLoc& loc)	// tracks path property changes
{
	SEGREF& segCurr = polys.seg;
	NODEREF& nrInit = polys.nrInit;
	NODEREF& nrFinal = polys.nrFinal;
	SEGREF& seg = loc.seg;
	CPathNode* pNode;
	if (segCurr > seg)				// scan back through segments
	{
		for (;;)
		{
			pNode = GetPrevPathNode(nrFinal);
			if (!pNode)
				return false;
			int type = pNode->type;
			if (type & PNTF_NODE)
			{
				if (type & PNTF_END)			// get next end node
				{
					if (--segCurr <= seg)
						break;
				}
				else
					ASSERT(type & PNTF_CONTROL);		// must be a control node
			}
			else if (type & PNTF_COMMAND)
			{
				if (type == PNT_SETFEEDRATE)
					polys.pathProps.feedRate = pNode->y;
				else if (type == PNT_SELECTTOOL)
					polys.pathProps.iTool = (int)pNode->y;
			}
		}
		nrInit = nrFinal;				// start scan from end of seg
		segCurr++;						// to keep it on track
	}
	else	// if (segCurr <= seg)				// scan foward through segments
	{
		for (;;)
		{
			pNode = GetNextPathNode(nrInit);
			if (!pNode)
				return false;
			int type = pNode->type;
			if (type & PNTF_NODE)
			{
				if (type & PNTF_END)			// get next end node
				{
					if (++segCurr > seg)
						break;
				}
				else
					ASSERT(type & PNTF_CONTROL);		// must be a control node
			}
			else if (type & PNTF_COMMAND)
			{
				if (type == PNT_SETFEEDRATE)
					polys.pathProps.feedRate = pNode->x;
				else if (type == PNT_SELECTTOOL)
					polys.pathProps.iTool = (int)pNode->x;
			}
		}
	}
	// polys is set to seg after required seg, nrInit is the valid one
	ASSERT(segCurr == seg+1);
	GetPrevSegPolys(polys);
	while (polys.locPathPropChange >= loc)
	{
		SetPrevPathProp(polys);
		FindLocPrevSegPathPropChange(polys);
	}
	return true;
}

bool CPathDoc::GetNextSegPolys(SSegPolys& polys)
{
	NODEREF nr = polys.nrFinal;
	int sizeNodeArray = m_NodeArray.GetSize();
	if (nr >= sizeNodeArray)
		return false;			// no node list
	CPathNode* pNode = &m_NodeArray.ElementAt(nr);
	ASSERT(pNode->type & PNTF_NODE && pNode->type & PNTF_END);	// first must be an ENDNODE

	int iCountDriveProp = 0;
	CPathNode *arpNode[4];
	int idxArr = 1;
	arpNode[0] = pNode;
	while (1)
	{
		if (++nr >= sizeNodeArray)
			return false;			// end of node list
		pNode = &m_NodeArray.ElementAt(nr);
		int type = pNode->type;
		if (type & PNTF_NODE)
		{
			arpNode[idxArr++] = pNode;
			if (type & PNTF_END)		// second end node
				break;
			ASSERT(type & PNTF_CONTROL);		// must be a control node
		}
		else if (type & PNTF_COMMAND)
		{
			if (type == PNT_SETFEEDRATE || type == PNT_SETDRIVEPROP)
				if (++iCountDriveProp == 1)		// first DriveProp change
				{
					polys.nrPathPropChange = nr;
					polys.locPathPropChange.s = pNode->z;	// z contains s location of change
				}
			else if (type == PNT_SELECTTOOL)
			{
				ASSERT(0);		// not handled yet
				return false;	// treat as a break path anyway for now
			}
			else if (type == PNT_BREAKPATH)
				return false;		// this path ends, another follows, should set some flag
		}
	}

	ASSERT(!(arpNode[0]->type & PNTF_RELATIVE));		// ENDNODEs must be absolute
	ASSERT(!(pNode->type & PNTF_RELATIVE));			// ENDNODEs must be absolute
	polys.nrInit = polys.nrFinal;				// after nodes found, undate references
	polys.nrFinal = nr;
	polys.seg++;
	polys.numPathPropChanges = iCountDriveProp;
	if (iCountDriveProp != 0)
		polys.locPathPropChange.seg = polys.seg;
	else
		polys.locPathPropChange.SetPostEnd();

	GetSegFromNodes(polys, arpNode, idxArr);
	return true;
}

bool CPathDoc::GetPrevSegPolys(SSegPolys& polys)
{
	NODEREF nr = polys.nrInit;
	int sizeNodeArray = m_NodeArray.GetSize();
	if (nr >= sizeNodeArray)
		return false;			// no node list
	CPathNode* pNode = &m_NodeArray.ElementAt(nr);
	ASSERT(pNode->type & PNTF_NODE && pNode->type & PNTF_END);	// first must be an ENDNODE

	int iCountDriveProp = 0;
	CPathNode *arpNode[4];
	int idxArr = 4;
	arpNode[--idxArr] = pNode;
	while (1)		// get previous end node
	{
		if (--nr < 0)
			return false;			// end of node list
		pNode = &m_NodeArray.ElementAt(nr);
		int type = pNode->type;
		if (type & PNTF_NODE)
		{
			arpNode[--idxArr] = pNode;
			if (type & PNTF_END)		// second end node
				break;
			ASSERT(type & PNTF_CONTROL);		// must be a control node
		}
		else if (type & PNTF_COMMAND)
		{
			if (type == PNT_SETFEEDRATE || type == PNT_SETDRIVEPROP)
				if (++iCountDriveProp == 1)		// first DriveProp change
				{
					polys.nrPathPropChange = nr;
					polys.locPathPropChange.s = pNode->z;	// z contains s location of change
				}
			else if (type == PNT_SELECTTOOL)
			{
				ASSERT(0);		// not handled yet
				return false;	// treat as a break path anyway for now
			}
			else if (type == PNT_BREAKPATH)
				return false;		// this path ends, another follows, should set some flag
		}
	}

	ASSERT(!(arpNode[3]->type & PNTF_RELATIVE));	// ENDNODEs must be absolute
	ASSERT(!(pNode->type & PNTF_RELATIVE));			// ENDNODEs must be absolute
	polys.nrFinal = polys.nrInit;				// after nodes found, undate references
	polys.nrInit = nr;
	polys.seg--;
	polys.numPathPropChanges = iCountDriveProp;
	if (iCountDriveProp != 0)
		polys.locPathPropChange.seg = polys.seg;
	else
		polys.locPathPropChange.SetPreStart();

	GetSegFromNodes(polys, arpNode + idxArr, 4-idxArr);
	return true;
}

void CPathDoc::GetSegFromNodes(SSegPolys& polys, CPathNode *arpNode[], int numNodes)
{	// determine segment polys from num nodes
	if (numNodes == 2)
	{
		polys.pos.SetSize(4,3);
		polys.pos = 0;
		polys.pos.SetRow(0, *arpNode[0]);						// row 0 is x^0 coeff's
		polys.pos.SetRow(1, *arpNode[1] - *arpNode[0]);	// row 1 is x^1 coeff's
	}
	else if (numNodes == 4)
	{
		CMatrix bezPos(4,3);			// (rows, cols) format
		bezPos.SetRow(0, *arpNode[0]);
		bezPos.SetRow(1, *arpNode[1]);
		bezPos.SetRow(2, *arpNode[2]);
		bezPos.SetRow(3, *arpNode[3]);
		if (!(arpNode[1]->type & PNTF_RELATIVE) &&
			 !(arpNode[2]->type & PNTF_RELATIVE))
			bezPos.Bezier2Poly(polys.pos);
		else
		{
			if (!(arpNode[1]->type & PNTF_RELATIVE))	// set both to relative
				bezPos.SetRow(1, *arpNode[1] - *arpNode[0]);
			if (!(arpNode[2]->type & PNTF_RELATIVE))
				bezPos.SetRow(2, *arpNode[2] - *arpNode[3]);
			bezPos.BezierRel2Poly(polys.pos);
		}
	}
	else			// numNodes not 2 or 4
		ASSERT(0);
													// Pos is  s^3
	polys.pos.Derivative(polys.vel);		// Vel is  s^2
	polys.vel.Derivative(polys.acc);		// Acc is  s^1
	polys.acc.Derivative(polys.jerk);	// Jerk is s^0
}

bool CPathDoc::FindLocFirstSegPathPropChange(SSegPolys& polys)
{
	polys.nrPathPropChange = polys.nrInit;		// start search from initial
	polys.numPathPropChanges = 2;					// make it check for more than 1
	return FindLocNextSegPathPropChange(polys);
}

bool CPathDoc::FindLocLastSegPathPropChange(SSegPolys& polys)
{
	polys.nrPathPropChange = polys.nrFinal;	// start search from final
	polys.numPathPropChanges = 2;					// make it check for more than 1
	return FindLocPrevSegPathPropChange(polys);
}


bool CPathDoc::FindLocNextSegPathPropChange(SSegPolys& polys)
{
	if (polys.numPathPropChanges > 1)		// don't need to check for more!
	{
		NODEREF nr = polys.nrPathPropChange;
		while (++nr < polys.nrFinal)
		{
			CPathNode* pNode = &m_NodeArray.ElementAt(nr);
			int type = pNode->type;
			if (type == PNT_SETFEEDRATE || type == PNT_SETDRIVEPROP)
			{
				polys.nrPathPropChange = nr;
				polys.locPathPropChange.s = pNode->z;	// z contains s location of change
				polys.locPathPropChange.seg = polys.seg;	// should already be set!
				return true;
			}
		}
	}
	polys.locPathPropChange.SetPostEnd();		// no more drive props found in segment
	return false;
}

bool CPathDoc::FindLocPrevSegPathPropChange(SSegPolys& polys)
{
	if (polys.numPathPropChanges > 1)		// don't need to check for more!
	{
		NODEREF nr = polys.nrPathPropChange;
		while (--nr > polys.nrInit)
		{
			CPathNode* pNode = &m_NodeArray.ElementAt(nr);
			int type = pNode->type;
			if (type == PNT_SETFEEDRATE || type == PNT_SETDRIVEPROP)
			{
				polys.nrPathPropChange = nr;
				polys.locPathPropChange.s = pNode->z;	// z contains s location of change
				polys.locPathPropChange.seg = polys.seg;	// should already be set!
				return true;
			}
		}
	}
	polys.locPathPropChange.SetPreStart();		// no more drive props found in segment
	return false;
}


void CPathDoc::OnPathFindSpeeds() 
{
	// calc speed stage by stage
	CWaitCursor waitCursor;
	FindSpeeds();


}

void CPathDoc::FindSpeeds()
{
	if (GetNumSegments() == 0)
	{
		LOGERROR("No segments in path!");
		return;
	}

	// find CDocTemplate for CPathSpeedDoc - it's in the CWinApp
	CDocTemplate* pDocTemplate = ((CCNCControlApp*)AfxGetApp())->m_pPathSpeedDocTemplate;
	ASSERT_VALID(pDocTemplate);
	CPathSpeedDoc* pPSDoc = static_cast<CPathSpeedDoc*>(pDocTemplate->CreateNewDocument());
	ASSERT(pPSDoc->IsKindOf(RUNTIME_CLASS(CPathSpeedDoc)));
	ASSERT_VALID(pPSDoc);
	pPSDoc->SetPathDoc(this);
	m_pSpeedDoc = pPSDoc;

//	pPSDoc->FindSpeeds();

	CFrameWnd* pFrame = pDocTemplate->CreateNewFrame(pPSDoc, NULL);
		// find new CPathSpeedView object to set view type
//	CWnd* pWnd = pFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST);
//	ASSERT(pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CPathSpeedView)));
//	((CPathSpeedView*)pWnd)->SetViewType(nViewType);			// 1 for axis parameters or 0 for general

	m_iCountSpeedView++;		//	keep count of number path speed doc derived from path doc.

	// Format title string
	char szTitle[MAX_PATH] = "Path Speeds - ";
	char* pTitleEnd = szTitle + strlen(szTitle);
	const CString& strPath = GetPathName();
	int iNameLoc = max(strPath.ReverseFind('\\'), strPath.ReverseFind('/'));
	strcpy(pTitleEnd, (const char*)strPath + ++iNameLoc);
	pTitleEnd += strlen(pTitleEnd);
	strcpy(pTitleEnd, "   ");
	pTitleEnd += strlen(pTitleEnd);
	itoa(m_iCountSpeedView, pTitleEnd, 10);
	pPSDoc->SetTitle(szTitle);
	pDocTemplate->InitialUpdateFrame(pFrame, NULL);


//	pPSDoc->FindSpeeds();
	pPSDoc->FindSpeedsStart();

	for (;;)
	{
		if (!pPSDoc->FindSpeedsMore())
			break;
		char strMsg[128];
		sprintf(strMsg, "Got to segment %i of %i.  Calc more limits (in PathDoc)?", pPSDoc->GetLastScanSegment(), GetNumSegments());
		if (AfxMessageBox(strMsg, MB_YESNO) != IDYES)
			break;
	}


}



void CPathDoc::OnPathSendStart() 
{
//	CCNCControlApp* pApp = dynamic_cast<CCNCControlApp*>(AfxGetApp());
	CCNCControlApp* pApp = static_cast<CCNCControlApp*>(AfxGetApp());
	if (pApp->SetControllerPath(this))
		pApp->PathSendStart();
}

void CPathDoc::OnUpdatePathSendStart(CCmdUI* pCmdUI) 
{
	CCNCControlApp* pApp = static_cast<CCNCControlApp*>(AfxGetApp());
	pCmdUI->Enable(!pApp->IsSendingPath());	
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


void CPathDoc::FinaliseNewPath() 
{
	FindNodeBounds();
//	SetModifiedFlag();
	UpdateAllViews(NULL);
}

void CPathDoc::OnDepthCheck() 
{
	CPathBuilder builder(this);
	builder.DepthCheck();
	SetTitle(builder.GetTitle());
	FinaliseNewPath();
}

void CPathDoc::OnPathHoleMatrix() 
{
	CPathBuilder builder(this);
	builder.HoleMatrixGeneral();
	SetTitle("HoleMatrixGeneral");

	FinaliseNewPath();
}

void CPathDoc::OnPathHoleMatrixServoCalibration() 
{
	CPathBuilder builder(this);
	builder.HoleMatrixServoCalibration();
	SetTitle("HoleMatrixServoCalibration");

	FinaliseNewPath();
}

void CPathDoc::OnBaseScrewsArray() 
{
	CPathBuilder builder(this);
	builder.BaseScrewsArray();
	SetTitle("BaseScrewsArray");

	FinaliseNewPath();
}

void CPathDoc::OnBaseVacuumHoseSingle() 
{
	CPathBuilder builder(this);
	builder.BaseVacuumHoseSingle();
	SetTitle("BaseVacuumHoseSingle");
	FinaliseNewPath();
}
void CPathDoc::OnBaseVacuumHoseArray()
{
	CPathBuilder builder(this);
	builder.BaseVacuumHoseArray();
	SetTitle("BaseVacuumHoseArray");
	FinaliseNewPath();
}

void CPathDoc::OnManifoldVacuumBoxArray() 
{
	CPathBuilder builder(this);
	builder.ManifoldVacuumBoxArray();
	SetTitle("ManifoldVacuumBoxArray");
	FinaliseNewPath();
}

void CPathDoc::OnManifoldVacuumBoxSingle() 
{
	CPathBuilder builder(this);
	builder.ManifoldVacuumBoxSingle();
	SetTitle("ManifoldVacuumBoxSingle");
	FinaliseNewPath();
}

void CPathDoc::OnVacuumBoardStarArray() 
{
	CPathBuilder builder(this);
	builder.VacuumBoardStarArray();
	SetTitle("VacuumBoardStarArray");

	FinaliseNewPath();
}

void CPathDoc::OnVacuumBoardTrenching() 
{
	CPathBuilder builder(this);
	builder.VacuumBoardTrenching();
	SetTitle("VacuumBoardTrenching");

	FinaliseNewPath();
}

void CPathDoc::OnPathDoBoatshed(int iSize) 
{
	CPathBuilder builder;
	builder.SetPathDoc(this);
	builder.Boatshed(iSize);
	SetTitle("Boatshed");

	FinaliseNewPath();
}

void CPathDoc::OnPathDoRectangle() 
{
	CPathBuilder builder;
	builder.SetPathDoc(this);
	builder.Rectangle();
	SetTitle(builder.GetTitle());

	FinaliseNewPath();
}

void CPathDoc::OnPathDoCircle() 
{
	double diameter = 805;
	CPathBuilder builder;
	builder.SetPathDoc(this);
	builder.Circle(diameter);
	SetTitle("Circle 805mm");
	
	FinaliseNewPath();
}

void CPathDoc::OnPathDoHeart() 
{
//	double size = 150;
	CPathBuilder builder;
	builder.SetPathDoc(this);
//	builder.Heart1(size);
	builder.Heart2(138, 200);
	SetTitle("Heart2");

	FinaliseNewPath();
}

void CPathDoc::OnPathDoMinaret() 
{
	CPathBuilder builder;
	builder.SetPathDoc(this);
	builder.Minaret();
	SetTitle("Minaret");

	FinaliseNewPath();
}


void CPathDoc::OnChangedViewList() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CDocument::OnChangedViewList();
}


void CPathDoc::JoiningTabs()
{
/*
	tabs are a cubic ramp section followed by a flat

	cubic ramp section:
	rise     =  s*s*(3-2*s)		s = [0 1], rise = [0 1]
	drise/ds =  6*s*(1-s)		s = [0 1]
*/

	if (m_bGotJoiningTabs)
		return;
	m_bGotJoiningTabs = true;

	NODEREF nrCheck;
	int iNCProgNum = 0;
//	CPathNode* pNode = GetFirstNode(nrCheck);
	CPathNode* pNode = GetFirstPathNode(nrCheck);
	while (!pNode->IsPoint())
	{
		if (pNode->type == PNT_PROGRAMNUMBER)
			iNCProgNum = (int)pNode->x;
		pNode = GetNextPathNode(nrCheck);
	}
	ASSERT(pNode->IsEndPoint());		// first will be end node

	for (;;)
	{
		if (pNode->type & PNTF_ENDPASS && pNode->z <= m_JoinTab.zLowPassMax && iNCProgNum == 1)	// end of pass & lowest pass of a cut
		{											// backtrack dist to rise up and leave joining tab
			NODEREF nrEnd = nrCheck;		// initial node of end   seg + sEnd
			NODEREF nrStart = nrCheck;		// initial node of start seg + sStart
			double sEnd = 0;
			double sStart = 0;
			double zRamp = m_JoinTab.height - pNode->z;
			bool bDistOK = true;
			ASSERT(m_diaTool > 0);
			double distEnd = m_JoinTab.width + m_diaTool;	// adjust for tool diameter
			double distStart = distEnd + m_JoinTab.rampLength;
			if (!MoveBackDist(nrEnd, sEnd, distEnd))
				bDistOK = false;
			if (!MoveBackDist(nrStart, sStart, distStart))
				bDistOK = false;
			if (bDistOK)			// check new node positions aren't at or very close to existing!!!!!!!
			{
				bool bInOneSegment = (nrStart == nrEnd);
				NODEREF nr = nrEnd;
				bool bEndPointInLine = (GetNextNode(nr)->IsEndPoint());
				SSegPolys seg;
				CPathNode pathNode;
				pathNode.type = PNT_CONTROLNODE;
				CPathNode arBez[4];
				nr = nrStart;

				if (bInOneSegment)		// in one segment
				{
					// add 4 nodes if in line, 6 if in bezier
					ASSERT(ConsecutiveSegNodes(nrStart));	// in start seg not consec insert other nodes after nrStart	
					if (bEndPointInLine)		// in a line
					{
						arBez[0] = *GetNode(nr);
						arBez[1] = *GetNextNode(nr);
						m_NodeArray.InsertAt(nrStart+1, pathNode, 4);
						nrCheck += 4;
						LineAdjustStartEnd(arBez+2, arBez, sStart, sEnd);
						Line2Bezier(&m_NodeArray[nrStart+1], arBez+2);
						m_NodeArray[nrStart+1].type = PNT_ENDNODE;
						m_NodeArray[nrStart+4].type = PNT_ENDNODE;
						for (nr = nrStart+2; nr < nrCheck;)		// starts next after 2
							GetNextNode(nr)->z += zRamp;
					}
					else		// in a bezier
					{
						arBez[0] = *GetNode(nr);
						for (int i = 1; i < 4; i++)
							arBez[i] = *GetNextNode(nr);
						m_NodeArray.InsertAt(nrStart, pathNode, 6);
						nrCheck += 6;
						BezierAdjustEnd(&m_NodeArray[nrStart], arBez, sStart);	// line if start seg is line
						BezierAdjustStartEnd(&m_NodeArray[nrStart+3], arBez, sStart, sEnd);
						BezierAdjustStart(&m_NodeArray[nrStart+6], arBez, sEnd);
						m_NodeArray[nrStart+0].type = PNT_ENDNODE;
						m_NodeArray[nrStart+3].type = PNT_ENDNODE;
						m_NodeArray[nrStart+6].type = PNT_ENDNODE;
						// adjust z height of nodes within flat section
						for (nr = nrStart+4; nr < nrCheck;)		// starts next after 4
							GetNextNode(nr)->z += zRamp;
					}
				}
				else		// across two or more segments
				{
					bool bStartPointInLine = (GetNextNode(nr)->IsEndPoint());
					ASSERT(ConsecutiveSegNodes(nrStart));	// in start seg not consec insert other nodes after nrStart	
					ASSERT(ConsecutiveSegNodes(nrEnd));	// in start seg not consec insert other nodes after nrStart	
					// add 6 nodes
					m_NodeArray.InsertAt(nrStart, pathNode, 6);	// adjust nr's by +6 !!
					nrCheck += 6;
					nrEnd += 6;
					NODEREF nrStartRamp;					// initial node of start ramp bezier segment
					NODEREF nrEndRamp = nrEnd-3;		// initial node of end   ramp bezier segment
					nr = nrStart+6;
					if (!bStartPointInLine)		// start point in bezier
					{
						nrStartRamp = nrStart+3;
						arBez[0] = *GetNode(nr);
						for (int i = 1; i < 4; i++)
							arBez[i] = *GetNextNode(nr);
						BezierAdjustEnd(&m_NodeArray[nrStart], arBez, sStart);	// line if start seg is line
						BezierAdjustStart(&m_NodeArray[nrStart+3], arBez, sStart);
						m_NodeArray[nrStart+0].type = PNT_ENDNODE;
						m_NodeArray[nrStart+3].type = PNT_ENDNODE;
					}
					else					// start point in line
					{
						nrStartRamp = nrStart+1;
						arBez[0] = *GetNode(nr);
						arBez[2] = *GetNextNode(nr);
						arBez[1] = (1-sStart)*arBez[0] + sStart*arBez[2];
						m_NodeArray[nrStart] = arBez[0];
						Line2Bezier(&m_NodeArray[nrStart+1], arBez+1);
						m_NodeArray[nrStart+1].type = PNT_ENDNODE;
					}

					// move segments between last node of start seg and first node of end seg back 3 places - adjust z !!!!!
					nr = nrStartRamp+4;
					for (NODEREF nrSrc = nr+3; nrSrc < nrEnd;)
						m_NodeArray[nr++] = m_NodeArray[nrSrc++];

					nr = nrEnd;
					if (!bEndPointInLine)		// end point in bezier
					{
						arBez[0] = *GetNode(nr);
						for (int i = 1; i < 4; i++)
							arBez[i] = *GetNextNode(nr);
						BezierAdjustEnd(&m_NodeArray[nrEnd-3], arBez, sEnd);		// line if end seg is line
						BezierAdjustStart(&m_NodeArray[nrEnd], arBez, sEnd);
						m_NodeArray[nrEnd-3].type = PNT_ENDNODE;
						m_NodeArray[nrEnd-1].type = PNT_CONTROLNODE;		// required if bStartPointInLine
					}
					else					// end point in line
					{
						arBez[0] = *GetNode(nr);
						arBez[2] = *GetNextNode(nr);
						arBez[1] = (1-sEnd)*arBez[0] + sEnd*arBez[2];
						Line2Bezier(&m_NodeArray[nrEnd-3], arBez);
						m_NodeArray[nrEnd-3].type = PNT_ENDNODE;
						m_NodeArray[nrEnd-1].type = PNT_CONTROLNODE;		// maybe required if bStartPointInLine
						m_NodeArray[nrEnd  ].type = PNT_ENDNODE;
					}


					// first ramp end and control node remain at original height
					// adjust z height of nodes within ramp
					if (nrEndRamp == nrStartRamp+3)		// if no segments between StartRamp and EndRamp segments
					{
						CPathNode* pNodeStartSeg = &m_NodeArray[nrStartRamp+3];		// end node between start and end ramp segments
						// calc z of nrStartRamp+3 endnode: s = dist(nrStartRamp->nrStartRamp+3) / m_dTabRampLength
						double lenPrev = BezierLengthByThirds(pNodeStartSeg-3);
						double lenNext = BezierLengthByThirds(pNodeStartSeg);
						double s = lenPrev / (lenPrev + lenNext);
						double h = s*s*(3-2*s) * zRamp;
						double grad = 6*s*(1-s) * zRamp / (lenPrev + lenNext);
						ASSERT(pNodeStartSeg->IsEndPoint());
						ASSERT((pNodeStartSeg-1)->IsControlPoint());
						ASSERT((pNodeStartSeg+1)->IsControlPoint());

						double lenCtrl;
						lenCtrl = (*(pNodeStartSeg-1) - *pNodeStartSeg).Mag();
						(pNodeStartSeg-1)->z += h - grad * lenCtrl;				// previous control node
						lenCtrl = (*(pNodeStartSeg+1) - *pNodeStartSeg).Mag();
						(pNodeStartSeg+1)->z += h + grad * lenCtrl;				// next control node
						pNodeStartSeg->z += h;					// endnode

					}
					else		// there are 1 or more segments between StartRamp and EndRamp segments
					{
						// calc s = [0 1] from initial node of StartRamp seg to final node of EndRamp seg
						// s increments based on segment lengths (approx chord lengths)
						// end nodes z = rise(s of node)
						// control nodes z = adjacent end node z + rise/ds(s of end node) / length ramp
						CPathNode* pNodeStartSeg;
						double arLen[20];			// should only need a couple max!
						double lenSum = 0;
						int numSegs = 0;
						// first get all segment lengths
						for (nr = nrStartRamp; nr <= nrEndRamp; nr++)
						{
							ASSERT(numSegs < 20);
							pNodeStartSeg = &m_NodeArray[nr];
							ASSERT(pNodeStartSeg->IsEndPoint());
							if ((pNodeStartSeg+1)->IsControlPoint())		// calc bezier or line segment length
							{
								nr += 2;
								ASSERT((pNodeStartSeg+2)->IsControlPoint());		// check 2 control points!
								lenSum += BezierLengthByThirds(pNodeStartSeg);
							}
							else
								lenSum += (*(pNodeStartSeg+1) - *pNodeStartSeg).Mag();
							arLen[numSegs++] = lenSum;
						}
						ASSERT(m_NodeArray[nr].IsEndPoint());		// check end point of last segment!

						int idxSeg = 0;
						// next set rise according to length ratio through ramp
						for (nr = nrStartRamp+3; nr <= nrEndRamp; nr++)		// start on last node of StartRamp bezier
						{
							double s = arLen[idxSeg++] / lenSum;
							double h = s*s*(3-2*s) * zRamp;
							double grad = 6*s*(1-s) * zRamp / lenSum;

							pNodeStartSeg = &m_NodeArray[nr];
							ASSERT(pNodeStartSeg->IsEndPoint());
							if ((pNodeStartSeg-1)->IsControlPoint())		// previous is bezier segment
							{
								double lenCtrl = (*(pNodeStartSeg-1) - *pNodeStartSeg).Mag();
								(pNodeStartSeg-1)->z += h - grad * lenCtrl;		// previous control node
							}

							if ((pNodeStartSeg+1)->IsControlPoint())			// next is bezier segment
							{
								nr += 2;
								ASSERT((pNodeStartSeg+2)->IsControlPoint());		// check 2 control points!
								double lenCtrl = (*(pNodeStartSeg+1) - *pNodeStartSeg).Mag();
								(pNodeStartSeg+1)->z += h + grad * lenCtrl;		// next control node
							}
							pNodeStartSeg->z += h;								// end node - adjust after control lengths calc'd!
						}
						ASSERT(idxSeg == numSegs-1);
						ASSERT(nr == nrEnd);
					}

					
					// adjust z height of last ramp control and end node and nodes within flat section
					for (nr = nrEnd-2; nr < nrCheck;)		// starts next after -2
						GetNextNode(nr)->z += zRamp;

				}
			}		// if (bDistOK)
			else
				ASSERT(0);		// why?  small piece? add zRamp to whole cut
		}		// not end of pass and last of cut

		for (;;)
		{
			pNode = GetNextPathNode(nrCheck);
			if (!pNode)			// end of path
				break;
			if (pNode->IsEndPoint())
				break;
			if (pNode->type == PNT_PROGRAMNUMBER)
				iNCProgNum = (int)pNode->x;
		}
		if (!pNode)			// end of path
			break;
	}	// for (;;)

}





