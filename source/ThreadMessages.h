
#if !defined(THREADMESSAGES_H__EB3D90C2F__INCLUDED_)
#define THREADMESSAGES_H__EB3D90C2F__INCLUDED_

// common user application messages start WM_USER + 0x01  -  see \common\ directory files
// specific user application messages start WM_USER + 0x100

enum
{
	WMU_BASE = WM_USER + 0x100,

// messages within user thread
	WMU_SETACTIVEPATHVIEW,

	
// messages from monitor thread to user thread
	WMU_COMMNOTIFY,
	WMU_UPDATEPARAM,
	WMU_MEMVALUE,
	WMU_REGVALUE,


// messages to user thread
	WMU_LOGEVENT,


// messages from user thread to monitor thread
	WMU_REQUEST,

	WMU_SENDPARAM,
	WMU_REQUESTPARAM,
	WMU_SENDALLPARAM,
	WMU_REQUESTALLPARAM,

	WMU_REQUESTMEM,
	WMU_SENDMEM,
	WMU_REQUESTREG,
	WMU_SENDREG,

	WMU_SENDBYTES,

	WMU_INITCONTACT,
	WMU_SETPARAMETERS,



	// path messages
	WMU_SETPATHBUFFER,
	WMU_RESETPATH,
	WMU_SENDPATH,
//	WMU_SENDNEXTPATH,
	WMU_STOPPATH,
	WMU_SENDMOREPATH,

	WMU_CHANGEVELOCITY,
	WMU_SETVELOCITY,
	WMU_STOPMOVEMENT,


	WMU_REQUESTNEXTPATHDATA,
	WMU_REQUESTALLPATHDATA,

	WMU_SENDPKTCONT,
	WMU_SENDPKTSTEP,


// General messages - both ways


};


enum	// monitor notifications to user thread
{
	CN_UPDATE_RXLOG = 1,
	CN_UPDATE_TXLOG,

	CN_PATHBUFFER_LOW,
	CN_PATHFINISHED,

	CN_UPDATE_MACHINEBUFFERINFO,
	CN_UPDATE_MACHINESTATEINFO,

	CN_SETSIMULATESTATE,
};

enum	// user messages to monitor thread
{
	REQ_SetSimulateResponse = 1,
	REQ_SetLogCommsData,
	REQ_SetLogProbeSamples,
	REQ_RecalibrateProbe,
	REQ_UpdateProbeSettings,
};


#endif // !defined(THREADMESSAGES_H__EB3D90C2F__INCLUDED_)
