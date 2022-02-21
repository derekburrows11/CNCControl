// Packet.h: interface for the CPacket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PACKET_H__FCE28760_C3FF_11D7_86C3_B691A07ACE25__INCLUDED_)
#define AFX_PACKET_H__FCE28760_C3FF_11D7_86C3_B691A07ACE25__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Param.h"
#include "ControllerTracker.h"


enum	// Packet byte 0 header
{
	P0SR_MASK			= 0x40,
	P0SR_SEND			= 0x00,
	P0SR_REQUEST		= 0x40,

	P0SIZE_MASK			= 0x30,	// packet type - determines size
	P0SIZE_BYTE			= 0x00,
	P0SIZE_DATA			= 0x10,	// param or other values
	P0SIZE_PATH			= 0x20,	// for Path axis data
	P0SIZE_STREAM		= 0x30,	// variable length

	P0IR_MASK			= 0x08,
	P0IR_INITIATED		= 0x08,
	P0IR_REPLY			= 0x00,	// swap this bit

	P0ID_MASK			= 0x07,
};
enum	// Data Packet byte 1 (type of data)
{
	P1DATA_MASK					= 0xf0,
	P1DATA_PARAM				= 0x00,
	P1DATA_REGISTER			= 0x20,
	P1DATA_REGISTER2			= 0x30,
	P1DATA_REQPATH				= 0x40,
	P1DATA_REQSAMPLE			= 0x50,
	P1DATA_REQDESCR			= 0x70,
	P1DATA_PATHCONF			= 0x80,
	P1DATA_MEMORY				= 0xc0,
	P1DATA_CONTROLLERERROR	= 0xe0,
	P1DATA_COMMSERROR			= 0xf0,

	P1AXIS_MASK			= 0x0f,		// axis 0 is general param
	P1AXIS_ALL			= 0x0f,		// all axes
	P1AXIS_SET			= 0x0e,		// a predefined set of axes
};

enum	// Path Packet byte 1 (type of data)
{
	P1PATH_MASK				= 0xf0,
	P1PATH_ACCRAMP			= 0x00,
	P1PATH_ACCSTEP			= 0x10,
	P1PATH_ACCRAMPCOUNT	= 0x20,
	P1PATH_ACCSTEPCOUNT	= 0x30,
	P1PATH_VEL				= 0x20,	// change
	P1PATH_POS				= 0x30,	// change
};

enum	// Stream Packet byte 1 (type of data)
{
	P1STREAM_MASK				= 0xf0,
	P1STREAM_CONTDESCR		= 0x10,
	P1STREAM_LOCCHANGE		= 0x20,
	P1STREAM_SAMPLEDATA		= 0x30,
	P1STREAM_MOTIONUPDATE	= 0x40,
	P1STREAM_PROBESAMPLE		= 0x50,
};

enum	// packet sizes
{
	PKTSIZE_DATA		= 6,
//	PKTSIZE_DEFPATH	= 16,
//	PKTSIZE_MAX			= 16,	// path packet with 4byte/axis, 3 axis + other
	PKTSIZE_MAX			= 19,	// variable packet
};
enum	// iStatus values
{
	PSTAT_DETAILSET = 10,
	PSTAT_IDSET,
	PSTAT_CSSET,
	PSTAT_FROMDATA,

	PSTAT_RXPARTCONF1,		// if expecting more reply packets from initiated pkt, inc with each reply
	PSTAT_RXCONFIRMATION = PSTAT_RXPARTCONF1 + 32,
};
enum
{
	POB_NEXTTOUSE = 1,
	POB_LASTSENT,
};


class CPacket
{
public:
// data
	union {
		BYTE pktData[PKTSIZE_MAX];
#pragma warning(disable : 4201)					// nonstandard extension used : nameless struct/union
		struct {
			BYTE header, axis, param, vall, valh; };
		struct {
			BYTE header, inst0, inst1, value0, value1; };
//		struct {
//			BYTE header; WORD inst; short value; };	// 1 byte padded after header!
#pragma warning(default : 4201)
	};
	int iSize;
	int iStatus;
	static int m_iPathPktSize;
	static int m_iPathValueSize;
	static int m_iPathValueMax;
	static int m_iPathValueMin;
	static int m_iPathNumAxis;
	static char m_strPkt[64];

// functions
	CPacket() { iStatus = iSize = 0; }
	void SetFromData(char* pData, int iLen);
//	char* GetData() { return (char*)pktData; }
	BYTE* GetData() { return pktData; }
	int GetSize() const { return iSize; }
	static int GetPktLenFromHeader(char header);
	static int CalcPathPktSize() { return m_iPathPktSize = 4 + m_iPathValueSize*m_iPathNumAxis; }

	// set packet functions
	void ByteNOP();
	void SendNOP();
	void SendParam(CAxPar axpar, int iValue);
	void ReqParam(CAxPar axpar, int iValue = 0);
	void SendMem(int iAddr, int iBytes, int iValue);
	void ReqMem(int iAddr, int iBytes);
	void SendReg(int iReg, int iBytes, int iValue);
	void ReqReg(int iReg, int iBytes);

	// path packets
	void SendPath(SSegAccel& ad);
	void ReqPath(int iObjNum, int nBase);
	void SendPathPos();
	void SendPathVel();
	void SetPathValues(const int* arVal);
	void SetPathValues(const double* arVal, double unit);
	void GetPathValues(int* arVal) const;
	void GetPathValues(double* arVal, double unit) const;

	// stream packets
	static int GetStreamPktLenFromByte2(char byte2) { return 4 + (byte2 & 0x0f); }
	int GetStreamDataLen() { return 4 - 3 + (inst0 & 0x0f); }		// length without overheads
	BYTE* GetStreamData() { return pktData + 2; }


	void ReturnError(int iErrorCode, int iValue);

	// data extraction
	char* GetAsString() const;
	void GetSegAccel(SSegAccel& sa) const;


	void SetIDandCS(int iID);
	bool CheckCS() const;
	int CompareID(int iID) const;
	bool GetReplyFor(const CPacket& txPkt);
	bool IsInitiated() const { return (header & P0IR_MASK) == P0IR_INITIATED; }
	bool IsReply() const { return (header & P0IR_MASK) == P0IR_REPLY; }
	bool IsSend() const { return (header & P0SR_MASK) == P0SR_SEND; }
	bool IsRequest() const { return (header & P0SR_MASK) == P0SR_REQUEST; }
	bool IsCommsErrorPacket() const { return (GetSizeType() == P0SIZE_DATA) && (GetDataType() == P1DATA_COMMSERROR); }
	int GetSizeType() const { return header & P0SIZE_MASK; }
	int GetDataType() const { return inst0 & P1DATA_MASK; }
	int GetPathType() const { return inst0 & P1PATH_MASK; }
	int GetStreamType() const { return inst0 & P1STREAM_MASK; }
	CAxPar GetAxPar() const { return CAxPar(axis, param); }
	int GetAxis() const { return axis & P1AXIS_MASK; }
	bool IsGeneralParam() const { return axis == 0; }
	int GetParam() const { return param; }
	short GetValue() const { return *(short*)&value0; }
	void SetValue(int iValue) { *(short*)&value0 = (short)iValue; }

	BYTE GetErrorNumber() const { return inst1; }

	bool operator==(const CPacket& rhs) const;
	BYTE operator[](int idx) const { return pktData[idx]; }
};




#endif // !defined(AFX_PACKET_H__FCE28760_C3FF_11D7_86C3_B691A07ACE25__INCLUDED_)
