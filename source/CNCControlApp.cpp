// CNCControl.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CNCControl.h"

#include "CNCControlApp.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "CNCControlDoc.h"
#include "CNCControlView.h"
#include "ParamDoc.h"
#include "ParamView.h"
#include "MachineState.h"

#include "ParamLoader.h"
#include "PathDoc.h"
#include "PathView.h"
#include "PathNCIDoc.h"
#include "PathSpeedDoc.h"
#include "PathSpeedView.h"
#include "PathAnimateView.h"
#include "LogDoc.h"
#include "Settings.h"

#include "DiagnoseDlg.h"
#include "SendBytesDlg.h"
//#include "CommsDataDlg.h"

#include <malloc.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif





/////////////////////////////////////////////////////////////////////////////
// CCNCControlApp

IMPLEMENT_DYNAMIC(CCNCControlApp, CWinApp)

BEGIN_MESSAGE_MAP(CCNCControlApp, CWinApp)
	//{{AFX_MSG_MAP(CCNCControlApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_MACHINE_OPENCOMMUNICATIONS, OnMachineOpenComms)
	ON_COMMAND(ID_MACHINE_OPENCLOSECOMMUNICATIONS, OnMachineOpenCloseComms)
	ON_UPDATE_COMMAND_UI(ID_MACHINE_OPENCLOSECOMMUNICATIONS, OnUpdateMachineOpenCloseComms)
	ON_COMMAND(ID_OPTIONS_PORTSELECT, OnOptionsPortSelect)
	ON_COMMAND(ID_OPTIONS_PORTCONFIG, OnOptionsPortConfig)
	ON_COMMAND(ID_VIEW_PARAMETERS_AXIS, OnViewParamAxis)
	ON_COMMAND(ID_VIEW_PARAMETERS_GENERAL, OnViewParamGeneral)
	ON_COMMAND(ID_VIEW_PARAMETERS_ALL, OnViewParamAll)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_MACHINE_SIMULATERESPONSE, OnMachineSimulateResponse)
	ON_UPDATE_COMMAND_UI(ID_MACHINE_SIMULATERESPONSE, OnUpdateMachineSimulateResponse)
//	ON_COMMAND(ID_PATH_SENDSTART, OnPathSendStart)
//	ON_UPDATE_COMMAND_UI(ID_PATH_SENDSTART, OnUpdatePathSendStart)
	ON_COMMAND(ID_PATH_SENDSTOP, OnPathSendStop)
	ON_UPDATE_COMMAND_UI(ID_PATH_SENDSTOP, OnUpdatePathSendStop)
	ON_COMMAND(ID_PATH_SENDRESET, OnPathSendReset)
	ON_UPDATE_COMMAND_UI(ID_PATH_SENDRESET, OnUpdatePathSendReset)
	ON_COMMAND(ID_MACHINE_SHOWCOMMSDATA, OnMachineShowCommsData)
	ON_UPDATE_COMMAND_UI(ID_MACHINE_SHOWCOMMSDATA, OnUpdateMachineShowCommsData)
	ON_COMMAND(ID_MACHINE_LOGCOMMSDATA, OnMachineLogCommsData)
	ON_UPDATE_COMMAND_UI(ID_MACHINE_LOGCOMMSDATA, OnUpdateMachineLogCommsData)
	ON_COMMAND(ID_MACHINE_SENDBYTES, OnMachineSendBytes)
	ON_COMMAND(ID_SENDPKT, OnSendPacket)
	ON_COMMAND(ID_SENDPKTSTEP, OnSendPktStep)
	ON_UPDATE_COMMAND_UI(ID_SENDPKTSTEP, OnUpdateSendPktStep)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_VIEW_LOG, OnViewLog)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOG, OnUpdateViewLog)
	ON_COMMAND(ID_FILE_SAVESETTINGS, SaveConfigSettings)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	// User Messages
	ON_THREAD_MESSAGE(WMU_COMMNOTIFY, OnCommNotify)
	ON_THREAD_MESSAGE(WMU_UPDATEPARAM, OnUpdateParam)
	ON_THREAD_MESSAGE(WMU_MEMVALUE, OnMemValue)
	ON_THREAD_MESSAGE(WMU_REGVALUE, OnRegValue)

	ON_THREAD_MESSAGE(WMU_LOGEVENT, OnLogEvent)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCNCControlApp construction

CCNCControlApp::CCNCControlApp()
{
	// Place all significant initialization in InitInstance
	m_pParamDoc = NULL;
	m_pLogDoc = NULL;

	m_pParamDocTemplate = NULL;
	m_pPathDocTemplate = NULL;
	m_pPathSpeedDocTemplate = NULL;
	m_pLogDocTemplate = NULL;

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCNCControlApp object

CCNCControlApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCNCControlApp initialization

BOOL CCNCControlApp::InitInstance()
{
//	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

// 3d controls by default with win95+
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
//	InitCommonControls();

	// change location of ini file from windows directory to current
	 _msize((void*)m_pszProfileName);
	free((void*)m_pszProfileName);
	m_pszProfileName = (char*)malloc(MAX_PATH);
	ASSERT(m_pszProfileName != NULL);
	GetCurrentDirectory(MAX_PATH, (char*)m_pszProfileName);
	strcat((char*)m_pszProfileName, "\\Config\\");
	strcat((char*)m_pszProfileName, m_pszAppName);
	strcat((char*)m_pszProfileName, ".ini");

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
//	SetRegistryKey(_T("Local AppWizCard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	char szDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szDir);

	LoadConfigSettings();		// load config settings from .cfg file

	RegDocTemplates();
	
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;



	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();


	// floating point settings
//	_controlfp(,);

	// Initialise CNC Parameter Data
	m_pParamDoc = new CParamDoc;
 	m_pParamDoc->m_bAutoDelete = false;		// delete in ExitInstance()
	m_pParamDoc->SetCNCComm(&m_CNCComms);

	CParamLoader paramLoader;
	paramLoader.UseDataObj(m_pParamDoc);
	paramLoader.Load();					// Loads from default file


//	OnViewParamAll();				// Set initial views
//	OnMachineOpenComms();

	m_fontText.CreatePointFont(100, "courier");

	m_bSendPktStep = false;

	GetThreadPriority();
//	SetThreadPriority(THREAD_PRIORITY_HIGHEST);
//	SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);


	// after config settings loaded
	m_CNCComms.Init();
	g_utPosConvert.Init();

	// init m_ControllerPath
	m_ControllerPath.SetCNCComms(&m_CNCComms);
	m_ControllerPath.SetParamDoc(m_pParamDoc);
	m_ControllerPath.Init();


	return true;
}

int CCNCControlApp::ExitInstance() 
{
	SaveConfigSettings();

	if (m_CNCComms.IsCommOpen())
		m_CNCComms.CloseComms();

	delete m_pParamDoc;		// m_bAutoDeleted by default already if used in a view
	if (m_pLogDoc != NULL)
	{

//		m_pLogDoc->SetModifiedFlag(false);	// so don't get 'destroying unsaved doc'
		delete m_pLogDoc;
	}
	return CWinApp::ExitInstance();
}

void CCNCControlApp::LoadConfigSettings() 
{
	// set file name
	char szFile[MAX_PATH];
	strcpy(szFile, m_pszProfileName);
	char* pDot = strrchr(szFile, '.');
	if (pDot != NULL)
		strcpy(pDot, ".cfg");
	g_Settings.SetFileName(szFile);

	g_Settings.Load();		// load config settings from .cfg file
	m_bSimulateControllerResponse = g_Settings.MachComms.bSimulateResponse;
}

void CCNCControlApp::SaveConfigSettings() 
{
	char szDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szDir);

	g_Settings.MachComms.iCommPort = m_CNCComms.GetCommPort();
	g_Settings.MachComms.bSimulateResponse = m_bSimulateControllerResponse;
	g_Settings.Store();		// store config settings to .cfg file
}

void CCNCControlApp::RegDocTemplates()
{
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;

	pDocTemplate = new CMultiDocTemplate(
		IDR_PARAMVIEW,
		RUNTIME_CLASS(CParamDoc),
		RUNTIME_CLASS(CChildFrame),
		RUNTIME_CLASS(CParamView));
	AddDocTemplate(pDocTemplate);
	m_pParamDocTemplate = pDocTemplate;
	
	pDocTemplate = new CMultiDocTemplate(
		IDR_PATHNCI,
		RUNTIME_CLASS(CPathNCIDoc),
		RUNTIME_CLASS(CMDIChildWnd),
		RUNTIME_CLASS(CPathAnimateView));
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CMultiDocTemplate(
		IDR_PATH,
		RUNTIME_CLASS(CPathDoc),
		RUNTIME_CLASS(CMDIChildWnd),
		RUNTIME_CLASS(CPathAnimateView));
	AddDocTemplate(pDocTemplate);
	m_pPathDocTemplate = pDocTemplate;

	pDocTemplate = new CMultiDocTemplate(
		IDR_PATHSPEED,
		RUNTIME_CLASS(CPathSpeedDoc),
		RUNTIME_CLASS(CMDIChildWnd),
		RUNTIME_CLASS(CPathSpeedView));
	AddDocTemplate(pDocTemplate);
	m_pPathSpeedDocTemplate = pDocTemplate;

/*	pDocTemplate = new CMultiDocTemplate(
		IDR_PATHANIMATE,
		RUNTIME_CLASS(CPathDoc),
		RUNTIME_CLASS(CMDIChildWnd),
		RUNTIME_CLASS(CPathAnimateView));
	AddDocTemplate(pDocTemplate);
	m_pPathAnimateDocTemplate = pDocTemplate;
*/

	pDocTemplate = new CMultiDocTemplate(
		IDR_LOGVIEW,
		RUNTIME_CLASS(CLogDoc),
		RUNTIME_CLASS(CChildFrame),
		RUNTIME_CLASS(CEditView));
	AddDocTemplate(pDocTemplate);
	m_pLogDocTemplate = pDocTemplate;

}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CCNCControlApp commands
/////////////////////////////////////////////////////////////////////////////

// App command to run the About dialog
void CCNCControlApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
//	CAboutDlg* pAboutDlg = new CAboutDlg;
//	pAboutDlg->Create(IDD_ABOUTBOX);
}

void CCNCControlApp::OnViewParamGeneral() 
{
	CreateParamView(CParamView::VT_GENPARAM);			// general type view
}

void CCNCControlApp::OnViewParamAxis() 
{
	CreateParamView(CParamView::VT_AXISPARAM);		// axis type view
}

void CCNCControlApp::OnViewParamAll() 
{
	CreateParamView(CParamView::VT_GENPARAM);			// general type view
	CreateParamView(CParamView::VT_AXISPARAM);		// axis type view
}

void CCNCControlApp::CreateParamView(int nViewType)
{
	ASSERT_VALID(m_pParamDocTemplate);
	CFrameWnd* pFrame = m_pParamDocTemplate->CreateNewFrame(m_pParamDoc, NULL);
	char* szLabel = (nViewType == CParamView::VT_GENPARAM) ? "ParamViewGen" : "ParamViewAxis";
	((CChildFrame*)pFrame)->SetStorageLabel(szLabel);

	// find new CParamView object to set view type
	CWnd* pWnd = pFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST);
	ASSERT(pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CParamView)));
	((CParamView*)pWnd)->SetViewType(nViewType);

	pFrame->InitialUpdateFrame(NULL, true);


//	CFrameWnd* pFrame = new CMDIChildWnd;
//	pFrame->LoadFrame(IDR_PARAMVIEW);
}

void CCNCControlApp::OnViewLog() 
{
	if (m_pLogDoc == NULL)
	{
		ASSERT_VALID(m_pLogDocTemplate);
		// first create a doc
		m_pLogDoc = (CLogDoc*)m_pLogDocTemplate->CreateNewDocument();
		ASSERT(m_pLogDoc->IsKindOf(RUNTIME_CLASS(CLogDoc)));
		VERIFY(m_pLogDoc->OnNewDocument());
		m_pLogDoc->m_bAutoDelete = false;		// delete in ExitInstance()
		// then frame
		CFrameWnd* pFrame = m_pLogDocTemplate->CreateNewFrame(m_pLogDoc, NULL);
		((CChildFrame*)pFrame)->SetStorageLabel("EventLog");
		pFrame->SetWindowText("Event Log");
		pFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST)->SetFont(&m_fontText);
		pFrame->InitialUpdateFrame(NULL, true);
	}
	else
	{
		POSITION pos = m_pLogDoc->GetFirstViewPosition();
		if (pos)
		{
			CView* pView = m_pLogDoc->GetNextView(pos);
			CWnd* pFrame = pView->GetParent();
			pFrame->ShowWindow(pFrame->IsWindowVisible() ? SW_HIDE : SW_SHOW);
		}
	}
}

void CCNCControlApp::OnUpdateViewLog(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(false);
	if (m_pLogDoc != NULL)
	{
		POSITION pos = m_pLogDoc->GetFirstViewPosition();
		if (pos)
		{
			CWnd* pFrame = m_pLogDoc->GetNextView(pos)->GetParent();
			pCmdUI->SetCheck(pFrame->IsWindowVisible());
		}
	}
}

void CCNCControlApp::OnLogEvent(WPARAM str, LPARAM)
{	
	if (m_pLogDoc == NULL)		// start if not open
		OnViewLog();

	CString* pStr = (CString*)str;
	if (m_pLogDoc != NULL)
		m_pLogDoc->LogEvent((const char*)*pStr);
	delete pStr;
}

void CCNCControlApp::OnUpdateMachineOpenCloseComms(CCmdUI* pCmdUI) 
{
	if (m_CNCComms.IsCommOpen())
		pCmdUI->SetText("&Close Communications");	
	else
		pCmdUI->SetText("&Open Communications");	
}

void CCNCControlApp::OnMachineOpenCloseComms() 
{
	if (m_CNCComms.IsCommOpen())
		OnMachineCloseComms();	
	else
		OnMachineOpenComms();
}

void CCNCControlApp::OnMachineOpenComms() 
{
	if (!m_CNCComms.OpenComms())			// Opens the serial port
	{
		char szMsg[40] = "Cannot open ";
		strcat(szMsg, m_CNCComms.GetCommName());
		AfxMessageBox(szMsg);
		return;
	}

//	GetMainWnd()->PostMessage(WM_COMMAND, ID_MACHINE_SHOWMACHINESTATUS);	// Activate Machine Status window
	GetMainWnd()->PostMessage(WM_COMMAND, ID_MACHINE_MANUALCONT);	// Activate manual controller - from main window!

	SetSimulateResponse(m_bSimulateControllerResponse);
	// send CCNCControlApp managed data

	OnViewParamAll();
}

void CCNCControlApp::OnMachineCloseComms() 
{
	m_CNCComms.CloseComms();	
}




void CCNCControlApp::OnOptionsPortSelect() 
{
	m_CNCComms.SelectPort();
}

void CCNCControlApp::OnOptionsPortConfig() 
{
	m_CNCComms.ConfigPort();
}



void CCNCControlApp::OnCommNotify(WPARAM wType, LPARAM lData)
{
//	static int iCount = 0;
//	iCount++;
//	TRACE1("Notified User thread in App %i times\n", iCount);
	if (!m_CNCComms.IsCommOpen())		// ignore any more messages if monitor closed down
		return;
	switch (wType)
	{
	case CN_UPDATE_TXLOG:
		m_CNCComms.UpdateTxLog(lData);		// lData is num of bytes
		break;
	case CN_UPDATE_RXLOG:
		m_CNCComms.UpdateRxLog(lData);			// lData is num of bytes
		break;
	case CN_UPDATE_MACHINEBUFFERINFO:
//		m_ControllerPath.UpdateMachineBufferStatus(*(SContBufferSpan*)lData);
		m_pParamDoc->UpdateMachineBufferStatus((SContBufferSpan*)lData);
		break;
	case CN_UPDATE_MACHINESTATEINFO:
//		m_ControllerPath.UpdateMachineStateStatus(*(SMachineState*)lData);
		m_pParamDoc->UpdateMachineStateStatus((CMachineState*)lData);
		break;
	case CN_PATHBUFFER_LOW:
		if (lData == 1)		// from timeout!
			LOGEVENT("Got Buffer low message from Monitor time out!");
		m_ControllerPath.SendMorePath();
		break;
	case CN_SETSIMULATESTATE:
		m_bSimulateControllerResponse = (lData != 0);
		break;
	case CN_PATHFINISHED:
		m_ControllerPath.OnPathFinished();
		break;
	default:
		LOGERROR("Unhandled notification in OnCommNotify()");
	}
}

void CCNCControlApp::OnUpdateParam(WPARAM axpar, LPARAM)
{
	m_pParamDoc->UpdateAllViews(NULL, HINT_AXPAR | axpar);
//	GetMainWnd()->SendMessage(WMU_UPDATEPARAM, axpar);
}

void CCNCControlApp::OnMemValue(WPARAM iAddr, LPARAM lValueBytes)
{
	// function is responsible for notifying all appropriate objects
	ASSERT(GetMainWnd()->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	CDiagnoseDlg* pDiagDlg = ((CMainFrame*)GetMainWnd())->m_pDiagnoseDlg;
	if (pDiagDlg)
		pDiagDlg->RxMemValue(iAddr, LOWORD(lValueBytes), HIWORD(lValueBytes));
}

void CCNCControlApp::OnRegValue(WPARAM iReg, LPARAM lValueBytes)
{
	// function is responsible for notifying all appropriate objects
	ASSERT(GetMainWnd()->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	CDiagnoseDlg* pDiagDlg = ((CMainFrame*)GetMainWnd())->m_pDiagnoseDlg;
	if (pDiagDlg)
		pDiagDlg->RxRegValue(iReg, LOWORD(lValueBytes), HIWORD(lValueBytes));
}


void CCNCControlApp::OnFileOpen() 
{
//	char dir[100];
//	GetCurrentDirectory(100, dir);
//	SetCurrentDirectory("c:\\derek\\vc\\cnccontrol\\"); // is cnccontrol already
//	GetCurrentDirectory(100, dir);

	char szDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szDir);
	// just this
	CWinApp::OnFileOpen();
	GetCurrentDirectory(MAX_PATH, szDir);

	// or open a particular one
//	CWinApp::DoPromptFileName();
//	VERIFY(OpenDocumentFile("Test Path1.pth"));
//	VERIFY(OpenDocumentFile("SOLID MACHINING - MM.NCI"));
}

LRESULT CCNCControlApp::ProcessWndProcException(CException* e, const MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CWinApp::ProcessWndProcException(e, pMsg);
}

void CCNCControlApp::SetSimulateResponse(bool bSim)
{
	m_bSimulateControllerResponse = bSim;
	m_CNCComms.SendMonitorRequest(REQ_SetSimulateResponse, bSim);
}

void CCNCControlApp::OnMachineSimulateResponse()
{
	SetSimulateResponse(!m_bSimulateControllerResponse);
}

void CCNCControlApp::OnUpdateMachineSimulateResponse(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bSimulateControllerResponse);
//	pCmdUI->Enable(m_CNCComms.IsCommOpen());
}


void CCNCControlApp::OnPathSendReset() 
{
	m_ControllerPath.ResetPath();
}

void CCNCControlApp::OnPathSendStart()
{
	m_ControllerPath.SendPath();
}

void CCNCControlApp::OnPathSendStop()
{
	m_ControllerPath.StopSendingPath();
}

void CCNCControlApp::OnSendPacket()
{
	m_bSendPktStep = true;
	m_CNCComms.SendMonitorMessage(WMU_SENDPKTSTEP, 1);
}

void CCNCControlApp::OnSendPktStep()
{
	m_bSendPktStep = !m_bSendPktStep;
	if (m_bSendPktStep)
		m_CNCComms.SendMonitorMessage(WMU_SENDPKTSTEP);
	else
		m_CNCComms.SendMonitorMessage(WMU_SENDPKTCONT);
}

void CCNCControlApp::OnUpdateSendPktStep(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bSendPktStep);
}

void CCNCControlApp::OnUpdatePathSendStart(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ControllerPath.GotPath() && !m_ControllerPath.IsSendingPath());
}

void CCNCControlApp::OnUpdatePathSendStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(true);
//	pCmdUI->Enable(m_ControllerPath.IsSendingPath());
}

void CCNCControlApp::OnUpdatePathSendReset(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ControllerPath.GetPathDoc() != NULL);
}

void CCNCControlApp::OnMachineLogCommsData() 
{
	m_CNCComms.LogCommsData(!m_CNCComms.IsLoggingCommsData());	
}

void CCNCControlApp::OnUpdateMachineLogCommsData(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_CNCComms.IsLoggingCommsData());
}

void CCNCControlApp::OnMachineShowCommsData() 
{
	bool bShow = !m_CNCComms.IsDataLogVisible();
	m_CNCComms.ShowDataLog(bShow);
	m_CNCComms.LogCommsData(bShow);	// on when on, off when off
}

void CCNCControlApp::OnUpdateMachineShowCommsData(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_CNCComms.IsDataLogVisible());
}


void CCNCControlApp::OnMachineSendBytes() 
{
	CSendBytesDlg dlg;
	dlg.m_pCNCComms = &m_CNCComms;

	dlg.DoModal();
}



CLog g_Log;


void CLog::Log(enum CLog::LOGTYPE nType, char* szFile, int iLine, const char* szDesc, ...)
{
	char* szFileName = strrchr(szFile, '\\');		// just file name
	if (szFileName == NULL)
		szFileName = szFile;
	else
		szFileName++;

	char szBuffer[512];
	char* pBuff = szBuffer;

	// insert type name
	char* szType = NULL;
	switch (nType)
	{
	case LOG_ERROR: szType = "ERROR"; break;
	case LOG_MESSAGE: szType = "MSG"; break;
	case LOG_EVENT: szType = NULL; break;
	}
	if (szType != NULL)
		pBuff += sprintf(pBuff, "%s", szType);

	// insert time
	char szTime[10];
	_strtime(szTime);
	pBuff += sprintf(pBuff, "(%s) - ", szTime);

	// insert description
	va_list args;
	va_start(args, szDesc);
	int nBuf = _vsnprintf(pBuff, sizeof(szBuffer)+szBuffer-pBuff, szDesc, args);
	if (nBuf >= 0)
		pBuff += nBuf;
	va_end(args);

	// insert file and line
	pBuff += sprintf(pBuff, " in %s line %i\r\n", szFileName, iLine);

	CString* pStr = new CString(szBuffer);		// will be deleted by receiving thread
	if (!AfxGetApp()->PostThreadMessage(WMU_LOGEVENT, (WPARAM)pStr, 0))
		delete pStr;

	if (nType == LOG_ERROR)
		TRACE(szBuffer);

	if (nType == LOG_ERROR)
	{
		SErrorInfo info;
		info.szFile = szFileName;
		info.iLine = iLine;
		info.szDesc = szBuffer;
		_strtime(info.szTime);
		m_ErrorList.Add(info);
	}
}


void CCNCControlApp::OnFileNew() 
{
	CWinApp::OnFileNew();
}


