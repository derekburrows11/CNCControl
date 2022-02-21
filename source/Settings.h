
#if !defined(SETTINGS_H)
#define SETTINGS_H


#include "Store.h"

#include "SettingsData.h"



// stored settings

////////////////////

class SMachinePathSegments : public CStore
{
public:
	int iMethod;				// standard segment time, or fit to longest time segment time
	double SegmentTime;		// sec	segment time for constant segments
	double PosTol;				// mm		for best fit method

	void Defaults();
	void SerializeBody();
	void SetSectionName();

//	DECLARE_SERIAL(SMachinePathSegments)
};

class SMachineParameters : public CStore
{
public:
	int iNumAxis;
	double dTime;				// sec	time step resolution of machine
	double dPosPerRev;		// mm		resolution of acceleration integer values
	int iCountsPerRev;
	int iPosTrackFractionBits;	// 24 fraction bits for 6 byte PosTrack

	double dPos;
	double dVel;
	double dAcc;
	double dPosTrack;
	double dVelTrack;
	double dAccTrack;

	CVector vtJerkMax;
	CVector vtAccMax;
	CVector vtVelMax;
	double SpeedXYMax;
	double SpeedXYZMax;
	double FeedRateMax;

	double AngCuspSegments;		// degrees - segments with an change above this will cause machine to stop and restart

	CVector BacklashPositive;	// mm backlash compensation for positive movement
	CVector BacklashNegative;	// mm backlash compensation for negative movement

	void Defaults();
	void SerializeBody();
	void SetSectionName();
	void CalculateAfterLoad();
};


class SMachineDimensions : public SMachineDimensionsData, public CStore
{
public:
	void Defaults();
	void SerializeBody();
	void SetSectionName();
};

class SProbeDimensions : public SProbeDimensionsData, public CStore
{
public:
	void Defaults();
	void SerializeBody();
	void SetSectionName();
};

class SControllerTracker : public CStore
{
public:
	double wtPos, wtVel, wtAcc;		// weighting for accel without step
	double wtPV, wtPA, wtVA;			// weighting for accel with step
	bool bAllowAccStep;					// allow accel step for controller path segments

	void Defaults();
	void SerializeBody();
	void SetSectionName();
};

class SMachineComms : public CStore
{
public:
	int iCommPort;
	bool bSimulateResponse;

	void Defaults();
	void SerializeBody();
	void SetSectionName();
};

class SPathOptions : public CStore
{
public:
	SSmoothSegsData m_SmoothSegs;
	SJoiningTabData m_JoinTab;

	void Defaults();
	void SerializeBody();
	void SetSectionName();
};

////////////////////////////////

class CSettings
{
public:
	// data structures
	SMachinePathSegments MachPathSeg;
	SMachineParameters MachParam;
	SControllerTracker ContTrack;
	SMachineComms MachComms;
	SMachineDimensions MachDimensions;
	SProbeDimensions ProbeDimensions;
	SPathOptions PathOptions;

	char m_szFile[MAX_PATH];
	bool m_bModified;

	CSettings();
	void SetFileName(const char* szFile);
	void Modified() { m_bModified = true; }
	void Serialize(bool bStoring);

	void Load() { Serialize(false); }
	void Store() { Serialize(true); }
//	void Load2();
//	void Store2();
//	void Load3();
//	void Store3();
};

extern CSettings g_Settings;		// declare global object




#endif	// #if !defined(SETTINGS_H)


