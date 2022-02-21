// MachineTracker.h: interface for the CMachineTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MACHINETRACKER_H__71C8C822_46AD_11D8_86C3_F7309AB88725__INCLUDED_)
#define AFX_MACHINETRACKER_H__71C8C822_46AD_11D8_86C3_F7309AB88725__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "PktControl.h"
#include "MostRecentList.h"
#include "MachineState.h"
#include "ProbeSampler.h"

class CParamDoc;



struct SLocatorChangeData
{
	int iPos, idPos, iPosMid;
	int iPrevSpanDist, iPrevSpanError;
	BYTE iNewStateByte;
	BYTE iNewState;			// new state of changed locator
	BYTE iLocator;				// locator number that changed
	char iDirection;			// -1->idPos < 0, 1->idPos > 0, 0->idPos == 0
};
typedef TMostRecentList<SLocatorChangeData> CLocCngList;

struct SLocator
{
	int numSpans;
	int span[5];
	int edge[2];
	int width;
	int offsetPos;
	int offsetNeg;
	char idxLoc;			// locator number on axis

	int SumSpans() const;
	int CalcEdge(int idxEdge);
	void OffsetBy(double offset);
};

struct SLocatorFloat		// : public CStore
{
	int numSpans;
	int axis;
	double span[5];
	double edge[2];
	double edgeDiff;
	double width;
	double offsetPos;
	double offsetNeg;

	SLocatorFloat() {}
	void GetFromLocatorInt(const SLocator& locInt);
	void GetLocatorInt(SLocator& locInt) const;
//	void SerializeBody();
};


struct SAxisDataXcng
{
	int iState;		// 0 for locked, nz avaliable (val = result)
	int iAxisData[NUM_AXIS];
	int iRampTime;
	int iHoldTime;
	int iMethod;
	SAxisDataXcng() { iState = 1; }
	void ZeroData() { for (int i=0; i<NUM_AXIS; i++) iAxisData[i] = 0; }
};



class CMachineTracker  
{
public:
	CMachineTracker();
	virtual ~CMachineTracker();

	// messages
	void NotifyUserThread(int iNotification, int iData = 0);

	// comms initilisation
	void InitContact();
	void CountRxPacket() { m_iRxPktCount++; }
	void CountTxPacket() { m_iTxPktCount++; }

	// checks
	void CheckPathStage();		// done regularly to check on path processing
	void CheckInitContact();
	CMachineState& GetMachState() { return m_State; }

	// handle packet groups
	bool HandleDataPacket(CPacket& pkt);
	bool HandlePathPacket(CPacket& pkt);
	bool HandleStreamPacket(CPacket& pkt);

	// Handle specific data packets
	bool HandleParamPacket(CPacket& pkt);
	bool HandleRegPacket(CPacket& pkt);
	bool HandleMemPacket(CPacket& pkt);
	bool HandlePathConfPacket(CPacket& pkt);
	bool HandleControllerErrorPacket(CPacket& pkt);
	bool HandleCommsErrorPacket(CPacket& pkt);

	// parameter value handling functions
	bool ConfirmSentValueFrom(CPacket& pkt);
	bool SetParamValueFrom(CPacket& pkt);


	void OnRxControllerDescription(CPacket& pkt);
	void OnRxLocatorChangeState(CPacket& pkt);
	void OnRxIndexChange(CPacket& pkt);
	void OnRxSampleData(CPacket& pkt);
	void OnRxMotionUpdate(CPacket& pkt);
	void OnRxProbeSample(CPacket& pkt);

	// locator functions
	SLocator* MatchAxisLocator(SLocator& locFound, int iAxis);
	void LoadLocatorData();
	void StoreLocator(SLocatorFloat& loc);


	// machine setup
	void SetStandardParameters();

	// path control
	void PathReset();
	void PathSend();
	void PathStop();
	bool ProcessingPath() { return m_bProcessingPath; }
	void OnMachineFinishedPath();

	// moving commands
	bool MoveToPos();
	void ChangeVelocity(SAxisDataXcng* pADX);
	void SetVelocity(SAxisDataXcng* pADX);
	void StopMovement();
	void DisableMachine();


	int GetPathTime() { return (m_iTimePathStart == -1) ? 0 : GetTickCount() - m_iTimePathStart; }	// local guess of machine time in ms
	void SetPathStartTime() { m_iTimePathStart = GetTickCount(); }



protected:
	// queueing data
	int QueueHPPacket(CPacket& pkt) { return m_pPktControl->QueueHighPriority(pkt); }	// returns free space in queue
	int QueuePacket(CPacket& pkt) { return m_pPktControl->Queue(pkt); }	// returns free space in queue

	int SendPathSeg(SSegAccel& sa) { return m_pPktControl->QueuePathSeg(sa); }	// returns free space in queue
	int SendParam(int iParam, int iValue) { m_State.SetSent(iParam, iValue); return m_pPktControl->QueueSendParam(iParam, iValue); }
	int SendAxisParam(int iAxis, int iParam, int iValue) { m_State.SetSent(CAxPar(iAxis, iParam), iValue); return m_pPktControl->QueueSendAxisParam(iAxis, iParam, iValue); }
	int ReqParam(int iParam, int iValue = 0) { m_State.SetRequested(iParam); return m_pPktControl->QueueReqParam(iParam, iValue); }
	int ReqAxisParam(int iAxis, int iParam, int iValue = 0) { m_State.SetRequested(CAxPar(iAxis, iParam)); return m_pPktControl->QueueReqAxisParam(iAxis, iParam, iValue); }
	int ReqAxisParamAllAxes(int iParam, int iValue = 0) { m_State.SetRequestedAllAxis(iParam); return m_pPktControl->QueueReqAxisParamAllAxes(iParam, iValue); }



// data
public:
	CParamDoc* m_pParamDoc;
	CWinThread* m_pUserThread;
	CPktControl* m_pPktControl;

	// probe sampling
	CProbeSampler m_ProbeSampler;

protected:
	// tx/rx tracking
	int m_iTxPktCount;
	int m_iRxPktCount;


	// times
	int m_iTimePathStart;
	int m_iTimeBuffFreeReq;


	CMachineState m_State;			// for reported values
	bool m_bMachineStateChanged;

	// byte offsets for motion updates
	int m_iPosTrackOffset;
	int m_iVelTrackOffset;
	int m_iPosOffset;
	int m_iVelOffset;
	int m_iPosErrOffset;



	// machine state info
	int m_iContDescrLen;
	char m_szContDescr[100];
	int m_iTimeLastContDescr;
	CLocCngList m_arLocCngList[NUM_AXIS];


	// path data
	bool m_bProcessingPath;
	int m_iPathSendStage;


	int m_iExpSampleCount;
	int m_iSampleValueBytes;
	CArray<int, int> m_SampleValuesArray;


	// initial contact
	int m_iInitContactStage;
	int m_iInitContactOKCount;
	int m_iInitContactNoRespCount;
	int m_iInitContactErrorRespCount;
	int m_iTimeTxContactPkt;

	// axis locator data
	enum { MAX_LOCATORS = 6 };
	SLocator m_MachineLocators[NUM_AXIS][MAX_LOCATORS];	// up to 6 locators on each axis
	char m_arNumAxisLocators[NUM_AXIS];
	double m_arLocatorOrigin[NUM_AXIS];
	char m_arReferencedLocIdx[NUM_AXIS];
	char m_arReferencedLocDir[NUM_AXIS];


};

#endif // !defined(AFX_MACHINETRACKER_H__71C8C822_46AD_11D8_86C3_F7309AB88725__INCLUDED_)
