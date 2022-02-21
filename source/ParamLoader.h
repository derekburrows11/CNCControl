// ParamLoader.h: interface for the CParamLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARAMLOADER_H__45D7A7C8_F75C_11D4_8C1E_E863D6A5072C__INCLUDED_)
#define AFX_PARAMLOADER_H__45D7A7C8_F75C_11D4_8C1E_E863D6A5072C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "StrUtils.h"
class CParamData;


#define MAXLENNAME	50
#define MAXLENDESCR	200
#define MAXLENITEM	50

class CParamLoader  
{
public:
	CParamLoader();
	virtual ~CParamLoader();

	void UseDataObj(CParamDoc* pPD);
	bool Load();

protected:
	void SelectSearchStage();
	void CheckForParamCode();
	void CheckForParamName();
	void CheckForParamStyle();
	void CheckForParamPrompt();
	void CheckForSectionLabel();
		
	void ClearParam(CParamEx& par);
	void ClearMultiWordParam(CParamEx& par);
	bool StoreParam(CParamEx& par);
//	void LogError(int iErrorNum);

// data members
protected:
	CParamDoc* m_pParamDoc;
	CString m_strParamFileName;

	CParamEx currPar;
	char szParName[MAXLENNAME], szParDesc[MAXLENDESCR];
	char szItem[MAXLENITEM];
	CInterpStr ParDescr;
	int m_iLineNum;
	int iSection;
	int iStage;
	int ErrorCode;
	int iInitIdx;
	int bNewLine;			// bool but also a count
	bool bBeginLine;

};

#endif // !defined(AFX_PARAMLOADER_H__45D7A7C8_F75C_11D4_8C1E_E863D6A5072C__INCLUDED_)
