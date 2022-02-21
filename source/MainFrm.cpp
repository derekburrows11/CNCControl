// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include <dde.h>        // for DDE requests
#include <process.h>

#include "CNCControl.h"
#include "Settings.h"

#include "ChildFrm.h"
#include "DiagnoseDlg.h"
#include "MachinePathSegDlg.h"
#include "MachinePropDlg.h"
#include "SetToolDlg.h"
#include "PosConverter.h"
#include "ManualControlView.h"
#include "MachineStatusView.h"
#include "ProbeSampleView.h"
#include "ParamDoc.h"
#include "GeneralOptionsDlg.h"


#include "CNCControlApp.h"		// to access CCNCComms


#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



int Plot(const double* arX, const double* arY, int numPoints, int plotStyle)
{
	// initiate DDE and the send data to "Graph"
	CWnd* pMainWnd = AfxGetMainWnd();
	ASSERT(pMainWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	return ((CMainFrame*)pMainWnd)->SendPlot(arX, arY, numPoints, plotStyle);
}

int Plot(const float* arX, const float* arY, int numPoints, int plotStyle)
{
	// initiate DDE and the send data to "Graph"
	CWnd* pMainWnd = AfxGetMainWnd();
	ASSERT(pMainWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	return ((CMainFrame*)pMainWnd)->SendPlot(arX, arY, numPoints, plotStyle);
}

int Plot(const CVect2* arPt, int numPoints, int plotStyle)
{
	CWnd* pMainWnd = AfxGetMainWnd();
	ASSERT(pMainWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	return ((CMainFrame*)pMainWnd)->SendPlot(arPt, numPoints, plotStyle);
}

int Plot(const CVect2f* arPt, int numPoints, int plotStyle)
{
	CWnd* pMainWnd = AfxGetMainWnd();
	ASSERT(pMainWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	return ((CMainFrame*)pMainWnd)->SendPlot(arPt, numPoints, plotStyle);
}

int PlotCommand(int command, int p1, int p2)
{
	CWnd* pMainWnd = AfxGetMainWnd();
	ASSERT(pMainWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	return ((CMainFrame*)pMainWnd)->SendPlotCommand(command, p1, p2);
}









/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_MACHINE_DIAGNOSEBOX, OnMachineDiagnoseBox)
	ON_WM_CLOSE()
	ON_COMMAND(ID_MACHINE_PROPERTIES, OnMachineProperties)
	ON_COMMAND(ID_PATH_SEGMENTTRACKING, OnPathSegmentTracking)
	ON_COMMAND(ID_MACHINE_MANUALCONT, OnMachineManualCont)
	ON_COMMAND(ID_MANUALCONTROL_CLOSED, OnManualControlViewClosed)
	ON_WM_DESTROY()
	ON_COMMAND(ID_MACHINE_SHOWMACHINESTATUS, OnMachineStatus)
	ON_COMMAND(ID_MACHINE_SETTOOL, OnMachineSetTool)
	ON_COMMAND(ID_MACHINE_PROBESAMPLE, OnMachineProbeSample)
	ON_COMMAND(ID_PROBESAMPLE_CLOSED, OnProbeSampleViewClosed)
	ON_COMMAND(ID_OPTIONS_SETTINGS, OnOptionsSettings)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(IDW_FIRST_BAR, IDW_LAST_BAR, OnViewToolBar)
	ON_UPDATE_COMMAND_UI_RANGE(IDW_FIRST_BAR, IDW_LAST_BAR, OnUpdateViewToolBar)

	ON_MESSAGE(WMU_UPDATEPARAM, OnUpdateParam)
	ON_MESSAGE(WMU_MEMVALUE, OnMemValue)
	ON_MESSAGE(WMU_SETPOSITIONINDICATOR, OnSetPosIndicator)

	ON_MESSAGE(WMU_SETACTIVEPATHVIEW, OnSetActivePathView)

	// message handling for standard DDE commands
//	ON_MESSAGE(WM_DDE_INITIATE, OnDDEInitiate)		// these 3 also handled by CFrameWnd
//	ON_MESSAGE(WM_DDE_EXECUTE, OnDDEExecute)			// these 3 also handled by CFrameWnd
//	ON_MESSAGE(WM_DDE_TERMINATE, OnDDETerminate)		// these 3 also handled by CFrameWnd
	ON_MESSAGE(WM_DDE_ACK, OnDDEAcknowledge)
END_MESSAGE_MAP()

/*
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_POSITION,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};
*/
/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_pDiagnoseDlg = NULL;
	m_pManualControlView = NULL;
	m_pMachineStatusView = NULL;
	m_pProbeSampleView = NULL;

	m_pActivePathView = NULL;

	m_iPlotMsgStatus = PMS_NONE;
	m_hGraphWnd = NULL;
	m_iNumGraphInst = 0;

	m_szStorageLabel = "MainFrame";

}

CMainFrame::~CMainFrame()
{
	if (m_pDiagnoseDlg)
		CloseDiagnoseBox();
}

//////////////////////////////////

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
/*	cs.cy = 800;		// defaults if size and pos not set
	cs.cx = 1000;
	cs.y = 100;
	cs.x = 500;
*/
	return CMDIFrameWnd::PreCreateWindow(cs);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// SetWindowPlacement can be before or after (CMDIFrameWnd::OnCreate....
	WINDOWPLACEMENT wp;
	if (ReadWindowPlacement(&wp, m_szStorageLabel))
	{
		AfxGetApp()->m_nCmdShow = wp.showCmd;	// frame is shown using this in CFrameWnd::InitialUpdateFrame
		wp.showCmd = SW_HIDE;
		SetWindowPlacement(&wp);
	}

	UINT indicators[] =
	{
		ID_SEPARATOR,           // status line indicator
		ID_INDICATOR_POSITION,
		ID_INDICATOR_CAPS,
		ID_INDICATOR_NUM,
		ID_INDICATOR_SCRL,
	};

//	int iStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP;
	int iStyle = WS_CHILD | CBRS_TOP;
	iStyle |= CBRS_SIZE_DYNAMIC | CBRS_FLYBY;	// | CBRS_TOOLTIPS;
	int iFailed = 0;
	if (!m_wndStatusBar.Create(this) ||
			!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))														iFailed++;
	if (!m_wndStandardBar.Create(this, iStyle, AFX_IDW_TOOLBAR)			|| !m_wndStandardBar.LoadToolBar(IDR_MAINFRAME))			iFailed++;
	if (!m_wndMotionPlotBar.Create(this, iStyle, IDW_MOTIONPLOT_BAR)	|| !m_wndMotionPlotBar.LoadToolBar(IDR_MOTIONPLOTBAR))	iFailed++;
	if (!m_wndViewBar.Create(this, iStyle, IDW_VIEW_BAR)					|| !m_wndViewBar.LoadToolBar(IDR_VIEWBAR))					iFailed++;
	if (!m_wndViewDirBar.Create(this, iStyle, IDW_VIEWDIR_BAR)			|| !m_wndViewDirBar.LoadToolBar(IDR_VIEWDIRBAR))			iFailed++;
	if (!m_wndPathDrawBar.Create(this, iStyle, IDW_PATH_BAR)				|| !m_wndPathDrawBar.LoadToolBar(IDR_PATHBAR))				iFailed++;
	if (!m_wndPlayerBar.Create(this, iStyle, IDW_PLAYER_BAR)				|| !m_wndPlayerBar.LoadToolBar(IDR_PLAYERBAR))				iFailed++;

	if (iFailed != 0)
	{
		TRACE1("Failed to create %i tool bars\n", iFailed);
		return -1;      // fail to create
	}

	m_wndStandardBar.SetWindowText("Standard");
	m_wndMotionPlotBar.SetWindowText("Path Speed Plot");
	m_wndViewBar.SetWindowText("View Zoom");
	m_wndViewDirBar.SetWindowText("View Direction");
	m_wndPathDrawBar.SetWindowText("Path Drawing");
	m_wndPlayerBar.SetWindowText("Path Animate");

	EnableDocking(CBRS_ALIGN_ANY);
	m_wndStandardBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndStandardBar);
	m_wndMotionPlotBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndMotionPlotBar);
	m_wndViewBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndViewBar);
	m_wndViewDirBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndViewDirBar);
	m_wndPathDrawBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndPathDrawBar);
	m_wndPlayerBar.EnableDocking(CBRS_ALIGN_ANY);
//	FloatControlBar(&m_wndPlayerBar, CPoint(300,90));
//	DockControlBar(&m_wndPlayerBar);

	LoadBarState("BarState");

//	ShowControlBar(&m_wndPlayerBar, false, true);	// is activated when got speeds
//	int v = m_wndPlayerBar.GetStyle() & WS_VISIBLE;
//	m_wndPlayerBar.ShowWindow(SW_HIDE);
//	FloatControlBar(&m_wndPlayerBar, CPoint(300,90));

/*
	if (!m_wndPathDrawBar.Create(this) ||
		!m_wndPathDrawBar.LoadToolBar(IDR_PATHTOOLBAR))
	{
		TRACE0("Failed to create path toolbar\n");
		return -1;      // fail to create
	}
	m_wndPathDrawBar.SetBarStyle(m_wndPathDrawBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndPathDrawBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndPathDrawBar, AFX_IDW_DOCKBAR_TOP);
*/

	return 0;
}

void CMainFrame::OnClose() 
{
	SaveBarState("BarState");

	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnDestroy() 
{
//	SaveBarState("BarState");	// animation player bar removed before here

	WINDOWPLACEMENT wp;
	if (GetWindowPlacement(&wp))
		WriteWindowPlacement(&wp, m_szStorageLabel);

	CMDIFrameWnd::OnDestroy();
	// TODO: Add your message handler code here
	
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG



/*
void CMainFrame::PlotGraph()
{
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_GRAPHVIEW,
		RUNTIME_CLASS(CDocument),
		RUNTIME_CLASS(CMDIChildWnd),
		RUNTIME_CLASS(CGraphView));
	CFrameWnd* pFrame = pDocTemplate->CreateNewFrame(NULL, NULL);

	// find new CParamView object to set view type
//	CWnd* pWnd = pFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
//	if (pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CParamView)))
//		((CParamView*)pWnd)->SetViewType(nViewType);			// 1 for axis parameters or 0 for general

	pDocTemplate->InitialUpdateFrame(pFrame, NULL);

	AfxGetApp->AddDocTemplate(pDocTemplate);		// why??
}
*/


/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers



void CMainFrame::OnMachineDiagnoseBox() 
{
	if (!m_pDiagnoseDlg)
	{
		m_pDiagnoseDlg = new CDiagnoseDlg;
		m_pDiagnoseDlg->ShowWindow(SW_SHOW);
		m_pDiagnoseDlg->m_pCNCComms = &((CCNCControlApp*)AfxGetApp())->m_CNCComms;
	}
	else			// bring it into view
	{
		m_pDiagnoseDlg->ShowWindow(SW_SHOW);
	}
}

void CMainFrame::CloseDiagnoseBox()
{
	// called by dialog box when closing
	m_pDiagnoseDlg->DestroyWindow();
	delete m_pDiagnoseDlg;
	m_pDiagnoseDlg = NULL;
}

LRESULT CMainFrame::OnMemValue(WPARAM, LPARAM)
{
	TRACE0("Got Mem Value message in MainFrame user thread\n");
	return 0;
}

LRESULT CMainFrame::OnSetPosIndicator(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
	{
		if (!m_wndStatusBar.m_hWnd)
			return 0;
		m_wndStatusBar.SetPaneText(m_wndStatusBar.CommandToIndex(ID_INDICATOR_POSITION), "");
	}
	else if (wParam >= 1 && wParam <=3)
		SetStatusPosition(*(CVect2*)lParam);
	else
		ASSERT(0);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// SendPlot message handlers

int CMainFrame::SendPlot(const double* arX, const double* arY, int numPoints, int plotStyle /*=GPS_LINE*/)
{
	SGraphPlotInfo gpi;
	gpi.plotStyle = plotStyle;
	gpi.numPoints = numPoints;
	gpi.numCoords = 2;
	gpi.pointFormat = GPF_DOUBLE;
	HGLOBAL hData = AllocateMemToSendPlot(gpi);
	CVect2* arPoint = gpi.arPoints;		// can be cast to point to other point types
	for (int i = 0; i < numPoints; i++)
	{
		arPoint[i].x = arX[i];
		arPoint[i].y = arY[i];
	}
	return SendGraphInfo(hData);
}
int CMainFrame::SendPlot(const float* arX, const float* arY, int numPoints, int plotStyle /*=GPS_LINE*/)
{
	SGraphPlotInfo gpi;
	gpi.plotStyle = plotStyle;
	gpi.numPoints = numPoints;
	gpi.numCoords = 2;
	gpi.pointFormat = GPF_FLOAT;
	HGLOBAL hData = AllocateMemToSendPlot(gpi);
	CVect2f* arPoint = (CVect2f*)gpi.arPoints;		// can be cast to point to other point types
	for (int i = 0; i < numPoints; i++)
	{
		arPoint[i].x = arX[i];
		arPoint[i].y = arY[i];
	}
	return SendGraphInfo(hData);
}
int CMainFrame::SendPlot(const CVect2* arPt, int numPoints, int plotStyle /*=GPS_LINE*/)
{
	SGraphPlotInfo gpi;
	gpi.plotStyle = plotStyle;
	gpi.numPoints = numPoints;
	gpi.numCoords = 2;
	gpi.pointFormat = GPF_DOUBLE;
	HGLOBAL hData = AllocateMemToSendPlot(gpi);
	CVect2* arPoint = gpi.arPoints;		// can be cast to point to other point types
	for (int i = 0; i < numPoints; i++)
		arPoint[i] = arPt[i];
	return SendGraphInfo(hData);
}
int CMainFrame::SendPlot(const CVect2f* arPt, int numPoints, int plotStyle /*=GPS_LINE*/)
{
	SGraphPlotInfo gpi;
	gpi.plotStyle = plotStyle;
	gpi.numPoints = numPoints;
	gpi.numCoords = 2;
	gpi.pointFormat = GPF_FLOAT;
	HGLOBAL hData = AllocateMemToSendPlot(gpi);
	CVect2f* arPoint = (CVect2f*)gpi.arPoints;		// can be cast to point to other point types
	for (int i = 0; i < numPoints; i++)
		arPoint[i] = arPt[i];
	return SendGraphInfo(hData);
}
int CMainFrame::SendPlotCommand(int command, int p1 /*=0*/, int p2 /*=0*/)
{
	SGraphPlotInfo gpi;
	gpi.plotStyle = GPS_COMMAND;
	gpi.numPoints = 3;
	gpi.numCoords = 1;
	gpi.pointFormat = GPF_COMMAND;
	HGLOBAL hData = AllocateMemToSendPlot(gpi);
	int* arParam = (int*)gpi.arPoints;		// can be cast to point to other point types
		arParam[0] = command;
		arParam[1] = p1;
		arParam[2] = p2;
	return SendGraphInfo(hData);
}

HGLOBAL CMainFrame::AllocateMemToSendPlot(SGraphPlotInfo& gpi)
{
	int pointSize = (gpi.pointFormat & GPF_SIZE) * gpi.numCoords;
	int memSize = sizeof(DDEPOKE) + sizeof(SGraphPlotInfo) + gpi.numPoints * pointSize;
	HGLOBAL hData = GlobalAlloc(GMEM_DDESHARE, memSize);
	DDEPOKE* pDdePoke = (DDEPOKE*)GlobalLock(hData);

	pDdePoke->fRelease = 1;		// server is responsible for freeing hData
	pDdePoke->cfFormat = 0;
	m_bDDEPOKEfRelease = (pDdePoke->fRelease != 0);
	gpi.arPoints = (CVect2*)(pDdePoke->Value + sizeof(SGraphPlotInfo));	// offsetin bytes, an actual pointer can't be transferred between apps - put points after SGraphPlotInfo
	*(SGraphPlotInfo*)pDdePoke->Value = gpi;		// copy SGraphPlotInfo into memory
	return hData;
}

int CMainFrame::SendGraphInfo(HGLOBAL hData)
{
	GlobalUnlock(hData);
	if (!CheckGraphRunning())
	{
		VERIFY(GlobalFree(hData) == 0);
		return 0;
	}
	// Now send the graph data
	// must use a graph window from acknowledge to continue
	m_hDataPOKE = hData;
	m_iPlotMsgStatus = PMS_SENDING_DATA;
	ATOM aItem = GlobalAddAtom("data");
	VERIFY(::PostMessage(m_hGraphWnd, WM_DDE_POKE, (WPARAM)m_hWnd,
			PackDDElParam(WM_DDE_POKE, (UINT)hData, (UINT)aItem)));
	return 1;
}

int CMainFrame::CheckGraphRunning()
{
//	if (m_hGraphWnd)		// Graph should be running
//		return 2;

	m_iNumGraphInst = 0;
	m_iPlotMsgStatus = PMS_INITIATING;

	m_aApp = GlobalAddAtom("Graph");		// application to send to
	m_aTopic = GlobalAddAtom("Plot");	// topic for plotting data

	::SendMessage(HWND_BROADCAST, WM_DDE_INITIATE, (WPARAM)m_hWnd,
			MAKELPARAM(m_aApp, m_aTopic));			// waits for return
	// any instances of 'Graph' will have acknowledged when SendMessage returns!
	if (m_iNumGraphInst == 0)
	{
		// _execl, _spawnl, system, CreateProcess  all start a program
//		system("c:\\derek\\vc\\graph\\debug\\graph.exe");		// need to wait for full initialisation
		HANDLE hProcess = (HANDLE)_spawnl(_P_NOWAIT,
				"c:\\derek\\vc\\graph\\debug\\graph", "notmuch", NULL);
		if (hProcess != (HANDLE)-1)
			WaitForInputIdle(hProcess, 2000);
		::SendMessage(HWND_BROADCAST, WM_DDE_INITIATE, (WPARAM)m_hWnd,
				MAKELPARAM(m_aApp, m_aTopic));			// waits for return
	}
	VERIFY(GlobalDeleteAtom(m_aApp) == 0);
	VERIFY(GlobalDeleteAtom(m_aTopic) == 0);
	m_aApp = m_aTopic = 0;

	if (m_iNumGraphInst == 0)
	{
		MessageBox("Can't start 'c:\\derek\\vc\\graph\\debug\\Graph.exe'");
		return 0;
	}
	ASSERT(m_hGraphWnd != NULL);
	return 1;
}




void CMainFrame::OnPathSegmentTracking() 
{
	CMachinePathSegDlg dlg;
	dlg.LoadDialogData();

	if (dlg.DoModal() == IDOK)
	{
		dlg.SaveDialogData();
		g_Settings.Modified();
	}
}

void CMainFrame::OnMachineProperties() 
{
	CMachinePropDlg dlg;
	dlg.SetData(g_Settings.MachParam);

	if (dlg.DoModal() == IDOK)
	{
		dlg.GetData(g_Settings.MachParam);
		g_Settings.Modified();
	}
}

void CMainFrame::SetStatusPosition(const CVect2& ptPos)
{
	if (!m_wndStatusBar.m_hWnd)
		return;
	char sz[50];
	sprintf(sz, "%#10.5f  %#10.5f", ptPos.x, ptPos.y);	
	m_wndStatusBar.SetPaneText(m_wndStatusBar.CommandToIndex(ID_INDICATOR_POSITION), sz);
}



/////////////////////////////////////////////////////////////////////////////
// DDE message handlers


LRESULT CMainFrame::OnDDEAcknowledge(WPARAM wParam, LPARAM lParam)
{
	HWND hFromWnd = (HWND)wParam;

	if (m_iPlotMsgStatus == PMS_INITIATING)
	{
		ATOM aApp = (ATOM)LOWORD(lParam);
		ATOM aTopic = (ATOM)HIWORD(lParam);
		if (aApp != 0 && aTopic != 0
				&& aApp == m_aApp && aTopic == m_aTopic)		// check names
		{
			m_iNumGraphInst++;
			m_hGraphWnd = hFromWnd;
			VERIFY(GlobalDeleteAtom(aApp) == 0);
			VERIFY(GlobalDeleteAtom(aTopic) == 0);
		}
	}
	else if (m_iPlotMsgStatus == PMS_SENDING_DATA)
	{
		ATOM aItem;
		DDEACK wStatus;
		VERIFY(UnpackDDElParam(WM_DDE_ACK, lParam, (UINT*)&wStatus, (UINT*)&aItem));
		VERIFY(FreeDDElParam(WM_DDE_ACK, lParam));
		char szItem[10];
		GlobalGetAtomName(aItem, szItem, sizeof(szItem)-1);
		VERIFY(stricmp(szItem, "data") == 0);

		if (!wStatus.fAck || !m_bDDEPOKEfRelease)
		{
			VERIFY(GlobalFree(m_hDataPOKE) == 0);
			m_hDataPOKE = NULL;
		}
	}

	return 0L;
}

LRESULT CMainFrame::OnDDETerminate(WPARAM wParam, LPARAM lParam)
{
	::PostMessage((HWND)wParam, WM_DDE_TERMINATE, (WPARAM)m_hWnd, lParam);
	return 0L;
}


void CMainFrame::OnViewToolBar(UINT nID) 
{
//	MSG* pMsg = GetCurrentMessage();		// could use if nID not given
	CControlBar* pBar = GetControlBar(nID);
	if (pBar == NULL)
		if (nID == IDW_PATHPOSITION_BAR)	// is actually dialog
		{
			CWnd* pWnd = &m_wndPathPosDlg;		// only need CWnd*
			if (pWnd->m_hWnd)
				pWnd->ShowWindow((pWnd->GetStyle() & WS_VISIBLE) ? SW_HIDE : SW_SHOW);
			else
				m_wndPathPosDlg.Create();
		}
		else
			ASSERT(0);
	else
		ShowControlBar(pBar, (pBar->GetStyle() & WS_VISIBLE) == 0, true);
}

void CMainFrame::OnUpdateViewToolBar(CCmdUI* pCmdUI) 
{
	CControlBar* pBar = GetControlBar(pCmdUI->m_nID);
	if (pBar == NULL)
		if (pCmdUI->m_nID == IDW_PATHPOSITION_BAR)	// is actually dialog
			pBar = (CControlBar*)&m_wndPathPosDlg;		// only need CWnd*
		else
			ASSERT(0);
	if (pBar->m_hWnd == NULL)		// only for IDW_PATHPOSITION_BAR
		pCmdUI->SetCheck(0);
	else
		pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
}

void CMainFrame::OnManualControlViewClosed()
{
	ASSERT(m_pManualControlView != NULL);
	m_pManualControlView = NULL;
}

void CMainFrame::OnMachineManualCont() 
{
	// just reactivate if exists
	if (m_pManualControlView != NULL)
	{
		CFrameWnd* pFrame = m_pManualControlView->GetParentFrame();
		ASSERT(pFrame != NULL);
		pFrame->ActivateFrame();
		return;
	}

	// try to see if a CManualControlView window is one of MainFrm's??
//	CWnd* pTW = GetTopWindow();
//	CWnd* pDW = GetDescendantWindow(IDR_MANUALCONTVIEW);
//	CWnd* pC = GetWindow(GW_CHILD);
//	CWnd* pN = GetWindow(GW_HWNDNEXT);



	CRuntimeClass* pFrameClass = RUNTIME_CLASS(CChildFrame);
	CFrameWnd* pFrame = (CFrameWnd*)pFrameClass->CreateObject();
	if (pFrame == NULL)
	{
		TRACE1("Warning: Dynamic create of frame %hs failed.\n",
			pFrameClass->m_lpszClassName);
		return;
	}

	((CChildFrame*)pFrame)->SetStorageLabel("ManualController");

	CCNCControlApp* pApp = (CCNCControlApp*)AfxGetApp();

	CCreateContext context;
	context.m_pCurrentFrame = NULL;
	context.m_pCurrentDoc = pApp->m_pParamDoc;
	context.m_pNewViewClass = RUNTIME_CLASS(CManualControlView);
	context.m_pNewDocTemplate = NULL;


//	int iStyle = WS_OVERLAPPED | WS_BORDER | WS_DLGFRAME | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	int iStyle = WS_OVERLAPPED | WS_BORDER | WS_DLGFRAME | WS_SYSMENU;
	VERIFY(pFrame->LoadFrame(IDR_MANUALCONTVIEW,
//			(WS_OVERLAPPEDWINDOW & ~WS_SYSMENU) | FWS_ADDTOTITLE,   // default frame styles
			(iStyle) | 0,   // default frame styles
			NULL, &context));

	pFrame->InitialUpdateFrame(NULL, true);


	CView* pView = pFrame->GetActiveView();
	ASSERT(pView != NULL);
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CManualControlView)));
//	((CDocument*)pApp->m_pParamDoc)->AddView(pView);		// if not in createcontext
	m_pManualControlView = (CManualControlView*)pView;
	m_pManualControlView->SetCNCComms(&pApp->m_CNCComms);
	m_pManualControlView->SetControllerPath(&pApp->m_ControllerPath);
}

void CMainFrame::OnProbeSampleViewClosed()
{
	ASSERT(m_pProbeSampleView != NULL);
	m_pProbeSampleView = NULL;
}

void CMainFrame::OnMachineProbeSample() 
{
	// just reactivate if exists
	if (m_pProbeSampleView != NULL)
	{
		CFrameWnd* pFrame = m_pProbeSampleView->GetParentFrame();
		ASSERT(pFrame != NULL);
		pFrame->ActivateFrame();
		return;
	}

	// create a new view
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_PROBESAMPLEVIEW,
		RUNTIME_CLASS(CParamDoc),
		RUNTIME_CLASS(CChildFrame),
		RUNTIME_CLASS(CProbeSampleView));
	AfxGetApp()->AddDocTemplate(pDocTemplate);		// required??

	CFrameWnd* pFrame = pDocTemplate->CreateNewFrame(((CCNCControlApp*)AfxGetApp())->m_pParamDoc, NULL);
	ASSERT(pFrame != NULL);
	((CChildFrame*)pFrame)->SetStorageLabel("ProbeSampler");
	pDocTemplate->InitialUpdateFrame(pFrame, NULL);
	CView* pView = pFrame->GetActiveView();
	ASSERT(pView != NULL);
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CProbeSampleView)));
	m_pProbeSampleView = (CProbeSampleView*)pView;
}

void CMainFrame::OnMachineManualCont2() 
{
	CFrameWnd* pFrame = new CMDIChildWnd;
	if (pFrame == NULL)
	{
		TRACE0("Warning: Dynamic new create of frame %hs failed.\n");
		return;
	}

	int nIDResource = IDR_MANUALCONTVIEW;

	
	VERIFY(pFrame->LoadFrame(nIDResource,
//			(WS_OVERLAPPEDWINDOW & ~WS_SYSMENU) | FWS_ADDTOTITLE,   // default frame styles
			(WS_OVERLAPPEDWINDOW) | 0,   // default frame styles
			NULL, NULL));

	CCreateContext context;
	context.m_pCurrentFrame = NULL;
	context.m_pCurrentDoc = NULL;
	context.m_pNewViewClass = RUNTIME_CLASS(CManualControlView);
	context.m_pNewDocTemplate = NULL;

//	pFrame->CreateView(&context);

	CManualControlView* pView = new CManualControlView;
	pFrame->SetActiveView(pView);

	pFrame->InitialUpdateFrame(NULL, true);

}

void CMainFrame::OnMachineStatus() 
{
	if (m_pMachineStatusView != NULL)
		return;

	CRuntimeClass* pFrameClass = RUNTIME_CLASS(CChildFrame);
	CFrameWnd* pFrame = (CFrameWnd*)pFrameClass->CreateObject();
	if (pFrame == NULL)
	{
		TRACE1("Warning: Dynamic create of frame %hs failed.\n",
			pFrameClass->m_lpszClassName);
		return;
	}
	((CChildFrame*)pFrame)->SetStorageLabel("MachineStatus");

	CCNCControlApp* pApp = (CCNCControlApp*)AfxGetApp();

	int nIDResource = IDR_MACHINESTATUSVIEW;
	ASSERT(nIDResource != 0); // must have a resource ID to load from
	CCreateContext context;
	context.m_pCurrentFrame = NULL;
	context.m_pCurrentDoc = (CDocument*)pApp->m_pParamDoc;
	context.m_pNewViewClass = RUNTIME_CLASS(CMachineStatusView);
	context.m_pNewDocTemplate = NULL;


//	int iStyle = WS_OVERLAPPED | WS_BORDER | WS_DLGFRAME | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	int iStyle = WS_OVERLAPPED | WS_BORDER | WS_DLGFRAME | WS_SYSMENU;
	VERIFY(pFrame->LoadFrame(nIDResource,
//			(WS_OVERLAPPEDWINDOW & ~WS_SYSMENU) | FWS_ADDTOTITLE,   // default frame styles
			(iStyle) | 0,   // default frame styles
			NULL, &context));

	pFrame->InitialUpdateFrame(NULL, true);


	CView* pView = pFrame->GetActiveView();
	ASSERT(pView != NULL);
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CMachineStatusView)));
//	((CDocument*)pApp->m_pParamDoc)->AddView(pView);		// if not in createcontext
	m_pMachineStatusView = (CMachineStatusView*)pView;
}

LRESULT CMainFrame::OnUpdateParam(WPARAM wParam, LPARAM)
{
	LOGERROR("Got OnUpdateParam() in CMainFrame");		// should go to App
	if (m_pManualControlView != NULL)
		m_pManualControlView->OnUpdateParam(wParam);
	return 0;
}

LRESULT CMainFrame::OnSetActivePathView(WPARAM wParam, LPARAM lParam)
{
	m_pActivePathView = (CView*)wParam;
	if (lParam)
		*(CPathPositionDlg**)lParam = &m_wndPathPosDlg;
//	if (m_pActivePathView && m_wndPathPosDlg.m_hWnd == NULL)
//		m_wndPathPosDlg.Create();		// if not created
	m_wndPathPosDlg.SetView(m_pActivePathView);
	return 0;
}


void CMainFrame::OnMachineSetTool() 
{
	CSetToolDlg dlg;
	// load dimension values
	SMachineDimensions& md = g_Settings.MachDimensions;
	*(SMachineDimensionsData*)&dlg = *(SMachineDimensionsData*)&md;

	dlg.m_zAxisPos = 0;
	dlg.m_bGotzAxisPos = false;
	CParamDoc* pParamDoc = ((CCNCControlApp*)AfxGetApp())->m_pParamDoc;
	if (pParamDoc->m_pMachineState != NULL)
	{
		CVector vtPos;
		pParamDoc->m_pMachineState->GetPosHead(vtPos);
		dlg.m_zAxisPos = vtPos.z;
		dlg.m_bGotzAxisPos = true;
	}

	if (dlg.DoModal() == IDOK)
	{
		*(SMachineDimensionsData*)&md = *(SMachineDimensionsData*)&dlg;
		g_utPosConvert.SetZAxisHead2Tip(dlg.GetTip2ZAxisHeadDist());
		pParamDoc->m_pMachineState->SetZAxisPosTipAtBaseTop(dlg.GetAxisPosTipAtBaseTop());
		// save to disk now!
		PostMessage(WM_COMMAND, ID_FILE_SAVESETTINGS);
	}
}

void CMainFrame::OnOptionsSettings() 
{
	CGeneralOptionsDlg dlg;
	dlg.LoadData();
	if (dlg.DoModal() == IDOK)
	{
		dlg.SaveData();
		PostMessage(WM_COMMAND, ID_FILE_SAVESETTINGS);
	}
}




