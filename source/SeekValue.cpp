// SeekValue.cpp: implementation of the CSeekValue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <math.h>
#include "SeekValue.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSeekValue::CSeekValue()
{
	Restart();
	m_Tol = 1e-6;		// tolerance for value range to include seekvalue
	m_bValueSet = false;
}

CSeekValue::CSeekValue(double Value)
{
	// this->CSeekValue::CSeekValue();	// Runs constructor for 'this'
	Restart();
	m_Tol = 1e-6;		// tolerance for value range to include seekvalue
	m_SeekValue = Value;
	m_bValueSet = true;
}

void CSeekValue::SetValue(double Value)
{
	Restart();
	m_SeekValue = Value;
	m_bValueSet = true;
}

void CSeekValue::Restart()
{
	m_iNumPoints = 0;
	m_iNumIterations = 0;
	m_bFoundBestS = false;
	m_bRangeCovered = false;
	m_bRangeWithinTol = false;
}


int CSeekValue::Point(double S, double Val)
{
	ASSERT(m_bValueSet);
	Val -= m_SeekValue;		// convert to relative
	m_LastPlacement = 0;
	m_HomingRatio = 1;

	newpt.s = S;
	newpt.val = Val;
	newpt.vsign = (char)(Val >= 0 ? 1 : -1);
	if (Val == 0) newpt.vsign = 0;

// debug checking.....
	if (m_iNumPoints == 0) m_arPointIdx = 0;
	m_arPoint[m_arPointIdx] = newpt;
	if (++m_arPointIdx >= m_arPointSize)
		m_arPointIdx = 0;
	m_iNumPoints++;
	ASSERT(m_iNumPoints < 50);

	// m_iNumPoints is number after this point is added
	if (m_iNumPoints >= 3)
	{
		if (S < min(low.s, high.s) || S > max(low.s, high.s))
		{
			TRACE0("CSeekValue::Point - S is outside current S range\n");
			ASSERT(0);
			return 0;	// iPlacement = 0
		}
		// newpt.s is within low.s<->high.s but not neccessarily value
		if (newpt.vsign == 1)
			high = newpt;
		else
			low = newpt;
	}
	else if (m_iNumPoints == 2)
	{
		if (newpt.vsign == 1)		// high.val & low.val are currently the same!
			high = newpt;
		else
			low = newpt;
		m_LastPlacement |= SR_CLOSE1;

		m_bRangeCovered = true;
		m_bRangeWithinTol = true;
		if (low.vsign == 1 || high.vsign == -1)
		{
			m_bRangeCovered = false;
			// check if one value is very close
			if (fabs(low.val) > m_Tol && fabs(high.val) > m_Tol)
			{
//				TRACE1("CSeekValue::Point() - Value range does not include seek value: %f\n", m_SeekValue);
//				ASSERT(0);
				m_bRangeWithinTol = false;
				m_iNumPoints--;		// will assert when trying to seek next s
				return 0;	// iPlacement = 0
			}
			else		//	use close value
			{
			}
		}
		if (low.s == high.s && (low.vsign != 0 || high.vsign != 0))
		{
			// check if both values aren't very close
			if (fabs(low.val) > m_Tol || fabs(high.val) > m_Tol)
			{
				TRACE("CSeekValue::Point() - No S range, value steps through seek value\n");
				ASSERT(0);
				return 0;	// iPlacement = 0
			}
			else		//	very small step!
			{
			}
		}
	}
	else // (m_iNumPoints == 1)
	{
		close1 = high = low = newpt;		// low & high refer to value
		m_LastPlacement = SR_LOW | SR_HIGH | SR_CLOSE1;
		return m_LastPlacement;
	}
	m_LastPlacement |= (newpt.vsign == 1) ? SR_HIGH : SR_LOW;

	if (newpt.vsign == close3.vsign || fabs(newpt.val) < fabs(close3.val) || m_iNumPoints <= 3)	// if on same side it will be closer if within s range!
		if (newpt.vsign == close2.vsign || fabs(newpt.val) < fabs(close2.val) || m_iNumPoints <= 2)
			if (newpt.vsign == close1.vsign || fabs(newpt.val) < fabs(close1.val))
			{
				close3 = close2;
				close2 = close1;
				close1 = newpt;
				m_LastPlacement |= SR_CLOSE1;
				m_HomingRatio = fabs(newpt.val / close2.val);
			}
			else
			{
				close3 = close2;
				close2 = newpt;
				m_LastPlacement |= SR_CLOSE2;
			}
		else
		{
			close3 = newpt;
			m_LastPlacement |= SR_CLOSE3;
		}
	return m_LastPlacement;
}

double CSeekValue::NextSGuess()
{
	ASSERT(m_iNumPoints >= 2);
	double Snew, Wt;

	if (m_iNumPoints == 2)
	{
		//if ((low.vsign == 1 || high.vsign == -1)	// changed 2/12/04
		if ((low.vsign != -1 || high.vsign != 1)
			&& (fabs(low.val) <= m_Tol || fabs(high.val) <= m_Tol))		// check if one value is very close
				Snew = (fabs(low.val) < fabs(high.val)) ? low.s : high.s;
		else
		{
			Wt = high.val / (high.val - low.val);	// low point weighting
			Snew = Wt * low.s + (1 - Wt) * high.s;
		}
	}
	else if ((m_LastPlacement & SR_CLOSE1 && m_HomingRatio < 0.5) || m_iNumPoints == 3)
	{
		// following is a 3 point quadratic interpolation (derived from CPathTracker::FindSAtPos or somewhere else?)		

		// set points 1/2 relative to close1
		double s0, s1, s2;
		double val0, val1, val2;
		s0 = close1.s;
		val0 = close1.val;
		s1 = close2.s - s0;
		s2 = close3.s - s0;
		val1 = close2.val - val0;
		val2 = close3.val - val0;
		// get 2x2 inverse to find a,b coefficents of quadratic
		double det = s1*s2*(s1-s2);
		ASSERT(det != 0);
		det = 1 / det;
		double a, b, c;		// quadratic coefficents
		a = (    s2*val1 -    s1*val2) * det;
		b = (-s2*s2*val1 + s1*s1*val2) * det;
		c = val0;

		double ds;
		if (fabs(a) < 1e-3)
			ds = -c / b;			// Take tangent at current Pos if curve is negligable
		else
		{
			double b24ac = b*b - 4*a*c;
			if (b24ac >= 0)
				if (high.s > low.s)		// increasing slope
					ds = (-b + sqrt(b24ac)) / (2*a);
				else
					ds = (-b - sqrt(b24ac)) / (2*a);
			else							// could use low & high in points so it will cross 0
				ds = -c / b;			// Take tangent at current Pos
		}
		Snew = s0 + ds;


		// check Snew is between low.s and high.s
		if (Snew < min(low.s, high.s) || Snew > max(low.s, high.s))
		{		
			Wt = close2.val / (close2.val - close1.val);	// close1 point weighting
			Snew = Wt * close1.s + (1 - Wt) * close2.s;
			if (Snew < min(low.s, high.s) || Snew > max(low.s, high.s))
			{
				Wt = high.val / (high.val - low.val);		// low point weighting
				Snew = Wt * low.s + (1 - Wt) * high.s;		// go between low/high if close extrapolation is not good
			}
		}
	}
	else
		Snew = (low.s + high.s) / 2;		// go midway if last point not good

	// check Snew is between low.s and high.s
	double Smin = min(low.s, high.s);
	double Smax = max(low.s, high.s);
	if (Snew < Smin)
	{
		ASSERT(Snew >= Smin - 1e-14);
		Snew = Smin;
	}
	if (Snew > Smax)
	{
		ASSERT(Snew <= Smax + 1e-14);
		Snew = Smax;
	}


	// check if Snew is same as previous low.s or high.s
	if ((Snew == low.s || Snew == high.s) && m_iNumIterations >= 4)	// make sure it's had a couple of guesses!
	{
		m_bFoundBestS = true;			// found best s value at s resolution. Guess after this will be same as low/high.s will be same!
		ASSERT(low.s != high.s || low.val == high.val);		// make sure values are equal if same s
	}
	
	m_iNumIterations++;
	newpt.s = Snew;
	return Snew;
}

double CSeekValue::GetBestValue()
{
	ASSERT(m_bFoundBestS);
	ASSERT(low.s != high.s || low.val == high.val);		// make sure values are equal if same s
	if (newpt.s == low.s)
		newpt.val = low.val;
	else if (newpt.s == high.s)
		newpt.val = high.val;
	return newpt.val;
}

