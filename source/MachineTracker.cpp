// MachineTracker.cpp: implementation of the CMachineTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNCControl.h"

#include "MachineTracker.h"

#include "ParamDoc.h"
#include "ThreadMessages.h"
#include "StrUtils.h"

#include "MachineCodes.h"

#include <iomanip.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////




int SLocator::SumSpans() const
{
	int iWidth = 0;
	for (int i = numSpans - 1; i >= 0; i--)
		iWidth += span[i];
	return iWidth;
}

int SLocator::CalcEdge(int idxEdge)
{
	if (idxEdge == 0)
		return edge[0] = edge[1] - SumSpans();
	if (idxEdge == 1)
		return edge[1] = edge[0] + SumSpans();
	ASSERT(0);
	return 0;
}

void SLocator::OffsetBy(double offset)
{
	int iOffset = NEARINT(offset / g_Settings.MachParam.dPos);
	edge[0] += iOffset;
	edge[1] += iOffset;
}
/*
void SLocatorFloat::SerializeBody()
{
	SerializeVal("num spans", numSpans);
	if (numSpans > 10) numSpans = 10;
	char szSpan[] = "span0";
	for (int i = 0; i < numSpans; i++)
	{
		szSpan[3] = char('0' + i);
		SerializeVal(szSpan, span[i]);
	}
	SerializeVal("edge low", edge[0]);
	SerializeVal("edge high", edge[1]);
	SerializeVal("edge diff", edgeDiff);
	SerializeVal("width", width);
}
*/
void SLocatorFloat::GetFromLocatorInt(const SLocator& locInt)
{
	numSpans = locInt.numSpans;
	double dPosEnc = g_Settings.MachParam.dPos;
	for (int i = 0; i < numSpans; i++)
		span[i] = locInt.span[i] * dPosEnc;
	edge[0] = locInt.edge[0] * dPosEnc;
	edge[1] = locInt.edge[1] * dPosEnc;
	edgeDiff = edge[1] - edge[0];
	width = locInt.SumSpans() * dPosEnc;
}

void SLocatorFloat::GetLocatorInt(SLocator& locInt) const
{
	locInt.numSpans = numSpans;
	double OneOndPosEnc = 1.0 / g_Settings.MachParam.dPos;
	for (int i = 0; i < numSpans; i++)
		locInt.span[i] = NEARINT(span[i] * OneOndPosEnc);
	locInt.edge[0] = NEARINT(edge[0] * OneOndPosEnc);
	locInt.edge[1] = NEARINT(edge[1] * OneOndPosEnc);
	locInt.width = NEARINT(width * OneOndPosEnc);
	locInt.offsetPos = NEARINT(offsetPos * OneOndPosEnc);
	locInt.offsetNeg = NEARINT(offsetNeg * OneOndPosEnc);
}


ostream& operator<<(ostream& os, const SLocator& loc)
{
	double dPosEnc = g_Settings.MachParam.dPos;
	const int w = 9;
	os.setf(ios::fixed);
	os.precision(4);		// so can be converted back to exact integer (res 0.6um)
	os << "edge0     " << setw(w) << loc.edge[0] * dPosEnc << endl;
	os << "tabs      ";
	for (int i = 0; i < loc.numSpans; i += 2)
		os << setw(w) << loc.span[i] * dPosEnc;
	os << endl;
	os << "gaps           ";
	for (i = 1; i < loc.numSpans; i += 2)
		os << setw(w) << loc.span[i] * dPosEnc;
	os << endl;

	os << "width     " << setw(w) << loc.SumSpans() * dPosEnc << endl;
	os << "edge1     " << setw(w) << loc.edge[1] * dPosEnc << endl;
	return os;
}




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMachineTracker::CMachineTracker()
{
	m_pUserThread = NULL;
	m_pParamDoc = NULL;
	m_pPktControl = NULL;

	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		m_arLocCngList[ax].SetBufferSize(10);
		m_arReferencedLocIdx[ax] = -1;
		m_arReferencedLocDir[ax] = 0;
	}

		// tx/rx tracking
	m_iTxPktCount = 0;
	m_iRxPktCount = 0;


	// initial contact
	m_iInitContactStage = 0;
	m_iContDescrLen = 0;

	m_iSampleValueBytes = 2;
	m_iExpSampleCount = 0;


		// path control
	m_iPathSendStage = 0;		
	m_bProcessingPath = false;
	m_iTimePathStart = -1;


	m_bMachineStateChanged = false;

	m_iPosTrackOffset = 0x1000000;	// 3 bytes
	m_iVelTrackOffset = 0x100;			// 1 byte
	m_iPosOffset = 0x1;
	m_iVelOffset = 0x1;
	m_iPosErrOffset = 0x1;

	SMachineParameters& mp = g_Settings.MachParam;
	m_State.dPos = mp.dPos;
	m_State.dVel = mp.dVel;
	m_State.dPosTrack = mp.dPosTrack;
	m_State.dVelTrack = mp.dVelTrack;
	m_State.dTime = mp.dTime;


	LoadLocatorData();

	// load dimension values
	ASSERT(g_Settings.MachDimensions.Valid());
	m_State.SetZAxisPosTipAtBaseTop(g_Settings.MachDimensions.GetAxisPosTipAtBaseTop());


	m_ProbeSampler.SetMachineState(&m_State);
	m_ProbeSampler.SetMachineTracker(this);


}

CMachineTracker::~CMachineTracker()
{

}

void CMachineTracker::NotifyUserThread(int iNotification, int iData)
{
	m_pUserThread->PostThreadMessage(WMU_COMMNOTIFY, iNotification, iData);
}

void CMachineTracker::OnRxControllerDescription(CPacket& pkt)
{
	// reset if no recent packets
	if (m_iContDescrLen != 0 && (GetTickCount() - m_iTimeLastContDescr) > 5000)
		m_iContDescrLen = 0;

	int iLen = pkt.GetStreamDataLen();
	if (m_iContDescrLen + iLen > sizeof(m_szContDescr))
		m_iContDescrLen = 0;

	memcpy(m_szContDescr + m_iContDescrLen, pkt.GetStreamData(), iLen);
	m_iContDescrLen += iLen;
	
	m_iTimeLastContDescr = GetTickCount();
//	if (m_iContDescrLen != 0 && m_szContDescr[m_iContDescrLen-1] == 0)		// got all of description!
//		;
}

void CMachineTracker::OnRxLocatorChangeState(CPacket& pkt)
{
	ASSERT(pkt.GetStreamDataLen() == 7);
	BYTE* pData = pkt.GetStreamData();
	int iPos = (*(int*)(pData+2)) & 0x00ffffff;	// take 3 bytes
	if (iPos & 0x00800000)				// if negative 24 bit value
		iPos |= 0xff000000;				// sign extend 24 bit value
	int idPos = *(short*)(pData+5);					// take 2 bytes
	int iAxis = (pData[0] & 0x0f) - 1;	// axis comes as 1-3
	int iStateByte = pData[1];

	char iDir = char(idPos >= 0 ? (idPos == 0 ? 0 : 1) : -1);
	int iChanged = (iStateByte >> 4) & 0x07;
	int iState = iStateByte & 0x07;
	bool bMissedChange = (iChanged == 0);			// no locators changed -> reporting a missed change
	bool bPosValid = (iChanged != 0);
	int iPosChangeMid = iPos - idPos/2;				// mid sample point

	if (iAxis < 0 || iAxis >= NUM_AXIS)			// bad case!
	{
		LOGERROR("Bad axis number from locator change");
		return;
	}

	CLocCngList& locCngList = m_arLocCngList[iAxis];
	SLocatorChangeData cng;
	cng.iPos = iPos;
	cng.idPos = idPos;
	cng.iPosMid = iPosChangeMid;
	cng.iNewStateByte = (BYTE)iStateByte;			// new state byte
	cng.iDirection = iDir;


	double dPosEncoder = g_Settings.MachParam.dPos;
	int m_iLocTabSizeMax = int(30.0 / dPosEncoder);
	int m_iLocatorSpanMin = int(0.5 / dPosEncoder);

	if (iAxis == 0 || iAxis == 1)		// X or Y axis
	{
		// only locator 1 used 
		if (iChanged & 0x06)
			LOGERROR("Invalid locator for axis X or Y");
		cng.iLocator = 1;
		cng.iNewState = (iState & 0x01) != 0;		// new state of locator 1

		if (locCngList.GetCount() == 0)		// check spans if some recorded
		{
			cng.iPrevSpanDist = cng.iPrevSpanError = 0;
			locCngList.Add(cng);
		}
		else if (bPosValid)
		{
			SLocatorChangeData& cngPrev = locCngList[0];		// previous axis change
			cng.iPrevSpanDist = abs(cng.iPosMid - cngPrev.iPosMid);
			cng.iPrevSpanError = abs(cng.idPos + cngPrev.idPos) / 2;
			if (abs(cng.iPrevSpanDist) >= m_iLocatorSpanMin)
				locCngList.Add(cng);
		}
		else
			cng.iPrevSpanDist = cng.iPrevSpanError = 0;


		if (locCngList.GetCount() >= 6)		// may have a whole locator tab
		{
			// check if:
			// latest state is unblocked
			// last 6 changed were same direction - 0 or same
			BYTE iExpState = 0;		// latest state unblocked
			char iDir = 0;
			for (int i = 0; i < 6; i++)
			{
				SLocatorChangeData& cngRecent = locCngList[i];
				if (cngRecent.iNewState != iExpState)		// check correct state
					break;
				iExpState = !iExpState;
				if (cngRecent.iDirection != iDir)
					if (iDir == 0)
						iDir = cngRecent.iDirection;	// set confirmed direction
					else if (iDir * cngRecent.iDirection == -1)	// dir changed
						break;
				if (cngRecent.iPrevSpanDist > m_iLocTabSizeMax && i < 5)
					break;
			}
			if (i == 6 && iDir != 0)		// got locator tab!
			{
				SLocator loc;		// will set to current locator
				loc.numSpans = 5;
				int idxCng = 4;
				int idxEdgeCng = 5;
				if (iDir == -1)			// start with lowest position edge
					idxCng = idxEdgeCng = 0;
				loc.edge[0] = locCngList[idxEdgeCng].iPosMid;
				loc.edge[1] = locCngList[5-idxEdgeCng].iPosMid;
				for (i = 0; i < loc.numSpans; i++)
				{
					loc.span[i] = locCngList[idxCng].iPrevSpanDist;
					idxCng -= iDir;
				}

				LOGEVENT("Writing locator to file");
				ofstream os;
				char* strDir = (iDir > 0) ? "Pos" : "Neg";
				os.open("Locators.txt", ios::app);	// append to file
				os << "Axis " << iAxis << "  Direction " << strDir << endl;
				os << loc;
				os << endl;
				os.close();

				
				SLocator* pLocMatch = MatchAxisLocator(loc, iAxis);
				if (pLocMatch == NULL)
					LOGERROR("No match for detected locator");
				else
				{
					int iPosAdjust = pLocMatch->edge[0] - loc.edge[0];	// take average of edge positions
					iPosAdjust += pLocMatch->edge[1] - loc.edge[1];
					iPosAdjust /= 2;
					// adjust for travel direction
					iPosAdjust += (iDir > 0) ? pLocMatch->offsetPos : pLocMatch->offsetNeg;
					// if not set then set
					if (m_State.GetParamValue(iAxis+1, AP_AxisLocated) == 0)
					{
						SendAxisParam(iAxis+1, AP_PositionW0, LOWORD(iPosAdjust));
						SendAxisParam(iAxis+1, AP_PositionW1, HIWORD(iPosAdjust));	// machine adjusts PosTrack as well
						SendAxisParam(iAxis+1, AP_AxisLocated, 1);
						m_arReferencedLocIdx[iAxis] = pLocMatch->idxLoc;
						m_arReferencedLocDir[iAxis] = iDir;
					}
					else	// else check expected location
					{
						double posAdjust = iPosAdjust * dPosEncoder;
						if (fabs(posAdjust) >= 0.00)
						{
							char str[100];
							sprintf(str, "Axis %i locator %i direction %s out of expected position by %.3f mm", iAxis, pLocMatch->idxLoc, strDir, posAdjust);
							LOGMESSAGE(str);
						}
					}
				}
			}
		}	// if (locCngList.GetCount() >= 6)



	}	// if (iAxis == 0 || iAxis == 1)		// X or Y axis
	else if (iAxis == 2)		// Z axis
	{
		double posRef = -1;
		char idxLoc = -1;
		// locator 1-3 for top, bottom, ceiling respectivly
		// set reference location
		if (iChanged & 0x02)			// bottom changed
			idxLoc = 0;
		else if (iChanged & 0x01)			// top changed
			idxLoc = 1;
		if (iChanged & 0x01 && iState & 0x01)			// top blocked
			posRef = 383.14;
		else if (iChanged & 0x02 && iState & 0x02)	// bottom blocked
			posRef = 16.60;
		int iPosRef = NEARINT(posRef / dPosEncoder);
		int iPosAdjust = iPosRef - iPosChangeMid;		// value to adjust machine axis position

		if (posRef != -1)
			if (m_State.GetParamValue(iAxis+1, AP_AxisLocated) == 0)
			{
				SendAxisParam(iAxis+1, AP_PositionW0, LOWORD(iPosAdjust));
				SendAxisParam(iAxis+1, AP_PositionW1, HIWORD(iPosAdjust));	// machine adjusts PosTrack as well
				SendAxisParam(iAxis+1, AP_AxisLocated, 1);
				m_arReferencedLocIdx[iAxis] = idxLoc;
				m_arReferencedLocDir[iAxis] = iDir;
			}
			else	// else check expected location
			{
				double posAdjust = iPosAdjust * dPosEncoder;
				if (fabs(posAdjust) >= 0.00)
					LOGMESSAGE3("Axis %i locator %i out of expected position by %.3f mm", iAxis, idxLoc, posAdjust);
			}
	}
}

SLocator* CMachineTracker::MatchAxisLocator(SLocator& locFound, int iAxis)
{
	// match the given locator to one of the axis ones to set position

//	double arErrSumSq[MAX_LOCATORS];
	double dPosEncoder = g_Settings.MachParam.dPos;
	const int iErrMaxAllowed = int(0.2 / dPosEncoder);
	double errSumSqMin = 1e6;
	int idxLocBest = -1;
	int numLocators = m_arNumAxisLocators[iAxis];
	int iSpansCheck = locFound.numSpans;
	for (int idxLocTest = 0; idxLocTest < numLocators; idxLocTest++)
	{
		SLocator& locTest = m_MachineLocators[iAxis][idxLocTest];

		// compare locator
		if (locTest.numSpans != iSpansCheck)
			continue;
		int iErrCompound = 0;
		int iErrLargest = 0;
		double errSumSq = 0;
		for (int i = 0; i < iSpansCheck; i++)
		{
			int iErr = locFound.span[i] - locTest.span[i];
			iErrCompound += iErr;
			iErr = abs(iErr);
			double err = iErr * dPosEncoder;
			errSumSq += err*err;
			if (iErr > iErrLargest)
				iErrLargest = iErr;
		}
		errSumSq /= iSpansCheck;		// normalise
//		arErrSumSq[i] = errSumSq;
		if (errSumSq <= errSumSqMin && iErrLargest <= iErrMaxAllowed)
		{
			errSumSqMin = errSumSq;
			idxLocBest = idxLocTest;
		}
	}

	if (idxLocBest != -1)
		return &m_MachineLocators[iAxis][idxLocBest];
	return NULL;
}

void CMachineTracker::StoreLocator(SLocatorFloat& loc)
{
	int iAxis = loc.axis;
	if (iAxis < 0 || iAxis >= NUM_AXIS)
	{
		LOGERROR("Invalid locator axis");
		return;
	}
	char idxLoc = m_arNumAxisLocators[iAxis]++;
	if (idxLoc >= MAX_LOCATORS)
	{
		LOGERROR("Too many locators on axis");
		m_arNumAxisLocators[iAxis]--;
		return;
	}
	SLocator& locInt = m_MachineLocators[iAxis][idxLoc];
	loc.GetLocatorInt(locInt);
	locInt.idxLoc = idxLoc;
	int sumSpans = locInt.SumSpans();
	int edgeDiff = locInt.edge[1] - locInt.edge[0];
	if (locInt.SumSpans() != locInt.width)
		LOGERROR("Bad locator width");
	if (locInt.edge[1] - locInt.edge[0] != locInt.width)
		LOGERROR("Bad locator edge difference");

//	TRACE1("storing locator on axis %i\n", iAxis);
//	ofstream os("MachineNew.loc", ios::app);
//	os << locInt << endl;
}

void CMachineTracker::LoadLocatorData()
{
	// load axis locator data
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		m_arNumAxisLocators[ax] = 0;
		m_arLocatorOrigin[ax] = 0;
	}

	// set file name
	char szFile[MAX_PATH];
	strcpy(szFile, AfxGetApp()->m_pszProfileName);
	char* pBackSlash = strrchr(szFile, '\\');
	if (pBackSlash == NULL)
		pBackSlash = szFile;
	else
		pBackSlash++;
	strcpy(pBackSlash, "Machine.loc");

	ifstream is;
	is.open(szFile, (ios::in | ios::nocreate));
	if (is.fail())
	{
		LOGERROR("Couldn't open locator data");
		return;
	}


	enum KEYS
	{
		KEY_AXIS,
		KEY_OSPOS,
		KEY_OSNEG,
		KEY_EDGE0,
		KEY_TABS,
		KEY_GAPS,
		KEY_WIDTH,
		KEY_EDGE1,
		KEY_ORIGIN,
		KEY_MAX,

		KEY_ALLLOC = (2<<KEY_EDGE1)-1,
	};
	// lowercase key words
	char* m_arKeys[KEY_MAX];
	m_arKeys[KEY_AXIS]	= "axis";
	m_arKeys[KEY_OSPOS]	= "offset pos";
	m_arKeys[KEY_OSNEG]	= "offset neg";
	m_arKeys[KEY_EDGE0]	= "edge0";
	m_arKeys[KEY_EDGE1]	= "edge1";
	m_arKeys[KEY_TABS]	= "tabs";
	m_arKeys[KEY_GAPS]	= "gaps";
	m_arKeys[KEY_WIDTH]	= "width";
	m_arKeys[KEY_ORIGIN]	= "origin";

	char szLine[100];
	SLocatorFloat loc;
	int nKeyExp = KEY_AXIS;
	int iLine = 0;
	int flagsSet = 0;

	for (;;)		// scan through lines
	{
		if (is.eof())
			break;
		is.getline(szLine, sizeof(szLine));
		iLine++;
		char* pLine = szLine;
		skipWS(pLine);
		if (pLine[0] == 0)
			continue;				// blank line
		strlwr(pLine);					// set to lowercase

		// check for a key word
		if (nKeyExp >= KEY_MAX)
			nKeyExp = 0;
		int nKey = nKeyExp;
		for (;;)
		{
			int idxEnd = strcmpsub(pLine, m_arKeys[nKey]);			// returns index at end of name or 0 if not found
			if (idxEnd != 0)
			{
				pLine += idxEnd;
				break;
			}
			if (++nKey >= KEY_MAX)
				nKey = 0;
			if (nKey == nKeyExp)		// got back to start without matching
			{
				LOGERROR("Bad keyword");
				continue;
			}
		}

		skipWS(pLine);
		bool bOK = false;
		bool bClearSetFlags = false;
		nKeyExp = nKey + 1;
		int flag = 1 << nKey;

		switch (nKey)
		{
		case KEY_AXIS:
			bOK = sscanf(pLine, "%i", &loc.axis) == 1;
			if (loc.axis < 0 || loc.axis >= NUM_AXIS)
				bOK = false;
			break;
		case KEY_OSPOS:
			bOK = sscanf(pLine, "%lf", &loc.offsetPos) == 1;
			break;
		case KEY_OSNEG:
			bOK = sscanf(pLine, "%lf", &loc.offsetNeg) == 1;
			break;
		case KEY_EDGE0:
			bOK = sscanf(pLine, "%lf", &loc.edge[0]) == 1;
			break;
		case KEY_TABS:
			bOK = sscanf(pLine, "%lf %lf %lf", &loc.span[0], &loc.span[2], &loc.span[4]) == 3;
			loc.numSpans = 5;
			break;
		case KEY_GAPS:
			bOK = sscanf(pLine, "%lf %lf", &loc.span[1], &loc.span[3]) == 2;
			break;
		case KEY_WIDTH:
			bOK = sscanf(pLine, "%lf", &loc.width) == 1;
			break;
		case KEY_EDGE1:
			bOK = sscanf(pLine, "%lf", &loc.edge[1]) == 1;
			nKeyExp = KEY_AXIS;
			break;
		case KEY_ORIGIN:
			if (flagsSet & (1<<KEY_AXIS))
				bOK = sscanf(pLine, "%lf", &m_arLocatorOrigin[loc.axis]) == 1;
			bClearSetFlags = true;
			nKeyExp = KEY_AXIS;
			break;
		default:
			bOK = false;;
		}

		if (bOK)
		{
			if (flagsSet & flag)
				LOGERROR("Already got value");
			flagsSet |= flag;
		}
		else
			LOGERROR("Invalid value");

		// store if got all parts
		if (flagsSet == KEY_ALLLOC)
		{
			StoreLocator(loc);
			bClearSetFlags = true;
		}
		if (bClearSetFlags)
			flagsSet = 0;

	}	// for (;;)
	is.close();

	// offset axis locators
	for (ax = 0; ax < NUM_AXIS; ax++)
		for (int i = 0; i < m_arNumAxisLocators[ax]; i++)
			m_MachineLocators[ax][i].OffsetBy(m_arLocatorOrigin[ax]);

}

void CMachineTracker::OnRxIndexChange(CPacket& pkt)
{
/*
	      Edge0   Edge1
	        v_______v
	        |       |
	________|       |___________
	   ^-dPos-^
	         Pos
	


*/
	int iAxis = 0;

	int idPos;
	int iPos;
	int iPosPrev = iPos - idPos;
	int iPosMin;
	int iPosMax;
		
	int iNewState;		// 0 or 1
	int iEdge;			// 0 or 1
	int idxDir;

	if (idPos > 0)
	{
		idxDir = 1;
		iEdge = iNewState == 0;
		iPosMin = iPosPrev;
		iPosMax = iPos;
	}
	else if (idPos < 0)
	{
		idxDir = 0;
		iEdge = iNewState != 0;
		iPosMin = iPos;
		iPosMax = iPosPrev;
	}
	else
	{
		iEdge = -1;			// can't tell direction - ignore
		return;
	}

	int  iPosEdgeMin[2][2][NUM_AXIS];		// [edge][dir][axis]		edge = 0,1  dir = 0:neg,1:pos
	int  iPosEdgeMax[2][2][NUM_AXIS];		// [edge][dir][axis]		edge = 0,1  dir = 0:neg,1:pos
	bool bPosEdgeSet[2][2][NUM_AXIS];		// [edge][dir][axis]		edge = 0,1  dir = 0:neg,1:pos

	if (bPosEdgeSet[iEdge][idxDir][iAxis])
	{
		// check regions intersect!
		int iPosMinNew = max(iPosEdgeMin[iEdge][idxDir][iAxis], iPosMin);
		int iPosMaxNew = min(iPosEdgeMax[iEdge][idxDir][iAxis], iPosMax);
		if (iPosMinNew <= iPosMaxNew)	// ok
		{
			iPosEdgeMin[iEdge][idxDir][iAxis] = iPosMinNew;
			iPosEdgeMax[iEdge][idxDir][iAxis] = iPosMaxNew;
		}
		else
			LOGERROR("Index change position");
	}
	else
	{
		bPosEdgeSet[iEdge][idxDir][iAxis] = true;
		iPosEdgeMin[iEdge][idxDir][iAxis] = iPosMin;
		iPosEdgeMax[iEdge][idxDir][iAxis] = iPosMax;
	}


	int iExpPosEdge0;
	int iExpPosEdge1;



}

void CMachineTracker::OnRxSampleData(CPacket& pkt)
{
	int iLen = pkt.GetStreamDataLen() - 1;		// number of data bytes
	BYTE* pData = pkt.GetStreamData();
	int iSampleCount = pData[0];
//	if (((iSampleCount - m_iExpSampleCount) & 0xff) != 0)
//		LOGERROR("Sample count mismatch");

	int iNumValues = iLen / m_iSampleValueBytes;
	if (iNumValues * m_iSampleValueBytes != iLen)		// check if bytes match
		LOGERROR("Sample value byte mismatch");

	int arVal[16];
	if (iNumValues > 16)
		iNumValues = 16;
	pData++;
	int idxData = 0;
	for (int i = 0; i < iNumValues; i++)
	{
		switch (m_iSampleValueBytes)
		{
		case 1:
			arVal[i] = *(char*)(pData + idxData);
			break;
		case 2:
			arVal[i] = *(short*)(pData + idxData);
			break;
		case 3:
			arVal[i] = (*(int*)(pData + idxData)) & 0x00ffffff;		// take 3 bytes
			if (arVal[i] & 0x00800000)				// if negative 24 bit value
				arVal[i] |= 0xff000000;				// sign extend 24 bit value
			break;
		case 4:
			arVal[i] = *(int*)(pData + idxData);
			break;
		}
		idxData += m_iSampleValueBytes;
	}
	for (i = 0; i < iNumValues; i++)
		m_SampleValuesArray.Add(arVal[i]);
}



void CMachineTracker::OnRxMotionUpdate(CPacket& pkt)
{
	int iLen = pkt.GetStreamDataLen();		// number of data bytes
	BYTE* pData = pkt.GetStreamData();

	int iTimeNew = m_State.iTime;
	unsigned short iTime = *(short*)(pData+0);				// take 2 bytes
	unsigned short iTimePrev = (short)iTimeNew;
	*(short*)&iTimeNew = iTime;
	if (iTime < iTimePrev)			// unsigned compare
		iTimeNew += 0x10000;
	m_State.iTime = iTimeNew;			// change in one go so thread safe, step time is reset on start path!
	


	int iParamCode = pData[2];
	int arVal[NUM_AXIS];
	pData += 3;
	int iValueBytes = (iLen - 3) / 3;
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		switch (iValueBytes)
		{
		case 1:
			arVal[ax] = *(char*)pData;
			break;
		case 2:
			arVal[ax] = *(short*)pData;
			break;
		case 3:
			arVal[ax] = (*(int*)pData) & 0x00ffffff;		// take 3 bytes
			if (arVal[ax] & 0x00800000)				// if negative 24 bit value
				arVal[ax] |= 0xff000000;				// sign extend 24 bit value
			break;
		case 4:
			arVal[ax] = *(int*)pData;
			break;
		}
		pData += iValueBytes;
	}

	for (ax = 0; ax < NUM_AXIS; ax++)
		switch (iParamCode)
		{
		case AP_PosTrackW0:
			m_State.iPosTrack[ax] = (__int64)arVal[ax] * m_iPosTrackOffset;
			break;
		case AP_VelTrackW0:
			m_State.iVelTrack[ax] = arVal[ax] * m_iVelTrackOffset;
			break;
		case AP_PositionW0:
			m_State.iPosServo[ax] = arVal[ax] * m_iPosOffset;
			break;
		case AP_Velocity:
			m_State.iVelServo[ax] = arVal[ax] * m_iVelOffset;
			break;
		case AP_PosErrorW0:
			m_State.iPosError[ax] = arVal[ax] * m_iPosErrOffset;
			break;
		}

	switch (iParamCode)
	{
	case AP_PosTrackW0:
		m_State.OnUpdatePosTrack();
		break;
	case AP_PositionW0:
		m_State.OnUpdatePosServo();
		break;
	};

	m_bMachineStateChanged = true;

}

void CMachineTracker::OnRxProbeSample(CPacket& pkt)
{
	int iLen = pkt.GetStreamDataLen();		// number of data bytes
	BYTE* pData = pkt.GetStreamData();
	if (iLen != 13)
		LOGERROR1("Probe sample packet data length %i (not 13)", iLen);

	int iProbeInfo = *(BYTE*)pData;
	bool bProbeError = (iProbeInfo & 0x80) != 0;
	pData += 1;
	int iProbeRelPosSampleStep = *(char*)pData;
	pData += 1;
	int iProbeZSampleValue = *(short*)pData;
	pData += 2;
//	m_State.iProbeZ = iProbeZSampleValue;
	for (int ax = 0; ax < NUM_AXIS; ax++)
	{
		int iPos = (*(int*)pData) & 0x00ffffff;		// take 3 bytes
		if (iPos & 0x00800000)				// if negative 24 bit value
			iPos |= 0xff000000;				// sign extend 24 bit value
		pData += 3;
		m_State.iPosServo[ax] = iPos;
	}
	m_State.OnUpdatePosServo();

	static int iErrorCount = 0;
	static int iErrorResultCodeLast = -1;
	int iErrorResultCode = -1;
	if (bProbeError)
	{
		iErrorCount++;
		int iErrorFlags = (BYTE)iProbeInfo;
		iErrorResultCode = (BYTE)iProbeRelPosSampleStep;
		if (iErrorResultCode == 2)		// no response
		{
			if (iErrorResultCodeLast != iErrorResultCode)
				LOGMESSAGE("No Response from Probe - Probe On?");
		}
		else
			LOGERROR2("Probe Error flags 0x%02x result code 0x%02x", (BYTE)iErrorFlags, (BYTE)iErrorResultCode);
	}
	iErrorResultCodeLast = iErrorResultCode;


	// check data
	bool bDataError = false;
	double ProbeZ;
	if (!bProbeError)
	{
		if (iProbeZSampleValue == 29999)
			LOGERROR("Probe error value without error flag");
		if (iProbeRelPosSampleStep < -3 || iProbeRelPosSampleStep > 3)
			LOGMESSAGE2("Probe Relative Pos Sample = %i ms (%i errors)", iProbeRelPosSampleStep, iErrorCount);
		int iProbeUnits = (iProbeInfo & 0xf0) >> 4;		// 0 = mm, 1 = inch
		int iProbeDecPlace = iProbeInfo & 0x0f;			// 2->5

		ProbeZ = iProbeZSampleValue;
		switch (iProbeDecPlace)
		{
		case 2:
			ProbeZ *= 1e-2;
			break;
		case 3:
			ProbeZ *= 1e-3;
			break;
		case 4:
			ProbeZ *= 1e-4;
			break;
		case 5:
			ProbeZ *= 1e-5;
			break;
		default:
			LOGMESSAGE1("Invalid Probe Decimal Place %i", iProbeDecPlace);
			bDataError = true;
		}

		switch (iProbeUnits)
		{
		case 0:			// mm
			if (iProbeDecPlace < 2 || iProbeDecPlace > 3)
				LOGMESSAGE1("Unexpected mm Probe Decimal Place %i", iProbeDecPlace);
			break;
		case 1:			// inches
			ProbeZ *= 25.4;			// convert inch to mm
			if (iProbeDecPlace < 4 || iProbeDecPlace > 5)
				LOGMESSAGE1("Unexpected Inch Probe Decimal Place %i", iProbeDecPlace);
			break;
		default:
			LOGMESSAGE1("Invalid Probe Units %i", iProbeUnits);
			bDataError = true;
		}
	}

	bool bError = bProbeError || bDataError;
	m_State.m_bProbeReadError = bError;
	if (!bError)
	{
		m_State.ProbeZ = ProbeZ;
		// set to get probe tip pos instead of tool
		m_ProbeSampler.StorePoint();
	}

}

void CMachineTracker::CheckPathStage()		// done regularly to check on path processing
{
	//CPacket pkt;
	int iFree;
	switch (m_iPathSendStage)
	{
	case 0:		// not sending path
		return;
	case 1:		// request buffer free space (and size?)

		SendParam(GP_PathReset, 1);
		
		ReqParam(GP_PathBuffMaxObjs);
		ReqParam(GP_PathBuffFreeObjs);
		m_iPathSendStage++;
		break;
	case 2:		// awaiting buffer free response
		if (!m_State.IsReceived(GP_PathBuffFreeObjs) || !m_State.IsReceived(GP_PathBuffMaxObjs))		// not received yet!
			break;
		iFree = m_State.GetParamValue(GP_PathBuffFreeObjs);
		if (iFree < 100)
		{
			LOGERROR("Not enough machine buffer space to start!");
			m_iPathSendStage = 0;
			break;
		}
		m_pPktControl->SendPathSegsUpTo(iFree - 2);	// -2 shouldn't be needed
		m_iPathSendStage++;
		break;
	case 3:		// send some segs to machine buffer, then request num in machine buffer
		if ((m_pPktControl->GetPathSegNum() >= 500)			// was 500
			|| (m_pPktControl->GetPathSegEndTime() >= 10*1000)		// 10 secs
			|| (m_pPktControl->IsPathFinished()) )
			;
		else
			break;
		m_State.ParamValueSent(GP_PathBuffNumObjs) = (short)m_pPktControl->GetPathSegNum();	// used to store expected minimum value
		ReqParam(GP_PathBuffNumObjs);
		m_iPathSendStage++;
		break;
	case 4:		// check machine buffer reports the segs, then set machine to process path
		if (!m_State.IsReceived(GP_PathBuffNumObjs))		// not received yet!
			break;
		if (m_State.GetParamValue(GP_PathBuffNumObjs) < m_State.ParamValueSent(GP_PathBuffNumObjs))
		{
			LOGERROR("Reported low num path objects");		// should report correct num in buffer
			break; 
		}
		SendParam(GP_PathProcess, 1);		// set to 1 to process path
		SetPathStartTime();
		m_iPathSendStage++;
		break;
	case 6:		// sent batch and requested free, awaiting response
		ASSERT(!m_pPktControl->SendingPath());
		iFree = -1;
		if (m_State.IsReceived(GP_PathBuffFreeObjs))
			iFree = m_State.GetParamValue(GP_PathBuffFreeObjs);
		if (iFree >= 20)
		{
			m_pPktControl->SendPathSegsUpTo(iFree - 2);	// -2 shouldn't be needed
			m_iPathSendStage--;
			break;
		}
		// not woth sending more, check later or when notified by machine
		if (GetTickCount() - m_iTimeBuffFreeReq < 1000)
			break;		// this check done after timeout check
		m_iPathSendStage--;			// been > 1 sec so request free space again
		// got onto previous stage
	case 5:		// sending path, check free when batch done
		if (m_pPktControl->SendingPath())		// finished sending path batch - check free space on machine buffer
			break;
		ReqParam(GP_PathBuffFreeObjs);
		m_iTimeBuffFreeReq = GetTickCount();
		m_iPathSendStage++;
		break;


	case 100:		// path stop initiated
		SendParam(GP_PathProcess, 0);		// set to 0 to stop path
		m_iPathSendStage++;
		break;
	case 101:		// waiting for path stop confirmation
		if (!m_State.IsReceivedSent(GP_PathProcess))		// not received yet!
			break;
		if (m_State.GetParamValue(GP_PathProcess) == 0)		// confirmed
		{
			SendParam(GP_PathReset, 1);		// set to reset path buffer
			m_iPathSendStage = 0;			// finalised
		}
		break;
	default:
		ASSERT(0);
	}	// switch (m_iPathSendStage)


	// set machine path status
	m_State.PathBuff.Init.Time = GetPathTime() * 1e-3;	// local guess of machine time!
	m_State.PathBuff.Init.AccSeg = 0;
	m_State.PathBuff.Final.Time = m_pPktControl->GetPathSegEndTime() * 1e-3;
	m_State.PathBuff.Final.AccSeg = m_pPktControl->GetPathSegNum();

}



bool CMachineTracker::SetParamValueFrom(CPacket& pkt)
{
	CAxPar axPar = pkt.GetAxPar();
	short iVal = pkt.GetValue();
	if (pkt.IsRequest())
		m_State.SetReceivedRequ(axPar, iVal);
	else		// was sent
		m_State.SetReceivedSent(axPar, iVal);

	int iParam = axPar.param;
	if (axPar.IsGeneralParam())
	{
		switch (iParam)
		{
		case GP_PathBuffNumObjs:
			if (m_State.IsReceived(GP_PathBuffMaxObjs))
				m_State.SetReceivedRequ(GP_PathBuffFreeObjs, m_State.GetParamValue(GP_PathBuffMaxObjs) - iVal);
			break;
		case GP_PathBuffFreeObjs:
			if (m_State.IsReceived(GP_PathBuffMaxObjs))
				m_State.SetReceivedRequ(GP_PathBuffNumObjs, m_State.GetParamValue(GP_PathBuffMaxObjs) - iVal);
			break;
		case GP_PathProcess:				// used to signal end
			if (pkt.IsInitiated() && pkt.IsSend())		// if sent to signal end
				OnMachineFinishedPath();				// notify of path finished, iVal = 0x10 if normal finish
			break;
		}
	}
	else	// is Axis param
	{
		switch (iParam)
		{
		case AP_IndexCngPosNotify:
			OnRxIndexChange(pkt);
			break;
		case AP_IndexCngdPosNotify:
			OnRxIndexChange(pkt);
			break;
		}
	}

	m_pParamDoc->SetRxParamValue(axPar, iVal);
	m_pUserThread->PostThreadMessage(WMU_UPDATEPARAM, axPar, 0);	
	return true;
}

bool CMachineTracker::HandleDataPacket(CPacket& pkt)
{
	switch (pkt.GetDataType())
	{
	case P1DATA_PARAM:			// standard parameter packet
		HandleParamPacket(pkt);
		break;
	case P1DATA_REGISTER:
	case P1DATA_REGISTER2:
		HandleRegPacket(pkt);
		break;
	case P1DATA_PATHCONF:
		HandlePathConfPacket(pkt);
		break;
	case P1DATA_MEMORY:
		HandleMemPacket(pkt);
		break;
	case P1DATA_CONTROLLERERROR:
		HandleControllerErrorPacket(pkt);
		break;
	case P1DATA_COMMSERROR:
		HandleCommsErrorPacket(pkt);
		break;
	}
	return true;
}

bool CMachineTracker::HandlePathPacket(CPacket& pkt)
{
	// initiated or reply
	if (pkt.IsSend())
	{
		switch (pkt.GetPathType())
		{
		case P1PATH_POS:
			pkt.GetPathValues(m_State.iPosServo);
			break;
		case P1PATH_VEL:
			pkt.GetPathValues(m_State.iVelServo);
			break;
		}
// done in RunMonitor()		NotifyUserThread(CN_UPDATE_MACHINESTATEINFO, (int)&m_State);	// notify of new machine state
	}
	return true;
}

bool CMachineTracker::HandleStreamPacket(CPacket& pkt)
{
	// whether initiated or reply
	switch (pkt.GetStreamType())
	{
	case P1STREAM_CONTDESCR:
		OnRxControllerDescription(pkt);
		break;
	case P1STREAM_LOCCHANGE:
		OnRxLocatorChangeState(pkt);		
		break;
	case P1STREAM_SAMPLEDATA:
		OnRxSampleData(pkt);
		break;
	case P1STREAM_MOTIONUPDATE:
		OnRxMotionUpdate(pkt);
		break;
	case P1STREAM_PROBESAMPLE:
		OnRxProbeSample(pkt);
		break;
	default:
		LOGMESSAGE1("Received unhandled stream packet type %i", pkt.GetStreamType());
		break;
	}
	return true;
}


//----------------------------------------------
// Handle received packet functions

bool CMachineTracker::ConfirmSentValueFrom(CPacket& pkt)
{
	CAxPar axpar;
	if (pkt.GetSizeType() == P0SIZE_PATH)
		return true;
	if (pkt.GetSizeType() == P0SIZE_DATA)
	{
		switch (pkt.GetDataType())
		{
		case P1DATA_PARAM:			// standard parameter packet
			axpar = pkt.GetAxPar();
			m_pParamDoc->SetRxParamValue(axpar, pkt.GetValue());	// use local thread copy of machine parameters
	//		m_pParamDoc->SetRxConfirmParamValue(axpar);		// if value not available
			m_pUserThread->PostThreadMessage(WMU_UPDATEPARAM, axpar, 0);	
			break;
		default:
			return false;
		}
		return true;
	}
	if (pkt.GetSizeType() == P0SIZE_BYTE)
		return true;
	if (pkt.GetSizeType() == P0SIZE_STREAM)
		return true;
	return false;
}

bool CMachineTracker::HandleParamPacket(CPacket& pkt)
{
	if (pkt.IsSend())
		SetParamValueFrom(pkt);	// if diff to sent value notify user thread
	else		// was request
		SetParamValueFrom(pkt);
	return true;
}

bool CMachineTracker::HandleControllerErrorPacket(CPacket& pkt)
{
//	Notification of Controller Error (p2=Last Error, p3=Last Error Axis, p4=Error Count(low byte))
	int iErrorCount;
	BYTE* arData = pkt.GetData();
	m_pParamDoc->GetParamValue(GP_ErrorCount, &iErrorCount);
	iErrorCount += BYTE(arData[4] - iErrorCount);			// packet has just low byte
	m_pParamDoc->SetRxParamValue(GP_ErrorCount, iErrorCount);
	m_pParamDoc->SetRxParamValue(GP_ErrorLast, arData[2]);
	m_pParamDoc->SetRxParamValue(GP_ErrorAxisLast, arData[3]);
	m_pUserThread->PostThreadMessage(WMU_UPDATEPARAM, GP_ErrorLast, 0);
	m_pUserThread->PostThreadMessage(WMU_UPDATEPARAM, GP_ErrorAxisLast, 0);
	m_pUserThread->PostThreadMessage(WMU_UPDATEPARAM, GP_ErrorCount, 0);

	char* strError = m_pParamDoc->GetErrorDescription(arData[2]);
	LOGERROR1("Machine Error: %s", strError);

	// update enable states
	ReqParam(GP_MachineDisable);
	ReqAxisParamAllAxes(AP_AxisDisable);

	// get latest path locatation for reference
	ReqParam(GP_PathCurrSegNum);
	ReqParam(GP_PathCurrTimeW0);
	ReqParam(GP_PathCurrTimeW1);

	return true;
}

bool CMachineTracker::HandleCommsErrorPacket(CPacket& pkt)
{
//	Error Code, Received Header, Error Info
	ASSERT(pkt.IsCommsErrorPacket());
	switch (pkt.GetErrorNumber())
	{
	case Error_RxPacketID:
		// PC transmitted wrong Packet ID, expected value is returned in .value1
		if (m_iInitContactStage != 0)
			m_pPktControl->SetNextTxID(pkt.value1);
		else
		{
			LOGMESSAGE("Tx ID was not what Controller expected. Aligning Tx ID with Controller");
			m_pPktControl->SetNextTxID(pkt.value1);
		}
		// resend previous packet
		break;
	case Error_PathBufferFull:
		pkt.GetValue();
		break;
	}
	return true;
}

bool CMachineTracker::HandlePathConfPacket(CPacket& /*pkt*/)
{
/*
;	Header	(Standard, Send, Reply)
;		0x80-0x8f	- [data confirmation] - low nibble for Path Time<8:11>
;		Data Operation Result - see below
;		Step Type/Count	- Received Step Type, next expected Step Count
;		Path Time	- Cumulative time of path so far (incremented if data stored)
;	Checksum
;
;	Data Operation Result:
;		7 6 5 4 3 2 1 0
;		        \_____/ - 0-15 room left on path buffer (15 = 15 or more)
;		  \___/         - Operation result status
;		|               - 1 = Error: Data wasn't stored
;
;	bits<4:6>
;	0 = OK, data stored
;	1 = buffer was full, data wasn't stored
;	2 = step count mismatch between received data and controller, data wasn't stored
;	7 = got end path data
*/

	return true;
}

bool CMachineTracker::HandleMemPacket(CPacket& pkt)
{
	int iBytes = 1;
	if (pkt.inst0 & 0x01)
		iBytes = 2;				// is a two byte value
	if (pkt.inst0 & 0x02)
		iBytes |= 0x10;		// address was incremented
	int iValue = MAKEWORD(pkt.value0, pkt.value1);
	int iAddr = pkt.inst1;
	m_pUserThread->PostThreadMessage(WMU_MEMVALUE, iAddr, MAKELONG(iBytes, iValue));	
//		QueueSendMem(int iAddr, int iBytes, int iValue)
//		QueueSendMem(msg.wParam, LOWORD(msg.lParam), HIWORD(msg.lParam));
	return true;
}

bool CMachineTracker::HandleRegPacket(CPacket& pkt)
{
	int iBytes = 1;
	if (pkt.inst0 & 0x10)
		iBytes = 2;				// is a two byte value
	int iValue = MAKEWORD(pkt.value0, pkt.value1);
	int iReg = MAKEWORD(pkt.inst1, pkt.inst0 & 0x07);
	m_pUserThread->PostThreadMessage(WMU_REGVALUE, iReg, MAKELONG(iBytes, iValue));	
	return true;
}





void CMachineTracker::InitContact()
{
/*
	Send NOP byte packet
	wait up to 100ms for response
	if no response repeat up to 10 times then abort
	if OK response repeat till 6 OK responses
	if error response (ID error) correct, clear OK count, repeat up to 10 times


*/
	m_iInitContactStage = 1;

	m_iInitContactOKCount = 0;
	m_iInitContactNoRespCount = 0;
	m_iInitContactErrorRespCount = 0;
}

void CMachineTracker::CheckInitContact()
{
	if (m_iInitContactStage == 0)
		return;
	CPacket pkt;
	bool bRepeat;
	do
	{
		bRepeat = false;
		switch (m_iInitContactStage)
		{
		case 1:				// send blanks
			pkt.SendNOP();
			QueuePacket(pkt);
			m_iTimeTxContactPkt = GetTickCount();
			m_iRxPktCount = 0;
			m_iInitContactStage++;
			break;

		case 2:				// check for a valid response
			if (m_iRxPktCount >= 1)
			{
				if (++m_iInitContactOKCount >= 6)
				{
					bRepeat = true;
					m_iInitContactStage++;			// initial contact successful
				}
				else
				{
					bRepeat = true;
					m_iInitContactStage = 1;			// repeat send
				}
				break;
			}

			// no response so check for timeout
			if (GetTickCount() - m_iTimeTxContactPkt > 100)
			{
				m_iInitContactOKCount = 0;
				if (++m_iInitContactNoRespCount >= 10)
					m_iInitContactStage = 0;		// abort initial contact
				else
				{
					bRepeat = true;
					m_iInitContactStage = 1;			// repeat send
				}
			}
			break;

		case 3:			// request some settings
			SendParam(GP_SendMotionUpdates, 0);			// Stop updates while requesting
			SendParam(GP_SendProbeSamples, 0);			// Stop updates while requesting

			ReqAxisParamAllAxes(AP_AxisLocated);
			ReqParam(GP_MachineDisable);
			ReqAxisParamAllAxes(AP_AxisDisable);

			SendParam(GP_PathReset, 1);

			m_iInitContactStage++;
			break;

		case 4:
			m_iInitContactStage = 0;		// finished
			break;
		}
	} while (bRepeat);

}



///////////////////////////
// path functions
///////////////////////////

void CMachineTracker::SetStandardParameters()
{
	// set due to current state
//	SendAxisParam(3, AP_NoPositionFeedback, 1);		// not connected yet


	SendAxisParam(1, AP_NegateDirection, 1);		// reverse X direction

	// set control parameters
	SendAxisParam(1, AP_FFVoltPerDir, 400);
	SendAxisParam(2, AP_FFVoltPerDir, 400);
	SendAxisParam(3, AP_FFVoltPerDir, 400);

	SendAxisParam(1, AP_FFVoltPerVel, 1500);
	SendAxisParam(2, AP_FFVoltPerVel, 1400);
	SendAxisParam(3, AP_FFVoltPerVel, 2000);

	SendAxisParam(1, AP_FFVoltPerAcc, 300);
	SendAxisParam(2, AP_FFVoltPerAcc, 300);
	SendAxisParam(3, AP_FFVoltPerAcc, 400);

	SendAxisParam(1, AP_FFVoltPerdAcc, 2000);
	SendAxisParam(2, AP_FFVoltPerdAcc, 2000);
	SendAxisParam(3, AP_FFVoltPerdAcc, 3000);

	SendAxisParam(1, AP_PIDp, 15000);	// was 10000
	SendAxisParam(2, AP_PIDp, 15000);	// was 10000
	SendAxisParam(3, AP_PIDp, 22000);	// was 15000

	SendAxisParam(1, AP_PIDi, 10000);	// was 5000
	SendAxisParam(2, AP_PIDi, 10000);	// was 5000
	SendAxisParam(3, AP_PIDi, 15000);	// was 7500

	SendAxisParam(1, AP_PIDd, 50000);	// was 0
	SendAxisParam(2, AP_PIDd, 50000);	// was 0
	SendAxisParam(3, AP_PIDd, 60000);	// was 0

	// set limits
/*	X axis absolute:
	adjust: 0
	Min:   -4.0	shield roller hits end
	Max: 1840.5	shield PTFE scraper hits end Al shield fastening

	Min Locator final block:  ?	block dist to abs end = 27.3
	Max Locator final block:  ?	block dist to abs end = 31.1

	Limit positions:      4.0mm short of absolute ends [0 -> 1836.5]
	Locator block limits: 3.0mm short of absolute ends

----------------

	Y axis absolute:
	adjust: 0
	Min:  -2.0	trolley hits end
	Max: 853.2	nut hits hole, bolt head 0.5mm off hitting

	Min Locator final block:  42.6	block dist to abs end = 44.6
	Max Locator final block: 811.1	block dist to abs end = 42.1

	Limit positions:      2.0mm short of absolute ends [0 -> 851.2]
	Locator block limits: 1.5mm short of absolute ends

----------------

	Z axis absolute:
	adjust: 0
	Min: -2.0	Top bracket hit bolt heads
	Max: ~470

	Min Locator final block: 	block dist to abs end = 18.6
	Max Locator final block: 	block dist to abs end = ?

	Limit positions:      2.0mm short of absolute ends
	Locator block limits: 1.5mm short of absolute ends
*/

	double dPosX256 = g_Settings.MachParam.dPos * 256;		// skips low byte = 2.5/16
	double dPosMinMax = dPosX256;									// skips low byte = 2.5/16
	SendAxisParam(1, AP_PositionMin, int(    0.0 / dPosMinMax));
	SendAxisParam(1, AP_PositionMax, int( 1836.5 / dPosMinMax));
	SendAxisParam(2, AP_PositionMin, int(    0.0 / dPosMinMax));
	SendAxisParam(2, AP_PositionMax, int(  851.2 / dPosMinMax));
	SendAxisParam(3, AP_PositionMin, int(    0.0 / dPosMinMax));
	SendAxisParam(3, AP_PositionMax, int(  400.0 / dPosMinMax));

	SendAxisParam(1, AP_LocatorBlockDistNeg, int((27.3-3.0) / dPosX256));	// trigger 3.0mm short of abs end. Skips low byte
	SendAxisParam(1, AP_LocatorBlockDistPos, int((31.1-3.0) / dPosX256));
	SendAxisParam(2, AP_LocatorBlockDistNeg, int((44.6-1.5) / dPosX256));	// trigger 1.5mm short of abs end. Skips low byte
	SendAxisParam(2, AP_LocatorBlockDistPos, int((42.1-1.5) / dPosX256));
	SendAxisParam(3, AP_LocatorBlockDistNeg, int((18.6-1.5) / dPosX256));	// trigger 1.5mm short of abs end. Skips low byte
	SendAxisParam(3, AP_LocatorBlockDistPos, int((41.5-1.5) / dPosX256));


	SendAxisParam(1, AP_PosTrackMin, int(-2000.0 / dPosMinMax));
	SendAxisParam(1, AP_PosTrackMax, int( 2000.0 / dPosMinMax));	// 2000mm
	SendAxisParam(2, AP_PosTrackMin, int(-1000.0 / dPosMinMax));
	SendAxisParam(2, AP_PosTrackMax, int( 1000.0 / dPosMinMax));	// 1000mm
	SendAxisParam(3, AP_PosTrackMin, int(-500.0 / dPosMinMax));
	SendAxisParam(3, AP_PosTrackMax, int( 500.0 / dPosMinMax));		// 500mm

	SendAxisParam(1, AP_VelTrackMin, -0x7000);
	SendAxisParam(1, AP_VelTrackMax,  0x7000);
	SendAxisParam(2, AP_VelTrackMin, -0x7000);
	SendAxisParam(2, AP_VelTrackMax,  0x7000);
	SendAxisParam(3, AP_VelTrackMin, -0x7000);
	SendAxisParam(3, AP_VelTrackMax,  0x7000);

	SendAxisParam(1, AP_AccTrackMin, -0x6000);
	SendAxisParam(1, AP_AccTrackMax,  0x6000);
	SendAxisParam(2, AP_AccTrackMin, -0x6000);
	SendAxisParam(2, AP_AccTrackMax,  0x6000);
	SendAxisParam(3, AP_AccTrackMin, -0x6000);
	SendAxisParam(3, AP_AccTrackMax,  0x6000);

	SendParam(GP_VoltStuckTrig, 30);			// high byte, 127 = full volts
	SendParam(GP_VelX2StuckTrig, 10);		// *0.3mm/s
	SendParam(GP_StuckTimeThreshold, 50);	// *4ms = 200ms

	SendParam(GP_CurrentSimTrig, 40);				// high byte sim current
	SendParam(GP_OverCurrentSimTimeThreshold, 60);	// *4ms = 240ms

	SendParam(GP_OverILimitTimeThreshold, 50);			// *4ms = 200ms

	SendParam(GP_PosErrorLimit, int(0.8 / dPosX256));	// 0.8mm


	// set to required condition
	// set to send 'pos track'
	SendParam(GP_MotionUpdatePeriod, 25);		// * 4ms
	SendParam(GP_SendMotionUpdates, 1);


//	SendParam(GP_MachineDisable, 0);
//	SendAxisParam(1, AP_AxisDisable, 0);
//	SendAxisParam(2, AP_AxisDisable, 0);

}

void CMachineTracker::PathReset()
{
	// reset machine path buffer and location trackers
	// get size of machine path buffer
	// send ? segs or ? seconds of path data to machine buffer
	// send run path signal
	// keep buffer full
	if (m_bProcessingPath)
		return;	
	m_bProcessingPath = false;
	m_iPathSendStage = 0;		
	m_pPktControl->ResetPath();
}

void CMachineTracker::PathSend()
{
	if (m_iPathSendStage == 0)
		m_iPathSendStage = 1;		
	m_bProcessingPath = true;

}

void CMachineTracker::PathStop()
{
	if (!m_bProcessingPath)
		return;	
	m_iPathSendStage = 100;			// initiate path stop
	m_bProcessingPath = false;
	m_pPktControl->StopSendingPath();
}

void CMachineTracker::OnMachineFinishedPath()
{
	m_iPathSendStage = 0;
	m_bProcessingPath = false;
	m_pPktControl->StopSendingPath();
	NotifyUserThread(CN_PATHFINISHED);
}


bool CMachineTracker::MoveToPos()
{
	return false;
}

void CMachineTracker::ChangeVelocity(SAxisDataXcng* pADX)
{
	if (ProcessingPath())		// check not processing path
	{
		pADX->iState = -1;		// not used
		return;
	}

	SSegAccel sa, saHold;
	for (int ax = 0; ax < NUM_AXIS; ax++)
		sa.iAcc[ax] = pADX->iAxisData[ax];	// ramp accel up

	if (pADX->iMethod == PATHSEG_dACCEL)
	{
		sa.nSegType = PATHSEG_dACCEL;
		sa.idTime = (short)pADX->iRampTime;
		SendPathSeg(sa);

		saHold.nSegType = PATHSEG_dACCEL;
		saHold.idTime = (short)pADX->iHoldTime;
		for (ax = 0; ax < NUM_AXIS; ax++)
			saHold.iAcc[ax] = 0;
		SendPathSeg(saHold);

		for (ax = 0; ax < NUM_AXIS; ax++)
			sa.iAcc[ax] = -sa.iAcc[ax];			// then down
		SendPathSeg(sa);
	}
	else if (pADX->iMethod == PATHSEG_ACCEL)
	{
		sa.idTime = (short)pADX->iHoldTime;
		sa.nSegType = PATHSEG_ACCEL;
		SendPathSeg(sa);
	}

	pADX->iState = 1;		// unlock, finished with it
	sa.nSegType = PATHSEG_ENDZEROACCEL;		// maintain velocity
	SendPathSeg(sa);
	SendParam(GP_PathProcess, 1);		// set to 1 to process path
}

void CMachineTracker::SetVelocity(SAxisDataXcng* pADX)
{
	if (ProcessingPath())		// check not processing path
	{
		pADX->iState = -1;		// not used
		return;
	}

	SSegAccel sa;
	sa.nSegType = PATHSEG_STEPTOVEL;
	sa.idTime = 0;
	for (int ax = 0; ax < NUM_AXIS; ax++)
		sa.iAcc[ax] = pADX->iAxisData[ax];	// ramp accel up
	SendPathSeg(sa);

	pADX->iState = 1;		// unlock, finished with it
	sa.nSegType = PATHSEG_ENDZEROACCEL;		// maintain velocity
	SendPathSeg(sa);
	SendParam(GP_PathProcess, 1);		// set to 1 to process path
}

void CMachineTracker::StopMovement()
{
	if (ProcessingPath())		// check not processing path
		return;
//	SendParam(GP_PathProcess, 0);		// set to 1 to process path
	SendParam(GP_StopAllMovementSmooth, 1);
}

void CMachineTracker::DisableMachine()
{
	SendParam(GP_MachineDisable, 1);
}
