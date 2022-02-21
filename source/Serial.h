#if !defined(SERIAL_H)
#define SERIAL_H


/***** Defines *****/


#define	BaudClock	115200L

#define	BRDL		0		// Baud rate divisor low byte
#define	BRDH		1		// Baud rate divisor high byte
#define	TxRx		0		// Transmit/Receive buffer
#define	IER		1		// Interrupt Enable Register
#define	IIR		2		// Interrupt Identification Register (read only)
#define	FCR		2		// FIFO Control Register (write only)
#define	LCR		3		// Line  Control Register
#define	MCR		4		// Modem Control Register
#define	LSR		5		// Line  Status  Register (read only)
#define	MSR		6		// Modem Status  Register (read only)


struct sIER {					// read/write
	bool EnDAvInt;
	bool EnTHREInt;
	bool EnLSRInt;
	bool EnMSRInt;
};
struct sIIR {					// read only
	bool IntPending;		// Interrupt pending
	char IntID;					// Interrupt ID
	bool TOIntPending;	// Timeout interrupt pending
	bool FIFOEn;				// FIFO queues enabled
};
struct sFCR {					// write only
	bool EnFIFO;
	bool ClrRxFIFO;
	bool ClrTxFIFO;
	bool ReadyPinMode1;
	char RxTrigLevel;
};
struct sLCR {					// read/write
	char WordLen;				// Word Length in bits (5->8)
	char StopBits;			// 1 or 2
	char Parity;				// None Odd or Even (0, 1, 2 respectively)
	bool EnParity;
	bool EnBreak;				// force spacing break state
	bool BRDReg;				// Access BRD reg if true, else Rx/Tx & IER
};
struct sMCR {					// read/write
	bool DTR;
	bool RTS;
	bool OUT1;
	bool OUT2;
	bool EnLoopBack;
};
struct sLSR {					// read only
	bool DataAv;				// Data available
	bool OverrunErr;
	bool ParityErr;
	bool FramingErr;
	bool BreakInt;
	bool THRE;					// Transmit holding register empty
	bool TSRE;					// Transmit shift register empty
	bool ErrFIFOQueue;	// Indicates a PE/FE/Break in FIFO queue
};
struct sMSR {					// read only
	bool delCTS;
	bool delDSR;
	bool delRI;
	bool delDCD;
	bool CTS;
	bool DSR;
	bool RI;
	bool DCD;
};



class CSerial {
protected:
	char m_COMNum;
	WORD m_PortAddr;
	WORD m_BRD;
	WORD m_BaudRate;

	inline void	SetPortAddr();
	inline void	UnselectBRD();
	inline void	SelectBRD();

public:
	char m_IERreg;
	char m_IIRreg;
	char m_FCRreg;
	char m_LCRreg;
	char m_MCRreg;
	char m_LSRreg;
	char m_MSRreg;
	sIER m_IER;
	sIIR m_IIR;
	sFCR m_FCR;
	sLCR m_LCR;
	sMCR m_MCR;
	sLSR m_LSR;
	sMSR m_MSR;


public:
	CSerial() { m_COMNum = -1; }
	CSerial(int port) { SetPort(port); }
	void SetPort(int port);
	void SetLoopBack(bool enable);

	WORD GetBRD();
	void SetBRD();

	struct sIER& GetIER();
	void SetIER();
	struct sIIR& GetIIR();
	void SetFCR();
	struct sLCR& GetLCR();
	void SetLCR();
	struct sMCR& GetMCR();
	void SetMCR();
	struct sLSR& GetLSR();
	struct sMSR& GetMSR();


};



/*
struct SerialParam {
	unsigned long BaudRate;
	int DataLength, StopBits, Parity, EvenParity, ParityEnable, Break;
	int FIFOEnable, RxTxReadyMode, TrigLevelRxFIFO;
	int DataAvailIntEnable, THREIntEnable, LinesStatusIntEnable, ModemStatXIntEnable;
};

class Serial {
	SerialParam Settings;
	int	COMNum;
	unsigned int PortAddr, BaudRateDivisor;
	int	FIFOEnabled;
	int	LineStatus;
	int	WaitFIFOEmpty, TxBuff, TxBuffSize;

	void	SetPortAddr()	{ PortAddr = *((int*)0x400+(COMNum-1)); }
								// Gets serial port(1-4) address from BIOS map
	void	SetTxRxIEReg() { outportb(PortAddr+LCR, 0x7f & inportb(PortAddr+LCR)); }
					// Clear bit 7 of LCR to access Tx/Rx and Interupt Enable registers
	void	SetBRDReg() { outportb(PortAddr+LCR, 0x80 | inportb(PortAddr+LCR)); }
					// Set bit 7 of LCR to access Baud Rate Divisor register
public:
	char	ClrRxFIFO, ClrTxFIFO;
	char	LoopBackTest;

	char	InteruptID;
	char	DTR, RTS, OUT1, OUT2;
	char	CTS, DSR, RI, RxSIG;

	Serial(int port = 1);
	int	SetPort(int port);
	int	GetPort();
	int	GetParameters(SerialParam &fetched);
	int	SetParameters(const SerialParam &newparam);
	void	ShowParameters(const SerialParam &settings);
	int	SetSignals();
	int	GetSignals();

	void	LetBuffEmpty() { WaitFIFOEmpty = 1; }
	int	TxReady();
	void	Tx(unsigned char data);

	inline int	RxAvail();   	// TRUE if Rx byte is available
	inline int	RxError();		// Use line status set in RxAvail()
	inline int	OverrunError();
	inline int	ParityError();
	inline int	FramingError();
	inline int	BreakIntError();
	unsigned char	RxByte();

};

*/

#endif		//!defined(SERIAL_H)
