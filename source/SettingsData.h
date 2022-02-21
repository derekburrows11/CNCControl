//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SETTINGSDATA_H__5CD80B5F_BDE9_408C_A4B8_CE9137AFD906__INCLUDED_)
#define AFX_SETTINGSDATA_H__5CD80B5F_BDE9_408C_A4B8_CE9137AFD906__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000




struct SMachineDimensionsData
{
	double m_dMount2ZAxisHead;
	double m_dChuck2Mount;
	double m_dTip2Chuck;
	double m_dVacuumBoard;
	double m_dBaseBoard;
	double m_zAxisAdjust;
	double m_dProbeAdjust;
	double m_dStock;
	BOOL m_bProbeOnStock;
	double m_diaTool;

	double GetTip2ZAxisHeadDist();
	double GetAxisPosTipAtBaseTop();
};

struct SProbeDimensionsData
{
	double m_AxisHead2TipRetracted;
	double m_StopProbeExt;
	double m_MaxProbeExt;
	double m_AngleChangeDetect;
	double m_MinDistToLog;
};


struct SSmoothSegsData
{
	BOOL bSmooth;
	double segTol;
	double segTolDirSet;
	double segStraightMin;
	double angCusp;
};

struct SJoiningTabData
{
	BOOL bAdd;
	double height;
	double width;
	double rampLength;
	double zLowPassMax;
};







#endif // !defined(AFX_SETTINGSDATA_H__5CD80B5F_BDE9_408C_A4B8_CE9137AFD906__INCLUDED_)
