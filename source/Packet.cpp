// Packet.cpp: implementation of the CPacket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cnccontrol.h"
#include "Packet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



/*
*****************************
Packet Types:
	BYTE		- 1 byte
	DATA		- 6 bytes
	PATH		- ~10-16 bytes variable
	STREAM	- variable

*****************************
BYTE Packet (1 byte)
	Header	(P0SIZE_BYTE)

*****************************
DATA Packet (6 bytes)
	Header	(P0SIZE_DATA)
	Inst0
	Inst1
	Value0
	Value1
	Checksum

Data Packet types - indicated by high nibble of Inst0
	Parameter
	Register
	Memory
	Error
-------------------
Parameter Data
	Header
	Param Type - axis
	param number
	Value0
	Value1
	Checksum

Error Data
	Header
	Error Type -
	Error number
	[Received Header, ]
	Value1
	Checksum
-------------------


************************
PATH Packet (~10-16 bytes variable)
	Header	(P0SIZE_PATH)
	Path Data Type (b7:4) | Segment Count (b3:0)
	Segment Time (0-255)
	Value Axis 1
	Value Axis 2
	Value Axis 3
	Checksum


************************
STREAM Packet (variable)
	Header	(P0SIZE_STREAM)
	Total number bytes
	Data...
	...
	Checksum

*****************************
*/

//////////////////////////////////////////////////////////////////////
// class CPacket
//////////////////////////////////////////////////////////////////////




int CPacket::m_iPathValueSize = 3;
int CPacket::m_iPathValueMin = -((1 << (CPacket::m_iPathValueSize*8 - 1)) - 1);
int CPacket::m_iPathValueMax =  ((1 << (CPacket::m_iPathValueSize*8 - 1)) - 1);
int CPacket::m_iPathNumAxis = 3;
int CPacket::m_iPathPktSize = CPacket::CalcPathPktSize();
char CPacket::m_strPkt[64];

void CPacket::SetFromData(char* pData, int iLen)
{
	for (int i = 0; i < iLen; i++)
		pktData[i] = pData[i];
	iSize = iLen;
	iStatus = PSTAT_FROMDATA;
}

void CPacket::ByteNOP()
{
	header = P0SIZE_BYTE;
	iSize = 1;
	iStatus = PSTAT_DETAILSET;
}

void CPacket::SendNOP()
{
	header = P0SIZE_BYTE | P0SR_SEND | P0IR_INITIATED;
	iSize = 1;
	iStatus = PSTAT_DETAILSET;
}

void CPacket::SendParam(CAxPar axpar, int iValue)
{
	header = P0SR_SEND | P0SIZE_DATA | P0IR_INITIATED;
	iSize = PKTSIZE_DATA;
	axis = (char)(P1DATA_PARAM | (axpar.axis & P1AXIS_MASK));
	param = axpar.param;
	SetValue(iValue);
	iStatus = PSTAT_DETAILSET;
}

void CPacket::ReqParam(CAxPar axpar, int iValue /*=0*/)
{
	header = P0SR_REQUEST | P0SIZE_DATA | P0IR_INITIATED;
	iSize = PKTSIZE_DATA;
	axis = (char)(P1DATA_PARAM | (axpar.axis & P1AXIS_MASK));
	param = axpar.param;
	SetValue(iValue);
	iStatus = PSTAT_DETAILSET;
}

void CPacket::ReturnError(int iErrorCode, int iValue)
{
	header = P0SR_SEND | P0SIZE_DATA | P0IR_REPLY;
	iSize = PKTSIZE_DATA;
	inst0 = P1DATA_COMMSERROR;
	inst1 = (char)iErrorCode;
	SetValue(iValue);
	iStatus = PSTAT_DETAILSET;
}

void CPacket::SendPath(SSegAccel& sa)
{
	header = P0SR_SEND | P0SIZE_PATH | P0IR_INITIATED;
	iSize = m_iPathPktSize;
	if (sa.nSegType == PATHSEG_dACCEL || sa.nSegType == PATHSEG_ACCEL)
	{
		if (sa.idTime <= 0xff)
			sa.nSegType += 2;			// change type to use count if ramptime bits not needed
	}
	else if (sa.nSegType == PATHSEG_dACCELCOUNT || sa.nSegType == PATHSEG_ACCELCOUNT)
	{
		if (sa.idTime > 0xff)
			sa.nSegType -= 2;			// change type to use ramptime bits if needed
	}

	pktData[1] = (BYTE)((sa.nSegType & 0x0f) << 4);
	if (sa.nSegType == PATHSEG_dACCEL || sa.nSegType == PATHSEG_ACCEL)
	{
		pktData[1] |= ((sa.idTime >> 8) & 0x0f);
		ASSERT(sa.idTime <= 0xfff);
	}
	else
		pktData[1] |= (sa.iSegCount & 0x0f);
	pktData[2] = (BYTE)sa.idTime;
	SetPathValues((int*)sa.iAcc);

	iStatus = PSTAT_DETAILSET;
}

void CPacket::GetSegAccel(SSegAccel& sa) const
{
	ASSERT(GetSizeType() == P0SIZE_PATH);
	ASSERT(IsSend());
	ASSERT(IsInitiated());
	ASSERT(iSize == m_iPathPktSize);
	sa.nSegType = short((pktData[1] & 0xf0) >> 4);
	sa.iSegCount = (pktData[1] & 0x0f);
	sa.idTime = pktData[2];
	GetPathValues((int*)sa.iAcc);
}

void CPacket::SendPathPos()
{
	header = P0SR_SEND | P0SIZE_PATH | P0IR_INITIATED;
	iSize = m_iPathPktSize;
//	inst0 = P1PATH_POS;
	inst1 = 0;
	iStatus = PSTAT_DETAILSET;
}

void CPacket::SendPathVel()
{
	header = P0SR_SEND | P0SIZE_PATH | P0IR_INITIATED;
	iSize = m_iPathPktSize;
//	inst0 = P1PATH_VEL;
	inst1 = 0;
	iStatus = PSTAT_DETAILSET;
}

void CPacket::SetPathValues(const int* arVal)
{
	ASSERT(arVal[0] >= m_iPathValueMin && arVal[0] <= m_iPathValueMax);
	ASSERT(arVal[1] >= m_iPathValueMin && arVal[1] <= m_iPathValueMax);
	ASSERT(arVal[2] >= m_iPathValueMin && arVal[2] <= m_iPathValueMax);
	switch (m_iPathValueSize)		// number of bytes for value
	{
	case 2:
		*(short*)&pktData[3] = (short)arVal[0];
		*(short*)&pktData[5] = (short)arVal[1];
		*(short*)&pktData[7] = (short)arVal[2];
		break;
	case 3:
		*(int*)&pktData[3] = arVal[0];	// any extra bytes are overwritten
		*(int*)&pktData[6] = arVal[1];
		*(int*)&pktData[9] = arVal[2];
		break;
	case 4:
		*(int*)&pktData[3]  = arVal[0];
		*(int*)&pktData[7]  = arVal[1];
		*(int*)&pktData[11] = arVal[2];
		break;
	default:
		ASSERT(0);
	}
}
void CPacket::SetPathValues(const double* arVal, double unit)
{
	double fVal;
	fVal = floor((arVal[0] / unit) + 0.5);
	*(int*)&pktData[3]  = (int)fVal;			// any extra bytes are overwritten
	ASSERT(fVal >= m_iPathValueMin && fVal <= m_iPathValueMax);

	fVal = floor((arVal[1] / unit) + 0.5);
	*(int*)&pktData[3+m_iPathValueSize]  = (int)fVal;			// any extra bytes are overwritten
	ASSERT(fVal >= m_iPathValueMin && fVal <= m_iPathValueMax);

	fVal = floor((arVal[2] / unit) + 0.5);
	*(int*)&pktData[3+2*m_iPathValueSize]  = (int)fVal;			// any extra bytes are overwritten
	ASSERT(fVal >= m_iPathValueMin && fVal <= m_iPathValueMax);
}

void CPacket::GetPathValues(int* arVal) const
{
	switch (m_iPathValueSize)		// number of bytes for value
	{
	case 2:
		arVal[0] = *(short*)&pktData[3];
		arVal[1] = *(short*)&pktData[5];
		arVal[2] = *(short*)&pktData[7];
		break;
	case 3:		// 4th byte of int needs sign extending!
		int val;
		val = *(int*)&pktData[3];
		arVal[0] = (val & 0x00800000) == 0 ? val & 0x00ffffff : val | 0xff000000;
		val = *(int*)&pktData[6];
		arVal[1] = (val & 0x00800000) == 0 ? val & 0x00ffffff : val | 0xff000000;
		val = *(int*)&pktData[9];
		arVal[2] = (val & 0x00800000) == 0 ? val & 0x00ffffff : val | 0xff000000;
		break;
	case 4:
		arVal[0] = *(int*)&pktData[3];
		arVal[1] = *(int*)&pktData[7];
		arVal[2] = *(int*)&pktData[11];
		break;
	default:
		ASSERT(0);
	}
}
void CPacket::GetPathValues(double* arVal, double unit) const
{
	int ariVal[3];
	GetPathValues(ariVal);
	arVal[0] = ariVal[0] * unit;
	arVal[1] = ariVal[1] * unit;
	arVal[2] = ariVal[2] * unit;
}

void CPacket::ReqPath(int iObjNum, int nBase)
{
	header = P0SR_REQUEST | P0SIZE_DATA | P0IR_INITIATED;
	iSize = PKTSIZE_DATA;
	inst0 = 0x60;			// Request data code
	inst1 = 0;
	int iObjPos;
	if (nBase == POB_NEXTTOUSE)
		iObjPos = iObjNum;
	else if (nBase == POB_LASTSENT)
		iObjPos = -iObjNum - 1;
	else
		iObjPos = iObjNum;
	vall = (char)iObjPos;
	valh = (char)(iObjPos >> 8);
	iStatus = PSTAT_DETAILSET;
}

void CPacket::ReqMem(int iAddr, int iBytes)
{
	// low nibble of iBytes is num bytes, bit 0x10 is inc addr bit
	header = P0SR_REQUEST | P0SIZE_DATA | P0IR_INITIATED;
	iSize = PKTSIZE_DATA;
	axis = 0xc0;			// Mem packet code
	if ((iBytes & 0x0f) == 2)
		axis |= 0x01;		// set two byte bit
	if (iBytes & 0x10)
		axis |= 0x02;		// set inc address bit
	param = LOBYTE(iAddr);
	value0 = HIBYTE(iAddr);
	value1 = LOBYTE(HIWORD(iAddr));
	iStatus = PSTAT_DETAILSET;
}

void CPacket::SendMem(int iAddr, int iBytes, int iValue)
{
	header = P0SR_SEND | P0SIZE_DATA | P0IR_INITIATED;
	iSize = PKTSIZE_DATA;
	axis = 0xc0;			// Mem packet code
	if ((iBytes & 0x0f) == 2)
		axis |= 0x01;		// set two byte bit
	if (iBytes & 0x10)
		axis |= 0x02;		// set inc address bit
	param = LOBYTE(iAddr);
	value0 = LOBYTE(iValue);
	if ((axis & 0x01))
		value1 = HIBYTE(iValue);
	else
		value1 = 0;
	iStatus = PSTAT_DETAILSET;
}

void CPacket::ReqReg(int iReg, int iBytes)
{
	// low nibble of iBytes is num bytes, bit 0x10 is inc addr bit
	header = P0SR_REQUEST | P0SIZE_DATA | P0IR_INITIATED;
	iSize = PKTSIZE_DATA;
	inst0 = 0x20;			// Register packet code
	if ((iBytes & 0x0f) == 2)
		inst0 |= 0x10;		// set two byte bit
	inst0 |= HIBYTE(iReg) & 0x07;		// set bank in low nibble
	inst1 = LOBYTE(iReg);
	value0 = 0;
	value1 = 0;
	iStatus = PSTAT_DETAILSET;
}

void CPacket::SendReg(int iReg, int iBytes, int iValue)
{
	header = P0SR_SEND | P0SIZE_DATA | P0IR_INITIATED;
	iSize = PKTSIZE_DATA;
	inst0 = 0x20;			// Register packet code
	if ((iBytes & 0x0f) == 2)
		inst0 |= 0x10;		// set two byte bit
	inst0 |= HIBYTE(iReg) & 0x07;		// set bank in low nibble
	inst1 = LOBYTE(iReg);
	value0 = LOBYTE(iValue);
	if (inst0 & 0x10)
		value1 = HIBYTE(iValue);
	else
		value1 = 0;
	iStatus = PSTAT_DETAILSET;
}

void CPacket::SetIDandCS(int iID)		// packet ID value and checksum
{
	header &= ~P0ID_MASK;
	header |= (char)(iID & P0ID_MASK);
	int cs = 0;
	for (int i = 0; i < iSize - 1; i++)
		cs += pktData[i];
	if (iSize > 1)
		pktData[iSize - 1] = (char)cs;
	iStatus = PSTAT_CSSET;
}

bool CPacket::CheckCS() const
{
	if (iSize == 1)
		return true;
	int cs = 0;
	for (int i = 0; i < iSize - 1; i++)
	cs += pktData[i];
	if (pktData[iSize - 1] == (BYTE)cs)
		return true;
	return false;
}

int CPacket::CompareID(int iID) const
{
	int diff = (header - iID) & P0ID_MASK;
	if (diff > (P0ID_MASK-1)/2)
		diff -= P0ID_MASK+1;
	return diff;
}


bool CPacket::GetReplyFor(const CPacket& txPkt)
{
	if (txPkt.IsReply())
		return false;
	header = txPkt.header;
	inst0 = txPkt.inst0;
	inst1 = txPkt.inst1;
	iSize = txPkt.iSize;
	iStatus = txPkt.iStatus;
	header &= ~P0IR_MASK;	// convert to reply
	header |=  P0IR_REPLY;

	// reply to a send
	if (txPkt.IsSend())
	{
		switch (txPkt.GetSizeType())
		{
		case P0SIZE_BYTE:
		case P0SIZE_DATA:
		case P0SIZE_STREAM:
			header &= ~P0SIZE_MASK;		// convert to byte
			header |=  P0SIZE_BYTE;
			iSize = 1;
			return true;
		case P0SIZE_PATH:
			header &= ~P0SIZE_MASK;		// convert to data packet
			header |=  P0SIZE_DATA;
			iSize = PKTSIZE_DATA;
			return true;
		default:
			ASSERT(0);
			return false;
		}
	}

	// reply to a request
	switch (txPkt.GetSizeType())
	{
	case P0SIZE_BYTE:
		ASSERT(0);
		return false;
	case P0SIZE_DATA:	// reply same is size
		break;
	case P0SIZE_PATH:
		iSize = PKTSIZE_DATA;
		return false;
	case P0SIZE_STREAM:
		iSize = PKTSIZE_DATA;
		return false;
	default:
		ASSERT(0);
		return false;
	}
	return true;
}

int CPacket::GetPktLenFromHeader(char hdr)
{
	switch (hdr & P0SIZE_MASK)
	{
	case P0SIZE_PATH:
		return m_iPathPktSize;
	case P0SIZE_DATA:
		return PKTSIZE_DATA;
	case P0SIZE_BYTE:
		return 1;
	case P0SIZE_STREAM:
		return 2;				// 2 so it returns after 2nd byte, 3 minimum length for a stream
	default:
		return 1;				// should not happen!
	}
}

bool CPacket::operator==(const CPacket& rhs) const
{
	if (iSize != rhs.iSize)
		return false;
	for (int i = iSize; --i >= 0;)
		if (pktData[i] != rhs.pktData[i])
			return false;
	return true;
}

char* CPacket::GetAsString() const
{
	char* pStr = m_strPkt;
	int iMax = sizeof(m_strPkt) / 3;
	int iSizeUse = (iSize <= iMax) ? iSize : iMax;
	for (int i = 0; i < iSizeUse; i++)
		pStr += sprintf(pStr, "%02x ", pktData[i]);
	if (i > 0)
		pStr[-1] = '\0';		// remove last space
	return m_strPkt;
}



