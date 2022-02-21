// ReadNCIFile.h: interface for the CReadNCIFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_READNCIFILE_H__29145E42_53AC_11D7_86C3_DE77C5965826__INCLUDED_)
#define AFX_READNCIFILE_H__29145E42_53AC_11D7_86C3_DE77C5965826__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "PathDoc.h"


struct SDrillParameters
{
	int iCycleType;
	double dwell;
	double firstPeck;
	double subsequentPeck;
	double peckClearance;
	double retractAmount;
	double topOfStock;
	double shift;
};


class CReadNCIFile  
{
public:
	CReadNCIFile();
	virtual ~CReadNCIFile();

	void InterperateFile(CPathDoc* pPathDoc, CFile* pFile);
//	void InterperateFile(CPathDoc* pPathDoc, CArchive& ar);


protected:
	CPathDoc* m_pPathDoc;

	char* m_pLine;
	char m_strLine[256];
	char m_strArgs[256];
	int m_sizeLineBuffer;
	int m_iLineNum;

	enum			// Argument Type Flags
	{
		ATF_STRING = 0x01,
		ATF_INT = 0x02,
		ATF_FLOAT = 0x04,
	};
	enum			// Feed Rate types
	{
		FR_NONE = 0,
		FR_NORMAL,
		FR_RAPID,
	};

#define MAX_ARGS	40
	int argt[MAX_ARGS];
	int argi[MAX_ARGS];
	double argf[MAX_ARGS];
	char* args[MAX_ARGS];
	int m_numArgs;
	double m_FileResolution;		// resolution of file values

	// NCI file specific
	enum
	{
		CW = 1,
		CCW,
	};
	bool m_bGotAllArgs;
	bool m_bInPath;
	int m_nCurrentTool;
	int m_nCurrentFeed;
	double m_fNormFeedRate;
	CPathNode m_ptStartHomePos;
	bool m_bStartHomePosSet;
	NODEREF m_nrStartOfMove;
	double m_diaTool;
	bool m_bDiaToolSet;

	CString m_strComment;

	bool m_bRampXYArcs;
	bool m_bRampXYArcsSet;

	SDrillParameters m_DrillParams;


	// line interperating functions
	bool ReadNextLine(CArchive& ar);
	bool ReadNextLine(CFile& file);
	void GetLineArgs();
	bool InterpArg(char*& strArg, int numArg);
	char* GetStringToEndFromArg(int numArg);

	// NCI path functions
	void ReadCodeArgs(int iCode);

	void StartPath();
	void RestartPath();
	void EndPath();

	void AddPoint(CPathNode& nd);
	void ArcXY(int nPlane, int dir, CPathNode& ndFinal, CPathNode& ndCentre);
	void SetFeedRate(double feedRate);
	void SetRapidFeedRate();
	void MoveFlags(int flags);

	void SetToolName(char* strName);
	void SetToolCode(char* strCode);
	void SelectTool(int nTool);
	void SetToolDiameter(double diaTool);
	void SetComment(char* strComment);
	void SetNCProgramNumber(int progNum);
	void SetDrillParameters(int iDrillCycle, const double* pParams);
	void DrillHole100();
};

#endif // !defined(AFX_READNCIFILE_H__29145E42_53AC_11D7_86C3_DE77C5965826__INCLUDED_)
