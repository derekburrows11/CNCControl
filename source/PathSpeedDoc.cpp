// PathSpeedDoc.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"

#include "PathDoc.h"
#include "CNCControlApp.h"		// only for template pointers

#include "PathAnimateView.h"

#include "PathSpeedDoc.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPathSpeedDoc

IMPLEMENT_DYNCREATE(CPathSpeedDoc, CDocument)

CPathSpeedDoc::CPathSpeedDoc()
{
	VERIFY(OnNewDocument());
	m_bProcessingPath = false;
}

BOOL CPathSpeedDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	m_pPathDoc = NULL;

	return TRUE;
}

CPathSpeedDoc::~CPathSpeedDoc()
{
}


BEGIN_MESSAGE_MAP(CPathSpeedDoc, CDocument)
	//{{AFX_MSG_MAP(CPathSpeedDoc)
	ON_COMMAND(ID_PATH_ANIMATE, OnShowPathAnimate)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPathSpeedDoc diagnostics

#ifdef _DEBUG
void CPathSpeedDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPathSpeedDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPathSpeedDoc serialization

void CPathSpeedDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPathSpeedDoc commands

void CPathSpeedDoc::SetPathDoc(CPathDoc* pPathDoc)
{
	ASSERT(m_pPathDoc == NULL);
	m_pPathDoc = pPathDoc;
	m_bArrayExtentsSet = false;
}

void CPathSpeedDoc::FindSpeeds()
{
	ASSERT_VALID(m_pPathDoc);
	bool bSaveResults = false;

	m_LimitList.SetSize(0, 1024);

	CPathSpeed pathSpeed;
	pathSpeed.SetPathDoc(m_pPathDoc);
	pathSpeed.SetLimitList(&m_LimitList);
	pathSpeed.Init();
	pathSpeed.SetScanMinTime(100);	// scan foward at least 100sec worth
	pathSpeed.m_bSaveLimits = bSaveResults;
	pathSpeed.SetToStart();

	TRACE0("Starting limit calculations\n");
	pathSpeed.FindSpeeds();
	TRACE0("Finished limit calculations\n");


	if (bSaveResults)
	{
		ofstream	os("Results Limits.txt");
		os << "Limit List\n";
		os << m_LimitList;		// print out limit list
		os.close();
	}


	CPathMove pathMove;
	pathMove.SetPathDoc(m_pPathDoc);
	pathMove.SetLimitList(&m_LimitList);
	pathMove.Init();
/*
	TRACE0("Calculating results at S\n");
	pathMove.SetToStart();
	pathMove.GetSpeedsAtS(m_MotionArray);
	TRACE0("GetSpeedsAtS Done\n");
	if (bSaveResults)
	{
		TRACE0("Saving results at S\n");
		os.open("Results S.txt");
		os << m_MotionArray;
		os.close();
	}
*/

	TRACE0("Calculating results at Time\n");
	pathMove.SetToStart();
	pathMove.GetSpeedsAtTime(m_MotionArray);	// overwrites previous m_MotionArray results
	TRACE0("GetSpeedsAtTime Done\n");
	if (bSaveResults)
	{
		TRACE0("Saving results at Time\n");
		ofstream	os("Results Time.txt");
		os << m_MotionArray;
		os.close();
	}

	FindBezierArray(m_BezMotionArray, m_MotionArray);	// bezier points stored in doc for displaying plots
	FindArrayExtents(m_MotionArray);



// following will be moved!
	CPathTimeStep pathStepper;
	pathStepper.SetPathDoc(m_pPathDoc);
	pathStepper.SetLimitList(&m_LimitList);
	pathStepper.Init();

	CControllerTracker* pTracker;
//	pTracker = &static_cast<CCNCControlApp*>(AfxGetApp())->m_ControllerPath;
	pTracker = &m_ContTracker;
	pTracker->m_bLogMotion = true;
	pTracker->Init();
	pathStepper.SetControllerTracker(pTracker);
	pathStepper.Init();

	TRACE0("Calculating controller time steps\n");
	SMotionStateBasic ms;
	pathStepper.GetPathStartPos(ms.vtPos);
	pTracker->SetCurrentMotion(ms);
	pTracker->SetCurrentMotionStopped();
	pathStepper.SetToStart();
	pathStepper.FindTimeSteps();
	TRACE0("Finished calculating controller time steps\n");

	// arrays to view
//	m_CtrlTrack.SetMotionArray(m_ActualMotionArray, m_CtrlTrack.m_iActualMotionArray);
	pTracker->SetActualMotionArray(m_ActualMotionArray);			// uses m_SegMotionArray
	pTracker->SetMotionArray(m_RequestMotionArray, pTracker->m_RequestMotionIArray);

//	m_CtrlTrack.SetMotionArray(m_SegMotionArray, m_CtrlTrack.m_SegMotionArray);

	FindBezierArray(m_BezActualMotionArray, m_ActualMotionArray);		// bezier points stored in doc for displaying plots
	FindBezierArray(m_BezRequestMotionArray, m_RequestMotionArray);	// bezier points stored in doc for displaying plots


	UpdateAllViews(NULL);		// first time there may be not views yet!

}


void CPathSpeedDoc::FindSpeedsStart()
{
	// used only when testing speeds, not when sending to machine
	// CControllerPath::StartSendingPath() used when sending to machine

	m_bSaveLimits = true;
	m_bSaveTimeResults = true;

	ASSERT_VALID(m_pPathDoc);
	m_LimitList.SetSize(0, 1024);

	// initialise path iterators

	m_PathSpeed.SetPathDoc(m_pPathDoc);
	m_PathSpeed.SetLimitList(&m_LimitList);
	m_PathSpeed.Init();
	m_PathSpeed.SetScanMinTime(100);	// scan foward at least 100sec worth
	m_PathSpeed.m_bSaveLimits = m_bSaveLimits;
	m_PathSpeed.SetToStart();

/*
	CPathMove pathMove;
	pathMove.SetPathDoc(m_pPathDoc);
	pathMove.SetLimitList(&m_LimitList);
	pathMove.Init();


	TRACE0("Calculating results at Time\n");
	pathMove.SetToStart();
	pathMove.GetSpeedsAtTime(m_MotionArray);	// overwrites previous m_MotionArray results
	TRACE0("GetSpeedsAtTime Done\n");
	if (m_bSaveTimeResults)
	{
		TRACE0("Saving results at Time\n");
		ofstream	os("Results Time.txt");
		os << m_MotionArray;
		os.close();
	}

	FindBezierArray(m_BezMotionArray, m_MotionArray);	// bezier points stored in doc for displaying plots
	FindArrayExtents(m_MotionArray);
*/

	m_PathStepper.SetPathDoc(m_pPathDoc);
	m_PathStepper.SetLimitList(&m_LimitList);
	m_PathStepper.Init();

	m_ContTracker.m_bLogMotion = true;
	m_ContTracker.Init();
	m_PathStepper.SetControllerTracker(&m_ContTracker);
	m_PathStepper.Init();

	SMotionStateBasic ms;
	m_PathStepper.GetPathStartPos(ms.vtPos);
	// set start pos to give appropriate servo positions
	LOGERROR("Running Find Speeds");
	CVector vtPosTipStart;
	//m_pPathDoc->GetStartPos(vtPosTipStart);
	vtPosTipStart = -m_pPathDoc->GetMinNodeValues();
	m_ContTracker.SetPosTipOfPathOrigin(vtPosTipStart);
	m_ContTracker.SetCurrentMotion(ms);
	m_ContTracker.SetCurrentMotionStopped();
	m_PathStepper.SetToStart();


	// finished setup, now calc limits and motions

	TRACE0("Starting limit calculations\n");
	m_bProcessingPath = true;
	m_PathSpeed.FindSpeeds();			// calc first section of limits
	m_PathStepper.FindInitialTimeStep();	// notify stepper of list change

	if (m_bSaveLimits)
	{
		ofstream	os("Results Limits.txt");
		os << "Limit List\n";
		os << m_LimitList;		// print out limit list
		os.close();
	}

	// ready to call FindSpeedsMore();
}

bool CPathSpeedDoc::FindSpeedsMore()
{
	// calc path speed segments in stages
	// clear motion arrays

	for (;;)
	{
		m_PathStepper.FindTimeSteps();	// will have added accel segments to buffer

		if (!m_PathStepper.AtEndOfCalcLimits())
			break;
		if (m_PathStepper.FinishedPathSteps())
		{
			m_bProcessingPath = false;
			TRACE0("Finished limit calculations\n");
			break;
		}
		char strMsg[128];
		sprintf(strMsg, "Got to segment %i of %i.  Calc more limits (in PathSpeedDoc)?", GetLastScanSegment(), m_pPathDoc->GetNumSegments());
		if (AfxMessageBox(strMsg, MB_YESNO) != IDYES)
			break;
		m_PathSpeed.FindSpeeds();		// find next section limits
		m_PathStepper.LimitListChanged();
		if (m_bSaveLimits)
		{
			ofstream	os("Results Limits.txt");
			os << "Limit List\n";
			os << m_LimitList;		// print out limit list
			os.close();
		}
	}

	// arrays to view
//	m_ContTracker.SetMotionArray(m_ActualMotionArray, m_ContTracker.m_iActualMotionArray);
	m_ContTracker.SetActualMotionArray(m_ActualMotionArray);			// uses m_SegMotionArray
	m_ContTracker.SetMotionArray(m_RequestMotionArray, m_ContTracker.m_RequestMotionIArray);
//	m_ContTracker.SetMotionArray(m_SegMotionArray, m_ContTracker.m_SegMotionArray);

	FindBezierArray(m_BezActualMotionArray, m_ActualMotionArray);		// bezier points stored in doc for displaying plots
	FindBezierArray(m_BezRequestMotionArray, m_RequestMotionArray);	// bezier points stored in doc for displaying plots

	UpdateAllViews(NULL);		// first time there may be not views yet!

	return m_bProcessingPath;
}

void CPathSpeedDoc::FindSpeedsAllLimits()	// runs through all limit calc's for to check for problems
{
	// send path speed segments to controller in stages
	for (;;)
	{
		if (m_PathSpeed.AtEndOfPath())
			break;
		m_PathSpeed.FindSpeeds();		// find next section limits
	}
}



void CPathSpeedDoc::OnShowPathAnimate()
{
// Launch a CPathAnimateView window
	// find CDocTemplate for CPathAnimateDoc - it's in the CWinApp
//	CDocTemplate* pDocTemplate = ((CCNCControlApp*)AfxGetApp())->m_pPathAnimateDocTemplate;

/*	CDocTemplate* pDocTemplate;
	CWinApp* pApp = AfxGetApp();
	POSITION pos = pApp->GetFirstDocTemplatePosition();
	while (pos)
	{
		pDocTemplate = pApp->GetNextDocTemplate(pos);
		ASSERT_VALID(pDocTemplate);

	}
*/
/*	ASSERT_VALID(pDocTemplate);
//	CFrameWnd* pFrame = pDocTemplate->CreateNewFrame(this, NULL);		// set doc to CPathSpeedDoc
	CFrameWnd* pFrame = pDocTemplate->CreateNewFrame(m_pPathDoc, NULL);	// set doc to CPathDoc

// find new CPathAnimateView object to set CPathSpeedDoc object
	CWnd* pWnd = pFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST);
	ASSERT(pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CPathAnimateView)));
	((CPathAnimateView*)pWnd)->SetSpeedDoc(this);			// set doc to CPathSpeedDoc
*/
/*
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
*/
//	pDocTemplate->InitialUpdateFrame(pFrame, NULL);

	

}






void CPathSpeedDoc::FindBezierArray(CMotionStateBasicArray& arrBez, const CMotionStateBasicArray& arr)
{
	int numPts = arr.GetSize();
	arrBez.SetSize(3 * numPts - 2);

	int idxBez = 0;
	arrBez[idxBez++] = arr[0];
	for (int i = 1; i < numPts; i++)
	{
		const SMotionStateBasic& prevMot = arr[i-1];	// accel of node is continuous before and after node
		const SMotionStateBasic& currMot = arr[i];
		SMotionStateBasic& c1Mot = arrBez[idxBez++];	// 1/3 control node
		SMotionStateBasic& c2Mot = arrBez[idxBez++];	// 2/3 control node
		arrBez[idxBez++] = currMot;

		double dton3 = 1.0/3 * (currMot.t - prevMot.t);
		c1Mot.t = prevMot.t + dton3;
		c2Mot.t = currMot.t - dton3;

		c1Mot.vtPos = prevMot.vtPos + dton3 * prevMot.vtVel;
		c2Mot.vtPos = currMot.vtPos - dton3 * currMot.vtVel;

		c1Mot.vtVel = prevMot.vtVel + dton3 * prevMot.vtAcc;
		c2Mot.vtVel = currMot.vtVel - dton3 * currMot.vtAcc;

		c1Mot.vtAcc = prevMot.vtAcc + dton3 * prevMot.vtJerk;
		c2Mot.vtAcc = currMot.vtAcc - dton3 * currMot.vtJerk;
	}
	ASSERT(idxBez == 3 * numPts - 2);
}

void CPathSpeedDoc::FindArrayExtents(const CMotionStateBasicArray& arr)
{
	if (arr.GetSize() == 0)
	{
		m_bArrayExtentsSet = false;
		return;
	}
	SMotionStateBasic minMot = arr[0];
	SMotionStateBasic maxMot = minMot;
	int numPts = arr.GetSize();
	for (int i = 1; i < numPts; i++)
	{
		const SMotionStateBasic& mot = arr[i];
		if (minMot.t > mot.t) minMot.t = mot.t;
		if (maxMot.t < mot.t) maxMot.t = mot.t;
		if (minMot.s > mot.s) minMot.s = mot.s;
		if (maxMot.s < mot.s) maxMot.s = mot.s;
		if (minMot.dsdt > mot.dsdt) minMot.dsdt = mot.dsdt;
		if (maxMot.dsdt < mot.dsdt) maxMot.dsdt = mot.dsdt;
		for (int pva = 0; pva < 4; pva++)
		{
			minMot.vtPVAJ[pva].Min(mot.vtPVAJ[pva]);
			maxMot.vtPVAJ[pva].Max(mot.vtPVAJ[pva]);
		}
	}
	m_MinArrayVals = minMot;
	m_MaxArrayVals = maxMot;
	m_bArrayExtentsSet = true;
}

void CPathSpeedDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsModified());
}


