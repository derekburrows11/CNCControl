
#include "stdafx.h"
#include "cnccontrol.h"


#include "Settings.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CSettings g_Settings;	// globaly declared settings







/////////////////////////////////////////////////
// SMachineComms
/////////////////////////////////////////////////

void SMachineComms::SetSectionName()
{
	SetSection("MACHINE COMMS");
}

void SMachineComms::SerializeBody()
{
	SerializeVal("CommPort", iCommPort);
	SerializeVal("Simulate Response", bSimulateResponse);
}

void SMachineComms::Defaults()
{
	iCommPort = 2;
	bSimulateResponse = false;
}

/////////////////////////////////////////////////
// SMachinePathSegments
/////////////////////////////////////////////////

void SMachinePathSegments::SetSectionName()
{
	SetSection("MACHINE PATH SEGMENTS");
}

void SMachinePathSegments::SerializeBody()
{
	SerializeVal("Method", iMethod);
	SerializeVal("Segment Time", SegmentTime);
	SerializeVal("Fit Position Tol", PosTol);
}

void SMachinePathSegments::Defaults()
{
	iMethod = 0;
	SegmentTime = 0.1;		// sec
	PosTol = 0.01;				// mm
}

/////////////////////////////////////////////////
// SMachineParameters
/////////////////////////////////////////////////

void SMachineParameters::SetSectionName()
{
	SetSection("MACHINE PARAMETERS");
}

void SMachineParameters::SerializeBody()
{
	SerializeVal("Number of Axis", iNumAxis);
	SerializeVal("Time Resolution", dTime);				// sec			time step resolution of machine
	SerializeVal("dPosition Per Rev", dPosPerRev);		// 2.5mm for thread
	SerializeVal("Counts Per Rev", iCountsPerRev);		// 4096 for encoders
	SerializeVal("PosTrack Fraction Bits", iPosTrackFractionBits);	// 24 fraction bits for 6 byte PosTrack

	SerializeVal("Vel Max", vtVelMax);
	SerializeVal("Accel Max", vtAccMax);
	SerializeVal("Jerk Max", vtJerkMax);
	SerializeVal("SpeedXY Max", SpeedXYMax);
	SerializeVal("SpeedXYZ Max", SpeedXYZMax);
	SerializeVal("FeedRate Max", FeedRateMax);

	SerializeVal("Angle Cusp Segments", AngCuspSegments);

	SerializeVal("Backlash Positive", BacklashPositive);
	SerializeVal("Backlash Negative", BacklashNegative);
	
}

void SMachineParameters::Defaults()
{
	iNumAxis = 3;
	dTime = 1.024e-3;					// sec	time step resolution of machine
	dPosPerRev = 2.5;					// mm		2.5mm for thread
	iCountsPerRev = 4096;			// 4096 for encoders
	iPosTrackFractionBits = 24;	// 24 fraction bits for 6 byte PosTrack

	vtVelMax.Set(25, 25, 20);
	vtAccMax.Set(50, 50, 40);
	vtJerkMax.Set(200, 200, 140);
	SpeedXYMax  = 25;
	SpeedXYZMax = 25;
	FeedRateMax = 25;

	AngCuspSegments = 2.0;	// deg	segments with a change above this will cause a stop and start

	BacklashPositive.Set(0.15, 0.15, 0.15);
	BacklashNegative.Set(0, 0, 0);

	CalculateAfterLoad();
}

void SMachineParameters::CalculateAfterLoad()
{
	dPos = dPosPerRev / iCountsPerRev;						// mm
	dVel = dPos / dTime / 2;									// mm/sec		iVel is Vel*2
	dAcc = dPos / (dTime*dTime);								// mm/sec^2
	dPosTrack = dPos / (1 << iPosTrackFractionBits);	// mm
	dVelTrack = dPosTrack * 3 / dTime;						// mm/sec		iVel is Vel*2
	dAccTrack = dPosTrack * 6 / (dTime*dTime);			// mm/sec^2		iPos is Pos*6

}


/////////////////////////////////////////////////
// SMachineDimensions
/////////////////////////////////////////////////

void SMachineDimensions::SetSectionName()
{
	SetSection("MACHINE DIMENSIONS");
}

void SMachineDimensions::SerializeBody()
{
	SerializeVal("Mount to ZAxisHead", m_dMount2ZAxisHead);
	SerializeVal("Chuck to Mount", m_dChuck2Mount);
	SerializeVal("Tip to Chuck", m_dTip2Chuck);
	SerializeVal("Vacuum Board Thickness", m_dVacuumBoard);
	SerializeVal("Base Board Thickness", m_dBaseBoard);
	SerializeVal("Axis Adjust", m_zAxisAdjust);
	SerializeVal("Probe Adjust", m_dProbeAdjust);
	SerializeVal("Stock Thickness", m_dStock);
	SerializeVal("Probe On Stock", m_bProbeOnStock);
	SerializeVal("Dia. Tool", m_diaTool);
}

void SMachineDimensions::Defaults()
{
	m_dMount2ZAxisHead = 143.0;	// 143.0mm to router mount edge to Z Axis Head angle iron
	m_dChuck2Mount = 56.0;
	m_dTip2Chuck = 32.0;				// depends on bit!!
	m_dVacuumBoard = 12.5;
	m_dBaseBoard = 18.5;
	m_zAxisAdjust = 0;
	m_dProbeAdjust = 0;
	m_dStock = 0;
	m_bProbeOnStock = false;
	m_diaTool = 25.4 / 4;
}

/////////////////////////////////////////////////
// SProbeDimensions
/////////////////////////////////////////////////

void SProbeDimensions::SetSectionName()
{
	SetSection("PROBE DIMENSIONS");
}

void SProbeDimensions::SerializeBody()
{
	SerializeVal("AxisHead to Tip Retracted", m_AxisHead2TipRetracted);
	SerializeVal("Stop Probe Extension", m_StopProbeExt);
	SerializeVal("Max Probe Extension", m_MaxProbeExt);
	SerializeVal("Angle Change Detect", m_AngleChangeDetect);
	SerializeVal("Min Dist To Log", m_MinDistToLog);
}

void SProbeDimensions::Defaults()
{
	m_AxisHead2TipRetracted = 249.0;	// mm
	m_StopProbeExt = 2.0;			// mm
	m_MaxProbeExt = 27.3;			// mm
	m_AngleChangeDetect = 5.0;		// degrees
	m_MinDistToLog = 5.0;			// mm
}

/////////////////////////////////////////////////
// SControllerTracker
/////////////////////////////////////////////////

void SControllerTracker::SetSectionName()
{
	SetSection("CONTROLLER TRACKER");
}

void SControllerTracker::SerializeBody()
{
	SerializeVal("WtPosContinuous", wtPos);			// weighting for accel without step
	SerializeVal("WtVelContinuous", wtVel);
	SerializeVal("WtAccContinuous", wtAcc);
	SerializeVal("WtPVStep", wtPV);						// weighting for accel with step
	SerializeVal("WtPAStep", wtPA);
	SerializeVal("WtVAStep", wtVA);

	SerializeVal("Allow Accel Step", bAllowAccStep);
}

void SControllerTracker::Defaults()
{
	wtPos = 0.1;						// 0.1  0.01 is better
	wtVel = 0.45;						// 0.45 0.45
	wtAcc = 1 - wtPos - wtVel;		// 0.45
	wtPV = 0.025;
	wtPA = 0.10;
	wtVA = 0.30;
// origional defaults
/*
	wtPos = 0.1;						// 0.1  0.01 is better
	wtVel = 0.45;						// 0.45 0.45
	wtAcc = 1 - wtPos - wtVel;		// 0.45
	wtSIPosVel = 0.1;									// 0.4
	wtSIVelAcc = 0.7;									// 0.2
	wtSIPosAcc = 1 - wtSIPosVel - wtSIVelAcc;	// 0.4
	wtSFPosVel = 0.1;									// 0.4
	wtSFVelAcc = 0.7;									// 0.2
	wtSFPosAcc = 1 - wtSFPosVel - wtSFVelAcc;	// 0.4
*/	
	bAllowAccStep = true;
}


/////////////////////////////////////////////////
// SPathOptions
/////////////////////////////////////////////////

void SPathOptions::SetSectionName()
{
	SetSection("PATH OPTIONS");
}

void SPathOptions::SerializeBody()
{
	SerializeVal("Smooth segs", m_SmoothSegs.bSmooth);
	SerializeVal("Smooth seg tol", m_SmoothSegs.segTol);
	SerializeVal("Smooth seg tol dir set", m_SmoothSegs.segTolDirSet);
	SerializeVal("Smooth seg straight min", m_SmoothSegs.segStraightMin);
	SerializeVal("Smooth seg ang cusp", m_SmoothSegs.angCusp);

	SerializeVal("Add Joining Tabs", m_JoinTab.bAdd);
	SerializeVal("Tab Height", m_JoinTab.height);
	SerializeVal("Tab Width", m_JoinTab.width);
	SerializeVal("Tab Ramp Length", m_JoinTab.rampLength);
	SerializeVal("Tab z Low Pass Max", m_JoinTab.zLowPassMax);
}

void SPathOptions::Defaults()
{
	m_SmoothSegs.bSmooth = true;
	m_SmoothSegs.segTol = 0.026;
	m_SmoothSegs.segTolDirSet = m_SmoothSegs.segTol * 1.375;
	m_SmoothSegs.segStraightMin = 3.0;
	m_SmoothSegs.angCusp = 10;

	m_JoinTab.bAdd = true;
	m_JoinTab.height	= 0.8;
	m_JoinTab.width	= 5.0;
	m_JoinTab.rampLength = 5.0;
	m_JoinTab.zLowPassMax = 0.1;
}







/////////////////////////////////////////////////
// CSettings
/////////////////////////////////////////////////

CSettings::CSettings()
{
	m_szFile[0] = 0;
	Modified();
}

void CSettings::SetFileName(const char* szFile)
{
	strcpy(m_szFile, szFile);
}

void CSettings::Serialize(bool bStoring)
{
	fstream fs;
	if (bStoring)
		fs.open(m_szFile, ios::out);
	else
		fs.open(m_szFile, ios::in | ios::nocreate);
	
	if (fs.fail())
		LOGERROR1("Couldn't open config file %s", m_szFile);

	MachPathSeg.Serialize(fs, bStoring);
	MachParam.Serialize(fs, bStoring);
	ContTrack.Serialize(fs, bStoring);
	MachComms.Serialize(fs, bStoring);
	MachDimensions.Serialize(fs, bStoring);
	ProbeDimensions.Serialize(fs, bStoring);
	PathOptions.Serialize(fs, bStoring);

	fs.close();
}


/*
void CSettings::Load2()
{
	CFile file;
	if (file.Open(m_SettingsFile, CFile::modeRead))
	{
		CArchive ar(&file, CArchive::load);
		MachPathSeg.Serialize(ar);
		MachParam.Serialize(ar);
		ContTrack.Serialize(ar);
		ar.Close();
		file.Close();
	}
	else
	{
		TRACE0("Loading default settings\n");
		MachPathSeg.Defaults();
		MachParam.Defaults();
		ContTrack.Defaults();
	}
	m_bModified = false;
}

void CSettings::Store2()
{
	CFile file;
	if (file.Open(m_SettingsFile, CFile::modeCreate | CFile::modeWrite))
	{
		CArchive ar(&file, CArchive::store);
		MachPathSeg.Serialize(ar);
		MachParam.Serialize(ar);
		ContTrack.Serialize(ar);
		ar.Close();
		file.Close();
		m_bModified = false;
	}
}


void CSettings::Load3()
{
	Load();
	return;

	// get values from ini file preferences
	CWinApp* pApp = AfxGetApp();
	const char* szSection;
	CString str;

	szSection = MPS.szSection;
	MPS.iMethod = pApp->GetProfileInt(szSection, sziMethod, 0);
	str = pApp->GetProfileString(szSection, szSegmentTime, "0.1");
	MPS.SegmentTime = atof(str);
	str = pApp->GetProfileString(szSection, szPosTol, "0.01");
	MPS.PosTol = atof(str);

	szSection = MP.szSection;
	MP.iNumAxis = pApp->GetProfileInt(szSection, sziNumAxis, 3);
	str = pApp->GetProfileString(szSection, szStepTime, "1e-3");
	MP.dTime = atof(str);

	m_bModified = false;

}

void CSettings::Store3()
{
	Store();
	return;

	// set ini preference values
	CWinApp* pApp = AfxGetApp();
	const char* szSection;
	char buffer[50];

	szSection = MPS.szSection;
	pApp->WriteProfileInt(szSection, sziMethod, MPS.iMethod);
	_gcvt(MPS.SegmentTime, 10, buffer);
	pApp->WriteProfileString(szSection, szSegmentTime, buffer);
	_gcvt(MPS.PosTol, 10, buffer);
	pApp->WriteProfileString(szSection, szPosTol, buffer);

	szSection = MP.szSection;
	pApp->WriteProfileInt(szSection, sziNumAxis, MP.iNumAxis);
	_gcvt(MP.dTime, 10, buffer);
	pApp->WriteProfileString(szSection, szStepTime, buffer);

	m_bModified = false;

}

*/
