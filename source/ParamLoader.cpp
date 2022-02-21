// ParamLoader.cpp: implementation of the CParamLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <fstream.h>

#include "CNCControl.h"
#include "ParamDoc.h"

#include "PktControl.h"	// only used for errors... change to ErrorLog.h


#include "ParamLoader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


enum section {
	SECT_NONE = 0,
	SECT_GEN,
	SECT_AXIS,
	SECT_EOF,
};

enum errorcode{
	EC_NULL = 0,
	EC_OK = 1,
	EC_FLUSHED,
	EC_SYNTAX,
	EC_COLONEXP,
	EC_NUMEXP,
	EC_STYLEEXP,

	EC_NOTUSED = 0x100,		// any above 'EC_NOTUSED' have not used extracted item
	EC_NOTCOMMAND,
	EC_NOTPARAMCODE,
	EC_NOTPARAMNAME,
	EC_NOTSTYLE,
	EC_NOTPROMPT,
};

enum stage {
	SG_PARAMCODE = 1,
	SG_COMMAND,
	SG_STYLE,
	SG_PROMPT,
	SG_FLUSH,

	SG_PARAMNAME = 10,
	SG_EOF,
};





//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParamLoader::CParamLoader()
{
	m_pParamDoc = NULL;
	m_strParamFileName = "CNC.par";		// default
}

CParamLoader::~CParamLoader()
{

}

//////////////////////////////////////////////////////////////////////

void CParamLoader::UseDataObj(CParamDoc* pPD)
{
	m_pParamDoc = pPD;
}

bool CParamLoader::Load()
{
	ASSERT(m_pParamDoc != NULL);

	ifstream paramFile(m_strParamFileName);
	if (!paramFile)
		return false;

	m_pParamDoc->m_nMaxAxisPar = -1;
	m_pParamDoc->m_nMaxGeneralPar = -1;
	m_iLineNum = 0;

	ParDescr.SetStrSize(MAXLENDESCR);
	iSection = SECT_NONE;
	iStage = SG_COMMAND;
	ErrorCode = EC_NULL;

	currPar.name = szParName;
	currPar.description = szParDesc;
	ClearParam(currPar);
	ClearMultiWordParam(currPar);

	while (iStage != SG_EOF)
	{
		if (ErrorCode >= EC_NOTUSED)
			ParDescr.SetIdx(iInitIdx);
		else		// Item was used
			bBeginLine = false;

		bNewLine = 0;
		while (ParDescr.AtEOnonWS())		// if at end of line or only WS left
		{
			if (!paramFile.getline(ParDescr.Buff(), ParDescr.BuffLen()))
			{
				iSection = SECT_EOF;
				break;
			}
			m_iLineNum++;
			ParDescr.Reset();
			bBeginLine = true;
			bNewLine++;			// bNewLine > 1 if blank lines encountered
		}
		iInitIdx = ParDescr.GetIdx();

		SelectSearchStage();

		ParDescr.EatWS();
		ErrorCode = EC_OK;
		switch (iStage)		// check line items depending on stage
		{
		case SG_PARAMCODE:		// check for parameter code 'p0x??'
			CheckForParamCode();
			break;
		case SG_PARAMNAME:		// check for parameter name, >= 2 chars
			CheckForParamName();
			break;
		case SG_COMMAND: 	// Check for section command
			CheckForSectionLabel();
			break;
		case SG_STYLE:		// check for style description '[...]'
			CheckForParamStyle();
			break;
		case SG_PROMPT:		// check for prompt string
			CheckForParamPrompt();
			break;
		case SG_FLUSH:
			ParDescr.Flush();
			ErrorCode = EC_FLUSHED;
			break;
		case SG_EOF:
			StoreParam(currPar);
			ErrorCode = EC_OK;
			break;
		default:
			ASSERT(0);
		}
	}	// while (iStage != SG_EOF)

	m_pParamDoc->SetAllParamsState(PSAB_UNKNOWN | PSAB_READY);
	m_pParamDoc->SetAllParamsValue(0);
	return true;
}

void CParamLoader::SelectSearchStage()
{
// control search stages, depending on current section
	switch (iSection)
	{
	case SECT_NONE:
		if (bNewLine)
			iStage = SG_COMMAND;		// do section command check
		else if (iStage == SG_COMMAND)	// always will if not new line in SECT_NONE
			iStage = SG_FLUSH;
		break;

	case SECT_GEN:		// In param section
	case SECT_AXIS:
		if (bNewLine)			// if on a new line start first check for parameter code
			iStage = SG_PARAMCODE;
		else if (bBeginLine)	// or if still at beginning of line
		{
			++iStage;
			if (iStage == SG_PROMPT && currPar.description[0]) // have prompt string already
				iStage = SG_FLUSH;
		}
		else		// not at beginning of line
					// if item recognised next stage is set at function
			if (ErrorCode != EC_OK)		// if item not recognised
					iStage = SG_FLUSH;
		break;

	case SECT_EOF:		// End of file
		iStage = SG_EOF;
		break;
	}
}

void CParamLoader::CheckForParamCode()
{
// check for parameter code 'p0x??' or just 'p ' then code inc's from last
	if (ParDescr.Extract('p'))
	{
		bool bSpaceAfterP = ParDescr.EatWS() != 0;
		if (ParDescr.IsItemNum())
		{
			StoreParam(currPar);			// finished reading previous parameter
			currPar.code = ParDescr.GetNumValue();
			currPar.iFileSection = iSection;			// remember section at start of param descr
			iStage = SG_PARAMNAME;
		}
		else if (bSpaceAfterP)			// no number, just inc prev
		{
			StoreParam(currPar);			// finished reading previous parameter
			currPar.code++;
			currPar.iFileSection = iSection;			// remember section at start of param descr
			iStage = SG_PARAMNAME;
		}
		else
			ErrorCode = EC_NOTPARAMCODE;
	}
	else
		ErrorCode = EC_NOTPARAMCODE;
}

void CParamLoader::CheckForParamName()
{
// check for parameter name, >= 2 chars
	if (ParDescr.GetAlphaNumItem(currPar.name, MAXLENNAME) >= 2)
		iStage = SG_STYLE;
	else
		ErrorCode = EC_NOTPARAMNAME;
}


void CParamLoader::CheckForParamStyle()
{
// check for style description '[...]'
	if (!ParDescr.Extract('['))
	{
		ErrorCode = EC_NOTSTYLE;		// not a '['
		return;
	}
	int style = 0;
	ErrorCode = EC_NULL;
	// Check style description codes
	while (!ErrorCode)
	{
		ParDescr.EatWS();
		if (ParDescr.GetAlphaNumItem(szItem, MAXLENITEM))
		{
			if (isdigit(szItem[0]))		// starts with num
			{
				int val, numChar = -1;
				int res = sscanf(szItem, "%i%n", &val, &numChar);
				if (!strcmp(szItem + numChar, "byte"))
				{
					if (currPar.iTotalBytes == -1)
						currPar.iTotalBytes = val;
					else if (currPar.iTotalBytes != val)
						LOGERROR2("Parameter bytes redefined in param file %s line %i -", m_strParamFileName, m_iLineNum);
				}
			}
			else if (!strcmp(szItem, "ro"))
				style |= PSY_READONLY;
			else if (!strcmp(szItem, "unsigned") || !strcmp(szItem, "us"))
				style |= PSY_UNSIGNED;
			else if (!strcmp(szItem, "signed"))
				style &= ~PSY_UNSIGNED;
			else if (!strcmp(szItem, "dec"))
				style |= PSY_DEC;
			else if (!strcmp(szItem, "hex"))
				style |= PSY_HEX;
			else if (!strcmp(szItem, "bin"))
				style |= PSY_BIN;
			else if (!strcmp(szItem, "max") || !strcmp(szItem, "min"))
			{
				if (ParDescr.Extract(':'))
					if (ParDescr.IsItemNum())
						if (szItem[2] == 'x')
						{	currPar.maxValue = ParDescr.GetNumValue();
							style |= PSY_HASMAX; }
						else
						{	currPar.minValue = ParDescr.GetNumValue();
							style |= PSY_HASMIN; }
					else
						ErrorCode = EC_NOTSTYLE;	//EC_NUMEXP;
				else
					ErrorCode = EC_NOTSTYLE;		//EC_COLONEXP;
			}
			else		// szItem is not a recognised keyword
				ErrorCode = EC_NOTSTYLE;
		}
					// not AlphaNum or WS therefor punctuation char
		else if (ParDescr.Extract(']'))
				ErrorCode = EC_OK;
		else		// not a recognise punct char, or EOL 
				ErrorCode = EC_NOTSTYLE;
	}	// while (!ErrorCode)
	if (ErrorCode == EC_OK)
		currPar.style = style;
}

void CParamLoader::CheckForParamPrompt()
{
// check for prompt string
	if (ParDescr)
	{
		strcpy(currPar.description, ParDescr.GetIdxPtr());
		iStage = SG_FLUSH;
	}
	else
		ErrorCode = EC_NOTPROMPT;
}

void CParamLoader::CheckForSectionLabel()
{
// Check for section label
	if (!ParDescr.Extract('['))
	{
		ErrorCode = EC_NOTCOMMAND;
		return;
	}
	ErrorCode = EC_OK;
	ParDescr.EatWS();
	ParDescr.GetAlphaNumItem(szItem, MAXLENITEM);
	if (!stricmp(szItem, "GENERALPARAM"))
		iSection = SECT_GEN;
	else if (!stricmp(szItem, "AXISPARAM"))
		iSection = SECT_AXIS;
	else if (!stricmp(szItem, "ENDPARAM"))
		iSection = SECT_NONE;
	else
		ErrorCode = EC_NOTCOMMAND;
	if (ErrorCode == EC_OK)
	{
		StoreParam(currPar);
		ClearMultiWordParam(currPar);		// in case we're within a multi word param
		ParDescr.EatWS();
		if (!ParDescr.Extract(']'))
			ErrorCode = EC_SYNTAX;
		iStage = SG_PARAMCODE;
	}
}

void CParamLoader::ClearParam(CParamEx& par)
{
//	par.code = -1;
	par.name[0] = 0;
	par.description[0] = 0;
}

void CParamLoader::ClearMultiWordParam(CParamEx& par)
{
	par.iTotalBytes = -1;	// for default
	par.iWordNum = 0;
	par.style = PSY_NOFORMAT;
}

bool CParamLoader::StoreParam(CParamEx& par)
//	Axis/General and Individual Axis flags are in style member of CParam
{
	if (par.code == -1 || par.name[0] == 0)
		return false;

	// Set remainder of style bits, or default
	if (par.iTotalBytes == -1)
		par.iTotalBytes = 2;					// default
	int bytes = par.iTotalBytes;
	bytes <<= PSY_TOTALBYTESSHIFT;
	bytes &= PSY_TOTALBYTESMASK;
	par.style &= ~PSY_TOTALBYTESMASK;
	par.style |= bytes;

	int wordNum = par.iWordNum;
	wordNum <<= PSY_WORDNUMSHIFT;
	wordNum &= PSY_WORDNUMMASK;
	par.style &= ~PSY_WORDNUMMASK;
	par.style |= wordNum;


	CParam* arGeneralPar = m_pParamDoc->m_GeneralPar;
	CParam (*arAxisPar)[NUM_AXIS] = m_pParamDoc->m_AxisPar;

	if (par.iFileSection == SECT_AXIS)	// Axis Parameter
	{
		if (arAxisPar[par.code][0].name)	// if already been set
			return false;
	// store parameter onto array and assign new memory for strings
		char* szalloc = new char[strlen(par.name) + strlen(par.description) + 2];
		char* szallocDescr = szalloc + strlen(par.name) + 1;
		strcpy(szalloc, par.name);
		strcpy(szallocDescr, par.description);

		for (int i = 0; i < m_pParamDoc->m_nNumAxis; i++)
		{
			CParam* pArrPar = &arAxisPar[par.code][i];
			if (par.style & PSY_UNIQUEAXIS)
				*pArrPar = (&par)[i];
			else
				*pArrPar = par;
			pArrPar->code = par.code;	// these should all be the same
			pArrPar->name = szalloc;
			pArrPar->description = szallocDescr;
		}

	// set maximum parameter number
		if (par.code > m_pParamDoc->m_nMaxAxisPar)
			m_pParamDoc->m_nMaxAxisPar = par.code;
	}
	else if (par.iFileSection == SECT_GEN)	// General Parameter
	{
		CParam* pArrPar = &arGeneralPar[par.code];
		if (pArrPar->name)	// if already been set
			return false;
	// store parameter onto array and assign new memory for strings
		*pArrPar = par;
		char* szalloc = new char[strlen(par.name) + strlen(par.description) + 2];
		pArrPar->name = szalloc;
		pArrPar->description = szalloc + strlen(par.name) + 1;
		strcpy(pArrPar->name, par.name);
		strcpy(pArrPar->description, par.description);

	// set maximum parameter number
		if (par.code > m_pParamDoc->m_nMaxGeneralPar)
			m_pParamDoc->m_nMaxGeneralPar = par.code;
	}


	if ((2 * (par.iWordNum + 1)) < par.iTotalBytes)		// check if another word if needed for bytes
		par.iWordNum++;			// next param should be next word of current one
	else
		ClearMultiWordParam(par);	// not next word of a multi word parameter
	ClearParam(par);
	return true;
}


