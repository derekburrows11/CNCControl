
#if !defined(STORE_H)
#define STORE_H


#include <fstream.h>
#include "Vector.h"




/////////////////////////////////////////////////////////////////////////////
// Helpers for saving/restoring window state

// read/write a window placement from settings section of app's ini file
bool ReadWindowPlacement(WINDOWPLACEMENT* pWP, LPCSTR szEntry);
bool WriteWindowPlacement(WINDOWPLACEMENT* pWP, LPCSTR szEntry);


/////////////////////////////////


class CStore
{
protected:
	bool m_bIsStoring;
	bool m_bFoundSection;
	streampos m_posStartSection;
	char m_arLine[120];
	char* m_strSection;

	fstream* m_pIOS;
//	CArchive* m_pAr;	// remove
	bool m_bDataValid;

public:
	void Serialize(CArchive& ar);
	void Serialize(fstream& ios, bool bStore);
	bool Valid() { return m_bDataValid; }

protected:
	CStore();
	void SetSection(char* strSection) { m_strSection = strSection; }
	virtual void SetSectionName() {}
	virtual void Defaults() {}
	virtual void SerializeBody() {}
	virtual void CalculateAfterLoad() {}

	void SerializeSectionEnd();
	void SerializeVal(const char* strName, int& iVal);
	void SerializeVal(const char* strName, bool& bVal);
	void SerializeVal(const char* strName, float& fVal);
	void SerializeVal(const char* strName, double& dVal);
	void SerializeVal(const char* strName, CVector& vtVal);

	char* FindName(const char* strName);

};


#endif	// #if !defined(STORE_H)

