// SeekValue.h: interface for the CSeekValue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SEEKVALUE_H)
#define SEEKVALUE_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000



class CSeekValue
{
public:
	enum
	{
		SR_LOW = 0x01,
		SR_HIGH = 0x02,
		SR_HIGHORLOW = SR_LOW | SR_HIGH,
		SR_CLOSE1 = 0x10,
		SR_CLOSE2 = 0x20,
		SR_CLOSE3 = 0x40,
		SR_CLOSE = SR_CLOSE1 | SR_CLOSE2 | SR_CLOSE3,
	};
	struct SVAL { double s, val; char vsign; };

protected:
	SVAL low, high;					// low & high in val range
	SVAL close1, close2, close3;	// closest in val range
	SVAL newpt;
	double m_SeekValue;			// absolute value
	double m_Tol;
	int m_iNumIterations;
	int m_iNumPoints;
	bool m_bValueSet;
	int m_LastPlacement;
	double m_HomingRatio;
	bool m_bFoundBestS;		// signals closest s and val attainable has been reached
	bool m_bRangeCovered;
	bool m_bRangeWithinTol;

	// for debuging
	enum {m_arPointSize = 50};
	SVAL m_arPoint[m_arPointSize];
	int m_arPointIdx;

public:
	CSeekValue();
	CSeekValue(double Value);
	void Restart();
	void SetValue(double Value);
	void SetTolerance(double tol) { m_Tol = tol; }
	int Point(double S, double Val);
	double NextSGuess();
	bool FoundBestS() { return m_bFoundBestS; }
	bool RangeCovered() { return m_bRangeCovered; }
	bool RangeWithinTol() { return m_bRangeWithinTol; }
	double GetBestValue();
	double ValueError(double Val) { return Val - m_SeekValue; }
	double Value() { return m_SeekValue; }
	int NumPoints() { return m_iNumPoints; }

};



#endif // !defined(SEEKVALUE_H)
