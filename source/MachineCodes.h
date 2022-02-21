#if !defined(AFX_MACHINECODES_H__INCLUDED_)
#define AFX_MACHINECODES_H__INCLUDED_


// see "CNC.par" for details

enum	// General Parameters
{
	GP_RegisterLoc = 0x00,
	GP_RAMLocationW0,
	GP_RAMLocationW1,
	GP_PWMcyclespupdate,

	GP_RealTimeSecW0 = 0x04,
	GP_RealTimeSecW1,
	GP_MachineDisable,

	GP_PathBuffMaxObjs = 0x07,
	GP_PathBuffNumObjs,
	GP_PathBuffFreeObjs,
	GP_PathBuffEndSegNum,
	GP_PathBuffEndTimeW0,
	GP_PathBuffEndTimeW1,

	GP_PathCurrSegNum = 0x0d,
	GP_PathCurrTimeW0,
	GP_PathCurrTimeW1,


	GP_PathReset = 0x10,
	GP_PathProcess,			// 1 to process path
	GP_ControlCode,

	GP_HandCtrlMove,			// 0x13
	GP_MemorySize,

	GP_SampleBuffMaxObjs,	// 0x15
	GP_SampleBuffNumObjs,
	GP_SampleBuffFreeObjs,
	GP_PathBuffObjSize,
	GP_SampleBuffObjSize,
	GP_LogSamplePeriod,
	GP_PathControlFlags,
	GP_CPUStatus,
	GP_PathBuffNumChangeToNotify,
	GP_NoLocatorNotify,

	GP_TimeEndIntMin,
	GP_TimeEndIntMax,
	GP_TimeEndIntOverflowCnt,

	GP_ErrorCount,
	GP_ErrorFirst,
	GP_ErrorLast,
	GP_RecentErrorCode,

	GP_SendSampleData,
	GP_LogSamples,
	GP_SampleRegBytes,
	GP_SampleReg0,
	GP_SampleReg1,
	GP_SampleParam0,
	GP_SampleParam1,				// 0x2c

	GP_MotionUpdateList0,	// 0x2d
	GP_MotionUpdateList7 = 0x34,

	GP_MotionUpdateNumSeq,	// 0x35
	GP_MotionUpdatePeriod,
	GP_SendMotionUpdates,

	GP_ServoPowerDisable = 0x3a,
	GP_StopAllMovement,
	GP_SetPosTrackToPos,
	GP_VoltStuckTrig,
	GP_VelX2StuckTrig,
	GP_StuckTimeThreshold,
	GP_CurrentSimTrig,
	GP_OverCurrentSimTimeThreshold,
	GP_OverILimitTimeThreshold,
	GP_PosErrorLimit,
	GP_ErrorAxisLast,
	GP_StopAllMovementSmooth,

	GP_I2CCommand,
	GP_I2CData,
	GP_I2CSlaveAddr,
	GP_I2CError,
	GP_I2CStatus,
	GP_I2CBaudRate,
	
	GP_SendProbeSamples,
	GP_ProbeSamplePeriod,
	GP_ProbeRelPosSampleStep,


};

enum	// Axis Parameters
{
	AP_RegisterLocation = 0x00,
	AP_ControlMode,
	AP_ChangeMode,
	AP_0x03,
	AP_PositionW0 = 0x04,
	AP_PositionW1,
	AP_Velocity,
	AP_Accel,
	AP_EncOffset,
	AP_Encoder,
	AP_Encoder1,
	AP_dPos1,
	AP_dPos2,
	AP_PosErrorW0,
	AP_PosErrorW1,
	AP_0x0f,

	AP_VoltCurr = 0x10,
	AP_VoltNew,
	AP_VoltMax,
	AP_VoltMin,
	AP_VoltMeasured,
	AP_CurrentMeasured,
	AP_PositionMin,
	AP_PositionMax,
	AP_0x18,
	AP_AxisControl,
	AP_AxisCmd,
	AP_AxisCmdValue,
	AP_AxisState,
	AP_AxisDisable,
	AP_AxisLocator,
	AP_AxisState2,
	AP_AccTrackW0 = 0x20,
	AP_AccTrackW1,
	AP_VelTrackW0 = 0x23,
	AP_VelTrackW1,
	AP_PosTrackW0 = 0x26,
	AP_PosTrackW1,
	AP_PosTrackW2,

	AP_dAccStepW0 = 0x2a,
	AP_dAccStepW1,
	AP_dAccelFactor,
	AP_AccelFactor,
	AP_OverflowCount,
	AP_OverflowType,

	AP_PIDp = 0x30,
	AP_PIDi,
	AP_PIDd,
	AP_0x33,
	AP_FFVoltPerDir,
	AP_FFVoltPerVel,
	AP_FFVoltPerAcc,
	AP_FFVoltPerdAcc,

	AP_PosTrackMax,
	AP_PosTrackMin,
	AP_VelTrackMax,
	AP_VelTrackMin,
	AP_AccTrackMax,
	AP_AccTrackMin,

	AP_LogAxisSamples,
	AP_NegateDirection,
	AP_NoPositionFeedback,	// = 0x40

	AP_PosErrorMax,
	AP_PosErrorMin,

	AP_AxisLocated,
	AP_LocatorBlockDistPos,
	AP_LocatorBlockDistNeg,
	AP_dIPerVolt,
	AP_dIPerCurr,
	AP_dIPerVel,
	AP_CurrentSim,
	AP_AccRampToVel,

	AP_IndexCngPosNotify,
	AP_IndexCngdPosNotify,


};

// Set error codes - text strings set in "ParamDoc.cpp"
enum	// Error Codes from PIC controller defined in 'Error.inc'
{
	//; Calculation errors in 'CalcPosVelAcc' in file 'VoltCtrl.asm'
	Error_PosOverflow = 2,		//; 24-bit position register overflowed in 'Calc_PosVelAcc'

	//; Comms errors in 'CheckPCIReceive' in file 'PCInter.asm'
	Error_PCRxBuffFull,			//; Received a byte from Serial port but Rx Buffer is full
	Error_RxBuffTooMany,			//; Received more bytes in buffer than Packet Length (fault in checking if end of packet)
	Error_RxPacketChecksum,		//; Received a packet with a checksum error
	Error_RxPacketID,				//; Received a packet with an ID mismatch
	Error_TxBuffOvf,				//; Serial Tx Buffer has overflowed (maybe midway through packet) in 'CheckPCIReceive'
	Error_TxBuffWontFit,			//; Serial Tx Buffer doesn't have room for packet in 'CheckPCIReceive'

	//; Path Buffer errors in file 'PCInter.asm'
	Error_PathBuffSizeSetNotEmpty,	//; PathBuffObjSize was attempted to be set with Path Buffer not empty
	Error_PathBuffRequGtNum,			//; Path buffer object greater than the current number of objects was requested (out of range)
	Error_PathTooManyObjsZeroTime,	//; Too many path objects with no time span to load in between interrupts
	Error_PathGetObjTimeLow,			//; Not enough time before interrupt to get more path objects
	Error_PathBufferFull,				//; Path buffer is full, new data discarded
	Error_PathObjCountMismatch,		//; Path object count wrong, new data discarded
	Error_PathObjChecksumFromBuff,	//; Checksum error when retrieving path object from buffer

	Error_SampleBuffEmpty,				//; Sample buff data requested but it's empty

	//; Interrupt errors in file 'PCInter.asm'
	Error_InterruptSendSampleData,	//; Interrupt occured in critical section
	Error_InterruptSetGetRAM,			//; Interrupt occured in critical section
	Error_InterruptSetPathData,		//; Interrupt occured in critical section
	Error_InterruptGetPathData,		//; Interrupt occured in critical section
	Error_InterruptSendMotionUpdate,	//; Interrupt occured in critical section

	//; Errors in file 'ParAcces.asm'
	Error_ParamNumBad,			//; Parameter Number requested/set is above maximum parameter number
	Error_ParamValueBad,			//; Tried to set to a bad value
	Error_ParamSetReadOnly,		//; A Read Only parameter was attempted to be set
	Error_ParamLocked,			//; Parameter value is currently locked
	Error_NotEnoughMemory,		//; Not enough memory for operation

	//; Errors in file 'HandCtrI.asm'
	Error_HandTxBuffOvf,			//; Serial Tx Buffer has overflowed (maybe midway through packet) in 'CheckPCIReceive'
	Error_HandTxBuffWontFit,	//; Serial Tx Buffer doesn't have room for packet in 'CheckPCIReceive'
	Error_HandRxBuffFull,		//; Received a byte from Serial port but Rx Buffer is full
	Error_HandRxBuffTooMany,	//; Received more byes in buffer than Packet Length (fault in checking if end of packet)
	Error_HandRxBuffChecksum,	//; Received a packet with a checksum error

	//; The folowing error cause 'ControlErrorShutDown'
	//; Errors in file 'VoltCtrl.asm'
	Error_ServoSimCurrentLimit,	//; Simulated servo current was over trigger
	Error_ServoAxisStuck,			//; Voltage high, velocity low condition
	Error_AxisPosErrorLimit,		//; Tracking position error >= error limit
	Error_AxisPastPosLimit,			//; Pos is outside bound
	Error_ProbeRequestTimedOut,	//; Did not receive probe data

	//; Errors in file 'Drive.asm'
	Error_LocalMachineShutdown,	//; Local Machine Shutdown pressed
	Error_RemoteMachineShutdown,	//; Remote Machine Shutdown pressed
	Error_LocatorBlockedDistance,	//; Locator was blocked above a limit distance
	Error_ServoDriveCurrentLimit,	//; Servo Drive Current Limit signaled

	//; Errors in file 'I2CInter.asm'
	Error_InterruptI2CCommand,		//; Interrupt occured during I2C transfer

	Error_Max,

};




#endif // !defined(AFX_MACHINECODES_H__INCLUDED_)

