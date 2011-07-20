#ifndef __MDD_H__
#define __MDD_H__

//
// The serial event structure contains fields used to pass serial
// event information from the PDD to the MDD.  This information
// is passed via a callback, so the scope of this structure remains
// limited to the MDD.
//
typedef struct __COMM_EVENTS	{
	HANDLE  hCommEvent;			// @field Indicates serial events from PDD to MDD
	ULONG	fEventMask;			// @field Event Mask requestd by application 
	ULONG	fEventData;			// @field Event Mask Flag. 
    ULONG   fAbort;             // @field TRUE after SetCommMask( 0 )
	CRITICAL_SECTION EventCS;	    //  CommEvent access and related sign atom
	} COMM_EVENTS, *PCOMM_EVENTS;


/*
 @doc
 @struct RX_BUFFER_INFO | Driver Receive Buffer Information.
 */
typedef struct __RX_BUFFER_INFO {
	ULONG	Read;				/* @field Current Read index. */
	ULONG	Write;				/* @field Current Write index. */
	ULONG	Length;				/* @field Length of buffer */
	BOOL	DataAvail;			/* @field BOOL reflecting existence of data. */
	PUCHAR	RxCharBuffer;		/* @field Start of buffer */
	CRITICAL_SECTION	CS;		/* @field Critical section */
	} RX_BUFFER_INFO, *PRX_BUFFER_INFO;

#define RxResetFifo(pSH)   pSH->RxBufferInfo.Write = pSH->RxBufferInfo.Read = 0
#define RxEnterCS(pSH)	   EnterCriticalSection (&(pSH->RxBufferInfo.CS))
#define RxLeaveCS(pSH)	   LeaveCriticalSection (&(pSH->RxBufferInfo.CS))
#define RxWrite(pSH)	   (pSH->RxBufferInfo.Write)
#define RxRead(pSH)		   (pSH->RxBufferInfo.Read)
#define RxLength(pSH)	   (pSH->RxBufferInfo.Length)
// Don't use the power of 2 trick for length, because the PDD has option of overriding
// buffer length, and may not specify a power of 2.  Lets be more flexible & a very
// little bit slower.
// #define RxBytesAvail(pSH)  ((RxWrite(pSH) - RxRead(pSH)) & (RxLength(pSH) - 1))

#define RxBuffWrite(pSH)   (pSH->RxBufferInfo.RxCharBuffer+pSH->RxBufferInfo.Write)
#define RxBuffRead(pSH)	   (pSH->RxBufferInfo.RxCharBuffer+pSH->RxBufferInfo.Read)

typedef struct __TX_BUFFER_INFO {
	DWORD	Permissions;		/* @field Current permissions */
	ULONG	Read;				/* @field Current Read index. */
	ULONG	Length;				/* @field Length of buffer */
	PUCHAR	TxCharBuffer;		/* @field Start of buffer */
	CRITICAL_SECTION	CS;		/* @field Critical section */
} TX_BUFFER_INFO, *PTX_BUFFER_INFO;

#define TxEnterCS(pSH)	   EnterCriticalSection (&(pSH->TxBufferInfo.CS))
#define TxLeaveCS(pSH)	   LeaveCriticalSection (&(pSH->TxBufferInfo.CS))
#define TxRead(pSH)		   (pSH->TxBufferInfo.Read)
#define TxLength(pSH)	   (pSH->TxBufferInfo.Length)
#define TxBytesAvail(pSH)  (TxLength(pSH)-TxRead(pSH))
#define TxBuffRead(pSH)	   (pSH->TxBufferInfo.TxCharBuffer+pSH->TxBufferInfo.Read)

typedef VOID (*COMMFUNC)(PVOID	pHead);

// Forward declare for use below
typedef struct __HWOBJ HWOBJ, *PHWOBJ;
typedef struct __HW_OPEN_INFO HW_OPEN_INFO, *PHW_OPEN_INFO;

// @struct	HW_INDEP_INFO | Hardware Independent Serial Driver Head Information.
typedef struct __HW_INDEP_INFO {
    CRITICAL_SECTION	TransmitCritSec1;		// @field Protects tx action
    CRITICAL_SECTION	ReceiveCritSec1;		// @field Protects rx action

    PHWOBJ			pHWObj;			// @field Represents PDD object.
    PVOID	        pHWHead;		// @field Device context for PDD.

    HANDLE			hSerialEvent;	// @field Serial event, both rx and tx
    HANDLE			hReadEvent;		// @field Serial event, both rx and tx
    HANDLE			hKillDispatchThread;	// @field Synchonize thread end
    HANDLE			hTransmitEvent;	// @field transmit event, both rx and tx
    HANDLE			pDispatchThread;// @field ReceiveThread 
	ULONG			Priority256;    // @field CeThreadPriority of Dispatch Thread.
	ULONG			DroppedBytesMDD;// @field Record of bytes dropped by MDD.
	ULONG			DroppedBytesPDD;// @field Record of bytes dropped by PDD.
	ULONG			RxBytes;	    // @field Record of total bytes received.
	ULONG			TxBytes;	    // @field Record of total bytes transmitted.
	ULONG			TxBytesPending;	// @field Record of total bytes awaiting transmit.
	ULONG			TxBytesSent;	// @field Record of bytes sent in one transmission
	DCB				DCB;			// @field DCB (see Win32 Documentation.
	COMMTIMEOUTS	CommTimeouts;	// @field Time control field. 
	DWORD			OpenCnt;		// @field Protects use of this port 
	DWORD			KillRxThread:1;	// @field Flag to terminate SerialDispatch thread.
	DWORD			XFlow:1;		// @field True if Xon/Xoff flow ctrl.
	DWORD			StopXmit:1;		// @field Stop transmission flag.
	DWORD			SentXoff:1;		// @field True if XOFF sent.
	DWORD			DtrFlow:1;		// @field True if currently DTRFlowed	
	DWORD			RtsFlow:1;		// @field True if currently RTSFlowed
	DWORD           fAbortRead:1;   // @field Used for PURGE
    DWORD           fAbortTransmit:1;// @field Used for PURGE
	DWORD			Reserved:24;	// @field remaining bits.
	ULONG	        fEventMask;		// @field Sum of event mask for all opens
	RX_BUFFER_INFO	RxBufferInfo;	// @field rx buffer info.
	TX_BUFFER_INFO	TxBufferInfo;	// @field tx buffer info.


    LIST_ENTRY      OpenList;       // @field Head of linked list of OPEN_INFOs    
	CRITICAL_SECTION OpenCS;	    // @field Protects Open Linked List + ref counts

    PHW_OPEN_INFO   pAccessOwner;   // @field Points to whichever open has acess permissions
} HW_INDEP_INFO, *PHW_INDEP_INFO;

__inline  ULONG RxBytesAvail(PHW_INDEP_INFO  pSH) {
    // Note: We have to copy RxRead and RxWrite index to local in order to make it atomic.
    register DWORD RxWIndex=RxWrite(pSH), RxRIndex=RxRead(pSH);
    return (RxWIndex>=RxRIndex?RxWIndex- RxRIndex : RxLength(pSH) - RxRIndex + RxWIndex );
}

// @struct	HW_OPEN_INFO | Info pertaining to each open instance of a device
typedef struct __HW_OPEN_INFO {
    PHW_INDEP_INFO  pSerialHead;    // @field Pointer back to our HW_INDEP_INFO
    DWORD	        AccessCode;     // @field What permissions was this opened with
    DWORD	        ShareMode;      // @field What Share Mode was this opened with
    DWORD           StructUsers;    // @field Count of threads currently using struct.
	COMM_EVENTS		CommEvents;		// @field Contains all info for serial event handling
    LIST_ENTRY      llist;          // @field Linked list of OPEN_INFOs
	} HW_OPEN_INFO, *PHW_OPEN_INFO;

#define E_OF_CHAR			0xd
#define ERROR_CHAR			0xd
#define BREAK_CHAR			0xd
#define EVENT_CHAR			0xd
#define X_ON_CHAR			0x11
#define X_OFF_CHAR			0x13

#define X_FLOW_CTRL			0x1

#define MAX_SERIAL_INDEX	16
#define RX_BUFFER_SIZE		2048
#define COMM_FUNC_SIZE		16

static	PHW_INDEP_INFO	pGlobalSerialHead[MAX_SERIAL_INDEX];
static	ULONG		GlobalSerialHeadNumber = 0;

static
PHW_INDEP_INFO
GetSerialHead(
	PVOID	pHead
	);

#ifndef MIN
    #define MIN(x,y)		((x) < (y) ? (x) : (y))
#endif

//
// Macros to maintain a usage count for a particular serial structure,
// so that we know when it is safe to deallocate it.
//
#define COM_INC_USAGE_CNT(pOpenHead)  \
    InterlockedIncrement(&(pOpenHead->StructUsers))

#define COM_DEC_USAGE_CNT(pOpenHead)  \
    InterlockedDecrement(&(pOpenHead->StructUsers))

VOID
SerialEventHandler(PHW_INDEP_INFO pSerialHead);

/* Implemented by hw dependent code.
 */
#include <serhw.h>

#endif // __MDD_H__
