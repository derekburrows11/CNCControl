// CNCControl.h : main common header file for the CNCCONTROL application
//

#if !defined(AFX_CNCCONTROL_H__4E7108A4_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
#define AFX_CNCCONTROL_H__4E7108A4_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"			// main symbols
#include "CommonResource.h"		// Common Shared user defined resources



#define NUM_AXIS	3		// was defined in "PathDataObjects.h", "Param.h"

#include <afxtempl.h>

class CLog
{
public:
	enum LOGTYPE
	{
		LOG_ERROR = 1,
		LOG_MESSAGE,
		LOG_EVENT,
	};

	CLog() { m_ErrorList.SetSize(0, 20); }
	void Log(enum CLog::LOGTYPE nType, char* szFile, int iLine, const char* szDesc, ...);


protected:
	struct SErrorInfo
	{
		char* szFile;
		int iLine;
		char* szDesc;
		char szTime[10];
	};
	CArray<SErrorInfo, SErrorInfo&> m_ErrorList;
};

extern CLog g_Log;


#define LOGERROR(sz)               g_Log.Log(CLog::LOG_ERROR, THIS_FILE, __LINE__, sz)
#define LOGERROR0(sz)              g_Log.Log(CLog::LOG_ERROR, THIS_FILE, __LINE__, sz)
#define LOGERROR1(sz, p1)          g_Log.Log(CLog::LOG_ERROR, THIS_FILE, __LINE__, sz, p1)
#define LOGERROR2(sz, p1, p2)      g_Log.Log(CLog::LOG_ERROR, THIS_FILE, __LINE__, sz, p1, p2)
#define LOGERROR3(sz, p1, p2, p3)  g_Log.Log(CLog::LOG_ERROR, THIS_FILE, __LINE__, sz, p1, p2, p3)

#define LOGMESSAGE(sz)               g_Log.Log(CLog::LOG_MESSAGE, THIS_FILE, __LINE__, sz)
#define LOGMESSAGE0(sz)              g_Log.Log(CLog::LOG_MESSAGE, THIS_FILE, __LINE__, sz)
#define LOGMESSAGE1(sz, p1)          g_Log.Log(CLog::LOG_MESSAGE, THIS_FILE, __LINE__, sz, p1)
#define LOGMESSAGE2(sz, p1, p2)      g_Log.Log(CLog::LOG_MESSAGE, THIS_FILE, __LINE__, sz, p1, p2)
#define LOGMESSAGE3(sz, p1, p2, p3)  g_Log.Log(CLog::LOG_MESSAGE, THIS_FILE, __LINE__, sz, p1, p2, p3)

#define LOGEVENT(sz)               g_Log.Log(CLog::LOG_EVENT, THIS_FILE, __LINE__, sz)
#define LOGEVENT0(sz)              g_Log.Log(CLog::LOG_EVENT, THIS_FILE, __LINE__, sz)
#define LOGEVENT1(sz, p1)          g_Log.Log(CLog::LOG_EVENT, THIS_FILE, __LINE__, sz, p1)
#define LOGEVENT2(sz, p1, p2)      g_Log.Log(CLog::LOG_EVENT, THIS_FILE, __LINE__, sz, p1, p2)
#define LOGEVENT3(sz, p1, p2, p3)  g_Log.Log(CLog::LOG_EVENT, THIS_FILE, __LINE__, sz, p1, p2, p3)




#endif	// !defined(AFX_CNCCONTROL_H__4E7108A4_678C_11D4_8C1E_C12B6D9C0F2F__INCLUDED_)
