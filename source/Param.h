// Param.h: interface for the PARAM, CAxPar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARAM_H__2A190824_AFD7_11D4_8C1E_A573C67E3C2C__INCLUDED_)
#define AFX_PARAM_H__2A190824_AFD7_11D4_8C1E_A573C67E3C2C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000



class CAxPar
{
public:
	BYTE param;
	BYTE axis;		// 0 is general param, 0x0f means all axis
public:
	CAxPar() {}
	CAxPar(int axpar) { *(WORD*)&param = LOWORD(axpar); }
	CAxPar(int iAxis, int iParam) { param = (BYTE)iParam; axis = (BYTE)iAxis; }
	BYTE Param() { return param; }
	BYTE Axis() { return axis; }
	bool IsAllAxis() { return (axis & 0x0f) == 0x0f; }
	bool IsGeneralParam() { return (axis & 0x0f) == 0; }
	void NextParam() { if (param < 0xff) param++; }
	void PrevParam() { if (param > 0x00) param--; }
	operator int() { return *(WORD*)&param; }
};

class CParam
{
public:
	CParam() { code = -1; name = NULL; description = NULL; }
	int code;
	char* name;			// address of allocated memory block
	char* description;
	int value;			// displayed value
	int sendValue;		// change to valueRx - set on rx of request or send confirm
	int minValue, maxValue;
	int state;
	int style;
};





#endif // !defined(AFX_PARAM_H__2A190824_AFD7_11D4_8C1E_A573C67E3C2C__INCLUDED_)
