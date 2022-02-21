// Serial.cpp: implementation of the CSerial class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "CNCControl.h"

#include <conio.h>
#include "serial.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



void CSerial::SetPort(int port)
{
	m_COMNum = port;
	SetPortAddr();
}


void CSerial::SetPortAddr()
{					// Gets serial port(1-4) address from BIOS map
//	if (m_COMNum >= 1 && m_COMNum <= 4)
//		m_PortAddr = *((WORD*)0x400 + (m_COMNum-1));	// causes access violation
	if (m_COMNum == 1)
		m_PortAddr = 0x03f8;
	else if (m_COMNum == 2)
		m_PortAddr = 0x02f8;
	else if (m_COMNum == 3)
		m_PortAddr = 0x00;
	else if (m_COMNum == 4)
		m_PortAddr = 0x00;
}

void CSerial::UnselectBRD()
{					// Clear bit 7 of LCR to access Tx/Rx and Interupt Enable registers
	_outp(m_PortAddr+LCR, 0x7f & _inp(m_PortAddr+LCR));
}

void CSerial::SelectBRD()
{					// Set bit 7 of LCR to access Baud Rate Divisor register
	_outp(m_PortAddr+LCR, 0x80 | _inp(m_PortAddr+LCR));
}


WORD CSerial::GetBRD()
{
	SelectBRD();
	m_BRD = _inp(m_PortAddr + BRDL) + (_inp(m_PortAddr + BRDH) << 8);
	UnselectBRD();
	m_BaudRate = BaudClock / m_BRD;
	return m_BRD;
}

void CSerial::SetBRD()
{
	SelectBRD();
//	m_BRD = (WORD)(BaudClock / m_BaudRate);
	_outp(m_PortAddr + BRDL, 0x00ff & m_BRD);
	_outp(m_PortAddr + BRDH, (0xff00 & m_BRD) >> 8);
	UnselectBRD();
}


struct sIER& CSerial::GetIER()
{
	char data = _inp(m_PortAddr + IER);
	m_IERreg = data;
	m_IER.EnDAvInt  = data & 1;
	m_IER.EnTHREInt = (data >> 1) & 1;
	m_IER.EnLSRInt  = (data >> 2) & 1;
	m_IER.EnMSRInt  = (data >> 3) & 1;
	return m_IER;
}

void CSerial::SetIER()
{
	char data;
	data  = m_IER.EnDAvInt & 1;
	data |= (m_IER.EnTHREInt & 1) << 1;
	data |= (m_IER.EnLSRInt & 1) << 2;
	data |= (m_IER.EnMSRInt & 1) << 3;
	_outp(m_PortAddr + IER, data);
}

struct sIIR& CSerial::GetIIR()
{
	char data = _inp(m_PortAddr + IIR);
	m_IIRreg = data;
	m_IIR.IntPending   = (data & 1) == 0;
	m_IIR.IntID        = (data >> 1) & 3;
	m_IIR.TOIntPending = (data >> 3) & 1;
	m_IIR.FIFOEn       = (data & 0xc0) != 0;
	return m_IIR;
}

void CSerial::SetFCR()
{
	char data;
	data  = m_FCR.EnFIFO & 1;
	data |= (m_FCR.ClrRxFIFO & 1) << 1;
	data |= (m_FCR.ClrTxFIFO & 1) << 2;
	data |= (m_FCR.ReadyPinMode1 & 1) << 3;
	data |= (m_FCR.RxTrigLevel & 3) << 6;
	_outp(m_PortAddr + FCR, data);
}

struct sLCR& CSerial::GetLCR()
{
	char data = _inp(m_PortAddr + LCR);
	m_LCRreg = data;
	m_LCR.WordLen  = (data & 3) + 5;
	m_LCR.StopBits = ((data >> 2) & 1) + 1;
	if ((data & 0x18) == 0x08) m_LCR.Parity = 1;
	else if ((data & 0x18) == 0x18) m_LCR.Parity = 2;
	else m_LCR.Parity =	0;
	m_LCR.EnParity = (data >> 5) & 1;
	m_LCR.EnBreak  = (data >> 6) & 1;
	return m_LCR;
}

void CSerial::SetLCR()
{
	char data;
	data  = (m_LCR.WordLen - 5) & 3;
	data |= ((m_LCR.StopBits - 1) & 1) << 2;
	if (m_LCR.Parity == 1) data |= 0x08;
	else if (m_LCR.Parity == 2) data |= 0x18;
	data |= (m_LCR.EnParity & 1) << 5;
	data |= (m_LCR.EnBreak & 1) << 6;			// bit 7 (BRD) is 0
	_outp(m_PortAddr + LCR, data);
}

struct sMCR& CSerial::GetMCR()
{
	char data = _inp(m_PortAddr + MCR);
	m_MCRreg = data;
	m_MCR.DTR  = data & 1;
	m_MCR.RTS  = (data >> 1) & 1;
	m_MCR.OUT1 = (data >> 2) & 1;
	m_MCR.OUT2 = (data >> 3) & 1;
	m_MCR.EnLoopBack = (data >> 4) & 1;
	return m_MCR;
}

void CSerial::SetMCR()
{
	char data;
	data  = m_MCR.DTR & 1;
	data |= (m_MCR.RTS & 1) << 1;
	data |= (m_MCR.OUT1 & 1) << 2;
	data |= (m_MCR.OUT2 & 1) << 3;
	data |= (m_MCR.EnLoopBack & 1) << 4;
	_outp(m_PortAddr + MCR, data);
}

void CSerial::SetLoopBack(bool enable)
{
	char data = _inp(m_PortAddr + MCR);
	if (enable) data |= 0x10;
	else data &= ~0x10;
	_outp(m_PortAddr + MCR, data);
}

struct sLSR& CSerial::GetLSR()
{
	char data = _inp(m_PortAddr + LSR);
	m_LSRreg = data;
	m_LSR.DataAv     = data & 1;
	m_LSR.OverrunErr = (data >> 1) & 1;
	m_LSR.ParityErr  = (data >> 2) & 1;
	m_LSR.FramingErr = (data >> 3) & 1;
	m_LSR.BreakInt   = (data >> 4) & 1;
	m_LSR.THRE       = (data >> 5) & 1;
	m_LSR.TSRE       = (data >> 6) & 1;
	m_LSR.ErrFIFOQueue = (data >> 7) & 1;
	return m_LSR;
}

struct sMSR& CSerial::GetMSR()
{
	char data = _inp(m_PortAddr + MSR);
	m_MSRreg = data;
	m_MSR.delCTS = data & 1;
	m_MSR.delDSR = (data >> 1) & 1;
	m_MSR.delRI  = (data >> 2) & 1;
	m_MSR.delDCD = (data >> 3) & 1;
	m_MSR.CTS    = (data >> 4) & 1;
	m_MSR.DSR    = (data >> 5) & 1;
	m_MSR.RI     = (data >> 6) & 1;
	m_MSR.DCD    = (data >> 7) & 1;
	return m_MSR;
}


/*
Serial::Serial(int port)
{
	SetPort(port);
	ClrRxFIFO = ClrTxFIFO = 0;
//	TrigLevelRxFIFO = 0;
//	TxBuffSize = 16;
	WaitFIFOEmpty = 1;

//	LoopBackTest = 0;
}

int
Serial::SetPort(int port)
{
	COMNum = port;
	SetPortAddr();
	return 1;
}

int
Serial::GetParameters(SerialParam &fetched)
{
	int	PortData;

	SetBRDReg();
	BaudRateDivisor = inportb(PortAddr+ BRDL)
							+ (inportb(PortAddr+ BRDH) << 8);
	SetTxRxIEReg();
	fetched.BaudRate = BaudClock / BaudRateDivisor;

	PortData = inportb(PortAddr+ LCR);
	fetched.DataLength =		(PortData & 3) + 5;
	fetched.StopBits =	  ((PortData >> 2) & 1) + 1;
	fetched.Parity =			(PortData >> 3) & 1;
	fetched.EvenParity =		(PortData >> 4) & 1;
	fetched.ParityEnable =	(PortData >> 5) & 1;
	fetched.Break =			(PortData >> 6) & 1;

	PortData = inportb(PortAddr+ IER);
	fetched.DataAvailIntEnable =	 PortData & 1;
	fetched.THREIntEnable =			(PortData >> 1) & 1;
	fetched.LinesStatusIntEnable =(PortData >> 2) & 1;
	fetched.ModemStatXIntEnable =	(PortData >> 3) & 1;

	PortData = inportb(PortAddr+ IIR);
	fetched.FIFOEnable = ((PortData >> 6) & 3) != 0;
	FIFOEnabled = fetched.FIFOEnable;

	return 1;
}

int
Serial::SetParameters(const SerialParam &newparam)
{
	int	PortData;

	SetBRDReg();
	BaudRateDivisor = (int)(BaudClock / newparam.BaudRate);
	outportb(PortAddr+ BRDL, 0xff & BaudRateDivisor);
	outportb(PortAddr+ BRDH, (0xff00 & BaudRateDivisor) >> 8);

	SetTxRxIEReg();
							// Compose Line Control Register
	PortData  = (newparam.DataLength-5) & 3;
	PortData |= ((newparam.StopBits-1) & 1) << 2;
	PortData |= (newparam.Parity & 1) << 3;
	PortData |= (newparam.EvenParity & 1) << 4;
	PortData |= (newparam.ParityEnable & 1) << 5;
	PortData |= (newparam.Break & 1) << 6;
	outportb(PortAddr+ LCR, PortData);

	PortData  = newparam.DataAvailIntEnable & 1;
	PortData |= (newparam.THREIntEnable & 1) << 1;
	PortData |= (newparam.LinesStatusIntEnable & 1) << 2;
	PortData |= (newparam.ModemStatXIntEnable & 1) << 3;
	outportb(PortAddr+ IER, PortData);

	PortData  = newparam.FIFOEnable & 1;
//	PortData |= (ClrRxFIFO & 1) << 1;
//	PortData |= (ClrTxFIFO & 1) << 2;
//	ClrRxFIFO = ClrTxFIFO = 0;
	PortData |= (newparam.RxTxReadyMode & 1) << 3;
	PortData |= (newparam.TrigLevelRxFIFO & 3) << 6;
	outportb(PortAddr+ FCR, PortData);
//	cprintf("IIR port: %x\r\n", inportb(PortAddr+ IIR));
	FIFOEnabled = newparam.FIFOEnable & 1;

	return 1;
}


void
Serial::ShowParameters(const SerialParam &settings)
{
	cprintf("\r\n");
	cprintf("COM%i address: %x\r\n", COMNum, PortAddr);
	cprintf("BaudRate: %li BRD: %x\r\n", settings.BaudRate, BaudRateDivisor);
	cprintf("DataLength: %i\r\n", settings.DataLength);
	cprintf("StopBits: %i\r\n", settings.StopBits);
	cprintf("Parity: %i\r\n", settings.Parity);
//	cprintf("Interupt Enable: %x\r\n", InteruptEnable);
	cprintf("Interupt ID reg: %x\r\n", InteruptID);
	cprintf("FIFO: %i\r\n", settings.FIFOEnable);
	cprintf("FIFOenabled: %i\r\n", FIFOEnabled);
	cprintf("LoopBackTest: %i\r\n", LoopBackTest);
}


int
Serial::SetSignals()
{
	int	PortData;

	PortData  = (DTR & 1);
	PortData |= (RTS & 1) << 1;
	PortData |= (OUT1 & 1) << 2;
	PortData |= (OUT2 & 1) << 3;
	PortData |= (LoopBackTest & 1) << 4;
	outportb(PortAddr+ MCR, PortData);

	return 1;
}

int
Serial::GetSignals()
{
	int PortData;

	PortData = inportb(PortAddr+ MCR);
	DTR = (PortData) & 1;
	RTS = (PortData >> 1) & 1;
	OUT1 = (PortData >> 2) & 1;
	OUT2 = (PortData >> 3) & 1;
	LoopBackTest = (PortData >> 4) & 1;

	PortData = inportb(PortAddr+ MSR);
	CTS = (PortData >> 4) & 1;
	DSR = (PortData >> 5) & 1;
	RI  = (PortData >> 6) & 1;
	RxSIG = (PortData >> 7) & 1;
	return PortData;
}

int
Serial::TxReady()
{
	if (!FIFOEnabled)
		return (inportb(PortAddr+LSR) & 0x20);
				// TRUE if Tx holding reg (or FIFO) is empty
	else
		if (WaitFIFOEmpty)
			if (inportb(PortAddr+LSR) & 0x20)
				{ WaitFIFOEmpty = TxBuff = 0; return 1; }
			else return 0;
		else
			if (TxBuff < TxBuffSize) return 1;
			else
				{ WaitFIFOEmpty = 1; return 0; }
}

void
Serial::Tx(unsigned char data)
{
	outportb(PortAddr+TxRx, data);
	TxBuff++;
#ifdef test
	text_info currinfo;
	gettextinfo(&currinfo);
	textcolor(TXCOLOR);
	cprintf("<%02x>", data);
	textattr(currinfo.attribute);
#endif
}

int
Serial::RxAvail()
{
	return (LineStatus = inportb(PortAddr+LSR)) & DATAREADY;
}

int
Serial::RxError()
{
	return LineStatus & 0x1e;		// These bits indicate various errors
}

int
Serial::OverrunError()
{
	return LineStatus & OVRUNERR;
}
int
Serial::ParityError()
{
	return LineStatus & PARITYERR;
}
int
Serial::FramingError()
{
	return LineStatus & FRAMERR;
}
int
Serial::BreakIntError()
{
	return LineStatus & BREAKINT;
}

unsigned char
Serial::RxByte()
{
#ifndef test
	return inportb(PortAddr+TxRx);
#else
	unsigned char data = inportb(PortAddr+TxRx);
	text_info currinfo;
	gettextinfo(&currinfo);
	textcolor(RXCOLOR);
	cprintf("<%02x>", data);
	textattr(currinfo.attribute);
	return data;
#endif
}

*/
