// ListArray.h: interface for the TListArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LISTARRAY_H__1A3289C9_52FA_4FE4_B7A4_935687E524DC__INCLUDED_)
#define AFX_LISTARRAY_H__1A3289C9_52FA_4FE4_B7A4_935687E524DC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include <afxtempl.h>


template<class TYPE, class ARG_TYPE> class TListArray : public CArray<TYPE, ARG_TYPE>
{
public:
	int GetMaxPosition() { return GetSize() - 1; }
	int GetMinPosition() { return GetSize() > 0 ? 0 : -1; }
	TYPE& GetLast() { return ElementAt(GetSize() - 1); }
	TYPE& GetFirst() { return ElementAt(0); }
	TYPE& GetPrev(int& pos)
	{
		ASSERT(pos >= 0 && pos < GetSize());
		return ElementAt(pos--);
	}
	TYPE& GetNext(int& pos)
	{
		ASSERT(pos >= 0 && pos < GetSize());
		TYPE& elem = ElementAt(pos++);
		if (pos >= GetSize())
			pos = -1;
		return elem;
	}
//	void RemoveLast() { RemoveAt(GetSize() - 1); }
	void ReverseList()
	{
		int iLow = 0;
		int iHigh = GetSize() - 1;
		while (iLow < iHigh)
		{
			TYPE tempElem = ElementAt(iLow);		// swap high/low elements
			ElementAt(iLow) = ElementAt(iHigh);
			ElementAt(iHigh) = tempElem;
			iLow++; iHigh--;
		}
	}
	void ReverseInto(TListArray& arrayRev)
	{
		int iDest = 0;
		int iSrc = GetSize() - 1;
		arrayRev.SetSize(iSrc + 1);
		while (iSrc >= 0)
		{
			arrayRev.ElementAt(iDest) = ElementAt(iSrc);
			iDest++; iSrc--;
		}
	}
	void operator=(TListArray& array)
	{
		int arraySize = array.GetSize();
		SetSize(arraySize);
		for (int i = 0; i < arraySize; i++)
			ElementAt(i) = array.ElementAt(i);
	}
};


template<class T, class ARG_T>
//ostream& operator<<(ostream& os, const TListArray<T, ARG_T>& array)
ostream& operator<<(ostream& os, const CArray<T, ARG_T>& array)
{
	int sizeArray = array.GetSize();
	os << "Array Size: " << sizeArray << endl;
	for (int i = 0; i < sizeArray; i++)
	{
		if (i % 10 == 0)
			if (i % 20 == 0)
				os << array[i].GetHeading();
			else
				os << endl;
		os << array[i];
	}
	return os;
}



#endif // !defined(AFX_LISTARRAY_H__1A3289C9_52FA_4FE4_B7A4_935687E524DC__INCLUDED_)
