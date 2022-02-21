
#include "stdafx.h"
#include "StrUtils.h"


#include "Store.h"

#include "CNCControl.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// Helpers for saving/restoring window state

static char szSection[] = "WindowPlacements";
static char szFormat[] = "%u %u %d %d %d %d %d %d %d %d";

// read a window placement from settings section of app's ini file
bool ReadWindowPlacement(WINDOWPLACEMENT* pWP, LPCSTR szEntry)
{
	if (szEntry == NULL)
		return false;
	CString strBuffer = AfxGetApp()->GetProfileString(szSection, szEntry);
	if (strBuffer.IsEmpty())
		return false;
	WINDOWPLACEMENT& wp = *pWP;
	int nRead = sscanf(strBuffer, szFormat,
		&wp.flags, &wp.showCmd,
		&wp.ptMinPosition.x, &wp.ptMinPosition.y,
		&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
		&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
		&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);
	if (nRead != 10)
		return false;
	return true;
}

// write a window placement to settings section of app's ini file
bool WriteWindowPlacement(WINDOWPLACEMENT* pWP, LPCSTR szEntry)
{
	if (szEntry == NULL)
		return false;
	char szBuffer[7 * 10];		// 10 values, max 6 digits plus seperators
	wsprintf(szBuffer, szFormat,
		pWP->flags, pWP->showCmd,
		pWP->ptMinPosition.x, pWP->ptMinPosition.y,
		pWP->ptMaxPosition.x, pWP->ptMaxPosition.y,
		pWP->rcNormalPosition.left, pWP->rcNormalPosition.top,
		pWP->rcNormalPosition.right, pWP->rcNormalPosition.bottom);
	AfxGetApp()->WriteProfileString(szSection, szEntry, szBuffer);
	return true;
}


/////////////////////////////////////////////////
// class CStore
/////////////////////////////////////////////////

CStore::CStore()
{
	m_pIOS = NULL;
//	m_pAr = NULL;
	m_strSection = NULL;
	m_bFoundSection = false;
	m_bDataValid = false;
}

void CStore::Serialize(CArchive& ar)
{
//	m_pAr = &ar;

	CFile* pFile = ar.GetFile();
	fstream fs;
	fs.attach(pFile->m_hFile);
	ASSERT(fs.good());
	ASSERT(!fs.bad());
	Serialize(fs, ar.IsStoring() != 0);
}

void CStore::Serialize(fstream& ios, bool bStore)
{
	m_bIsStoring = bStore;
	m_pIOS = &ios;
	SetSectionName();
	const char* strSection = m_strSection;
	ASSERT(strSection != NULL);

	if (m_bIsStoring)
	{
		if (ios.fail())
			return;
		ios << "[" << strSection << "]" << endl;
		SerializeBody();
		SerializeSectionEnd();
		return;
	}

	VERIFY(ios.setmode(filebuf::binary) > 0);		// text mode has trouble with tellg()
	Defaults();
	streampos posStartCheck = ios.tellg();
	bool bLooped = false;
	if (ios.fail())
		bLooped = true;		// make if finish loop
	m_bFoundSection = false;
	m_posStartSection = -1;
	ASSERT(strlen(strSection) < sizeof(m_arLine));
	for (;;)
	{
//		streampos posBefore = ios.tellg();
		ios.getline(m_arLine, sizeof(m_arLine));
//		streampos posAfter = ios.tellg();
//		int posCng = posAfter - posBefore;
		int len = strlen(m_arLine);
		if (m_arLine[len-1] == '\r')
			m_arLine[len-1] = 0;
//		ASSERT(posCng == len+1 || posCng == 0);

		char* pStr = m_arLine;
		extractWS(pStr);
		if (extractCh(pStr, '['))
		{
			extractWS(pStr);
			if (strextract(pStr, strSection))
			{
				extractWS(pStr);
				if (extractCh(pStr, ']'))
				{
					m_posStartSection = ios.tellg();		// at start of next line
					m_bFoundSection = true;
					break;
				}
			}
		}
		if (ios.eof())
		{
			//ios.seekg(0, ios::beg);
			ios.seekg(0);
			ios.clear();
			bLooped = true;
		}
//			if (!ios.good())		// is if eof
//				return;
		if (bLooped && ios.tellg() >= posStartCheck)
			break;		// section not found
	}	// for (;;)

	if (m_bFoundSection)
		SerializeBody();
	else
		LOGERROR1("Config section not found '%s'", strSection);

	SerializeSectionEnd();
	CalculateAfterLoad();		// do any calculations of values
	m_bDataValid = true;
}

void CStore::SerializeSectionEnd()
{
	if (m_bIsStoring)
		*m_pIOS << endl;
	m_pIOS = NULL;
//	m_pAr = NULL;
	m_bFoundSection = false;
}

char* CStore::FindName(const char* strName)
{
	if (!m_bFoundSection)
	{
		LOGERROR1("Config section not found while searching for item - %s", strName);
		return NULL;
	}
	fstream& is = *m_pIOS;
	streampos posStartCheck = is.tellg();
	bool bLooped = false;
	for (;;)
	{
		is.getline(m_arLine, sizeof(m_arLine));
		char* pStr = m_arLine;
		extractWS(pStr);
		if (strextract(pStr, strName) && isspace(*pStr))		// check for space after name
			return pStr+1;												// points to start of value;
		if (is.eof() || m_arLine[0] == '[')		// next section heading, restart from current section heading
		{
			if (is.eof())
				is.clear();			// if eof
			is.seekg(m_posStartSection);
			bLooped = true;
		}
		if (bLooped && is.tellg() >= posStartCheck)
		{
			LOGERROR1("Config item not found '%s'", strName);
			return NULL;
		}
	}
}

void CStore::SerializeVal(const char* strName, int& iVal)
{
	if (m_bIsStoring)
		*m_pIOS << strName << "  " << iVal << endl;
	else
	{
		char* strVal = FindName(strName);	// returns pointer to value string
		if (strVal == NULL)						// not found so don't alter iVal
			return;
		sscanf(strVal, "%i", &iVal);
	}
}

void CStore::SerializeVal(const char* strName, bool& bVal)
{
	if (m_bIsStoring)
		*m_pIOS << strName << "  " << bVal << endl;
	else
	{
		char* strVal = FindName(strName);	// returns pointer to value string
		if (strVal == NULL)						// not found so don't alter iVal
			return;
		int val;
		if (sscanf(strVal, "%i", &val) == 1)
			bVal = val != 0;
	}
}

void CStore::SerializeVal(const char* strName, double& dVal)
{
	if (m_bIsStoring)
		*m_pIOS << strName << "  " << dVal << endl;
	else
	{
		char* strVal = FindName(strName);	// returns pointer to value string
		if (strVal == NULL)						// not found so don't alter iVal
			return;
		sscanf(strVal, "%lf", &dVal);
	}
}

void CStore::SerializeVal(const char* strName, float& fVal)
{
	if (m_bIsStoring)
		*m_pIOS << strName << "  " << fVal << endl;
	else
	{
		char* strVal = FindName(strName);	// returns pointer to value string
		if (strVal == NULL)						// not found so don't alter iVal
			return;
		sscanf(strVal, "%f", &fVal);
	}
}

void CStore::SerializeVal(const char* strName, CVector& vtVal)
{
	if (m_bIsStoring)
		*m_pIOS << strName << "  " << vtVal << endl;
	else
	{
		char* strVal = FindName(strName);	// returns pointer to value string
		if (strVal == NULL)						// not found so don't alter iVal
			return;
		double x, y, z;
		if (sscanf(strVal, "%lf %lf %lf", &x, &y, &z) == 3)
			vtVal.Set(x, y, z);
	}
}

