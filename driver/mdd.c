/*
Functions:
COM_Init
COM_Open
COM_Close
COM_Deinit
COM_Read
COM_Write
COM_Seek
COM_PowerUp
COM_PowerDown
COM_IOControl
DllEntry
SerialEventHandler
SerialDispatchThread
ApplyDCB
SerialGetDroppedByteNumber
WaitCommEvent
EvaluateEventFlag
ProcessExiting
*/

#include <windows.h>
#include <linklist.h>
#include <serdbg.h>
#include <pegdser.h>
#include <devload.h>
#include <ceddk.h>
#include <pm.h>

#include "config.h"
#include "mdd.h"

#if (VGD_USE_LOG==VGD_LOG_MODE_FULL)
# include "log.h"
#endif

/* Debug Zones.
 */
#ifdef DEBUG

    #define DBG_INIT    0x0001
    #define DBG_OPEN    0x0002
    #define DBG_READ    0x0004
    #define DBG_WRITE   0x0008
    #define DBG_CLOSE   0x0010
    #define DBG_IOCTL   0x0020
    #define DBG_THREAD  0x0040
    #define DBG_EVENTS  0x0080
    #define DBG_CRITSEC 0x0100
    #define DBG_FLOW    0x0200
    #define DBG_IR      0x0400
    #define DBG_NOTHING 0x0800
    #define DBG_ALLOC   0x1000
    #define DBG_FUNCTION 0x2000
    #define DBG_WARNING 0x4000
    #define DBG_ERROR   0x8000

DBGPARAM dpCurSettings = {
    TEXT("Serial"), {
        TEXT("Init"),TEXT("Open"),TEXT("Read"),TEXT("Write"),
        TEXT("Close"),TEXT("Ioctl"),TEXT("Thread"),TEXT("Events"),
        TEXT("CritSec"),TEXT("FlowCtrl"),TEXT("Infrared"),TEXT("User Read"),
        TEXT("Alloc"),TEXT("Function"),TEXT("Warning"),TEXT("Error")},
    0
}; 
#endif

// Define some internally used functions
BOOL COM_Close(PHW_OPEN_INFO    pOpenHead);
BOOL COM_Deinit(PHW_INDEP_INFO pSerialHead);
VOID EvaluateEventFlag(PVOID pHead, ULONG fdwEventMask);

/*
 @doc INTERNAL
 @func	BOOL | DllEntry | Process attach/detach api.
 *
 @rdesc The return is a BOOL, representing success (TRUE) or failure (FALSE).
 */
BOOL
DllEntry(
              HINSTANCE   hinstDll,             /*@parm Instance pointer. */
              DWORD   dwReason,                 /*@parm Reason routine is called. */
              LPVOID  lpReserved                /*@parm system parameter. */
              )
{
    int proc_id = GetCurrentProcessId();

    if ( dwReason == DLL_PROCESS_ATTACH ) {
        //LOGINIT;
        DEBUGREGISTER(hinstDll);
        DEBUGMSG (ZONE_INIT, (TEXT("serial port process attach, process id = 0x%X\r\n"), proc_id));
        DisableThreadLibraryCalls((HMODULE) hinstDll);
    }

    if ( dwReason == DLL_PROCESS_DETACH ) {
        DEBUGMSG (ZONE_INIT, (TEXT("process detach called\r\n")));
        //LOGDONE;
    }

    return(TRUE);
}

/*
 @doc INTERNAL
 @func	VOID | DoTxData | Sends next available chunk of TX data.
 *
 */
VOID
DoTxData( PHW_INDEP_INFO pSerialHead )
{
    PHW_VTBL            pFuncTbl = pSerialHead->pHWObj->pFuncTbl;
    PVOID               pHWHead = pSerialHead->pHWHead;
    ULONG               Len;

    DEBUGMSG (ZONE_WRITE, (TEXT("DoPutBytes wait for CritSec %x.\r\n"),
                           &(pSerialHead->TxBufferInfo.CS)));
    TxEnterCS(pSerialHead);
    DEBUGMSG (ZONE_WRITE, (TEXT("DoPutBytes got CritSec %x.\r\n"),
                           &(pSerialHead->TxBufferInfo.CS)));

    // If device was closed from under us, stop transmitting
    if ( !pSerialHead->OpenCnt ) {
        DEBUGMSG (ZONE_THREAD|ZONE_WRITE , (TEXT("Device closed! Quit transmission!\r\n")));
        DEBUGMSG (ZONE_WRITE,
                  (TEXT("SerialEventHandler: %d sent up-to-now.\n\r"),pSerialHead->TxBytesSent));
        pSerialHead->TxBufferInfo.Permissions = 0;
        pSerialHead->TxBufferInfo.TxCharBuffer = NULL;
        pSerialHead->TxBufferInfo.Length = 0;
        TxRead(pSerialHead) = 0;
    }

    // Check the flow control status, and if not flowed off, call the
    // hw TX routine to actually transmit some data.
    if ( pSerialHead->TxBufferInfo.TxCharBuffer && TxBytesAvail(pSerialHead) ) {
        DWORD oldPerm = SetProcPermissions(pSerialHead->TxBufferInfo.Permissions);

        if ( pSerialHead->DCB.fRtsControl == RTS_CONTROL_TOGGLE ) {
            DEBUGMSG (ZONE_THREAD|ZONE_WRITE , (TEXT("RTS set.\r\n")));
            pFuncTbl->HWSetRTS(pHWHead);
        }

        // Don't transmit anything if we are flowed off.
        if ( pSerialHead->StopXmit ) {
            // But we still need to call TxIntrHandler so that the interrupt
            // gets cleared.
            DEBUGMSG (ZONE_FLOW|ZONE_WRITE , (TEXT("XOFF'ed, send nothing.\r\n")));
            Len = 0;
        } else {
            DEBUGMSG (ZONE_WRITE,
                      (TEXT("TxRead = %d, TxLength = %d, TxBytesAvail = %d.\r\n"),
                       TxRead(pSerialHead), TxLength(pSerialHead),
                       TxBytesAvail(pSerialHead)));
            Len = TxBytesAvail(pSerialHead);
        }
        DEBUGMSG (ZONE_WRITE, (TEXT("About to copy %d bytes\r\n"), Len));
        pFuncTbl->HWTxIntrHandler(pHWHead,
                                  TxBuffRead(pSerialHead),
                                  &Len);
        DEBUGMSG (ZONE_WRITE, (TEXT("%d bytes actually copied.\r\n"), Len));
        // Update Fifo info
        pSerialHead->TxBytes += Len;
        pSerialHead->TxBytesSent += Len;
        TxRead(pSerialHead) += Len;

        // Even if everything was Tx'ed, don't signal TX complete until
        // we get transmit interrupt indicating that the data has
        // actually been sent.  Since few/no UARTS have a way to tell
        // how much data remains, we don't bother trying to adjust the
        // return length to account for partially completed hardware buffer TX
        SetProcPermissions(oldPerm);
    } else {
        // Even if there is nothing left to send, we need to call
        // the interrupt handler so that it can clear the
        // transmit interrupt
        Len = 0;
        pFuncTbl->HWTxIntrHandler(pHWHead,
                                  NULL,
                                  &Len);        
        DEBUGMSG (ZONE_WRITE, (TEXT("Transmission complete, %d bytes sent\r\n"), Len));
        pSerialHead->TxBufferInfo.Permissions = 0;
        pSerialHead->TxBufferInfo.TxCharBuffer = NULL;
        pSerialHead->TxBufferInfo.Length = 0;
        TxRead(pSerialHead) = 0;
        SetEvent(pSerialHead->hTransmitEvent);
    }

    TxLeaveCS(pSerialHead);
    DEBUGMSG (ZONE_WRITE|ZONE_FUNCTION,
              (TEXT("DoPutBytes released CritSec: %x.\r\n"),
               &(pSerialHead->TxBufferInfo.CS)));

}

VOID
SerialEventHandler(PHW_INDEP_INFO       pSerialHead)
{
    PHW_VTBL            pFuncTbl = pSerialHead->pHWObj->pFuncTbl;
    PVOID               pHWHead = pSerialHead->pHWHead;
    ULONG               CharIndex;
    ULONG               RoomLeft = 0;
    ULONG               TotalLeft = 0;
    INTERRUPT_TYPE      it = INTR_NONE;
    BOOL                RxDataAvail = FALSE;

    DEBUGMSG (ZONE_THREAD, (TEXT("+SerialEventHandler, pHead 0x%X\r\n"),
                            pSerialHead));

    if ( pSerialHead->KillRxThread ||
         !pSerialHead->hSerialEvent ) {
        DEBUGMSG (ZONE_THREAD, (TEXT("Exitting thread\r\n")));
        SetEvent(pSerialHead->hKillDispatchThread);
        ExitThread(0);
    }

// NOTE - This one is a little tricky.  If the only owner is a monitoring task
// then I don't have an owner for read/write, yet I might be in this routine
// due to a change in line status.  Lets just do the best we can and increment
// the count for the access owner if available.
    if ( pSerialHead->pAccessOwner )
        COM_INC_USAGE_CNT(pSerialHead->pAccessOwner);

    while ( 1 ) {

        if ( !(it = pFuncTbl->HWGetIntrType(pHWHead)) ) {
            DEBUGMSG (ZONE_THREAD,
                      (TEXT("SerialEventHandler, No Interrupt.\r\n")));
            break;
        }

        DEBUGMSG (ZONE_THREAD,
                  (TEXT("SerialEventHandler, Interrupts 0x%X\r\n"), it));
        if ( it & INTR_RX ) {
            // It's read data event. Optimize the read by reading chunks
            // if the user has not specified using xflow control
            // or event/error/eof characters. Ack the receive,
            // unmask the interrupt, get the current data pointer
            // and see if data is available.
            // Note: We have to copy RxRead and RxWrite index to local in order to make it atomic.
            register DWORD RxWIndex, RxRIndex;

            DEBUGMSG (ZONE_THREAD|ZONE_READ , (TEXT("Rx Event\r\n")));

            RxEnterCS(pSerialHead);

            RxWIndex = RxWrite(pSerialHead);
            RxRIndex = RxRead(pSerialHead);

            if ( RxRIndex == 0 ) {
                // have to leave one byte free.
                RoomLeft = RxLength(pSerialHead) - RxWIndex - 1;
            } else {
                RoomLeft = RxLength(pSerialHead) - RxWIndex;
            }
            if ( RxRIndex > RxWIndex ) {
                RoomLeft = RxRIndex - RxWIndex - 1;
            }
            if ( RoomLeft ) {
                pSerialHead->DroppedBytesPDD +=
                pFuncTbl->HWRxIntrHandler(pHWHead,
                                          RxBuffWrite(pSerialHead),
                                          &RoomLeft);
            } else {
                BYTE    TempBuf[16];
                RoomLeft = 16;
                pFuncTbl->HWRxIntrHandler(pHWHead,
                                          TempBuf,
                                          &RoomLeft);

                pSerialHead->DroppedBytesMDD += RoomLeft;
                DEBUGMSG (ZONE_WARN|ZONE_READ, (TEXT("Tossed %d bytes\r\n"),
                                                RoomLeft));
                RoomLeft = 0;
            }

            DEBUGMSG (ZONE_READ ,
                      (TEXT("After HWGetBytes, Fifo(R=%d,W=%d,BA=%d,L=%d) ByteRead=%d\r\n"),
                       RxRead(pSerialHead), RxWrite(pSerialHead),
                       RxBytesAvail(pSerialHead), RxLength(pSerialHead),
                       RoomLeft));


            // If flow control enabled then we need to scan for XON/XOFF
            // characters
            if ( pSerialHead->XFlow ) {
                for ( CharIndex=0; CharIndex < RoomLeft; ) {
                    if ( RxBuffWrite(pSerialHead)[CharIndex] ==
                         pSerialHead->DCB.XoffChar ) {
                        DEBUGMSG (ZONE_FLOW, (TEXT("Received XOFF\r\n")));

                        pSerialHead->StopXmit = 1;
                        memmove (RxBuffWrite(pSerialHead)+CharIndex,
                                 RxBuffWrite(pSerialHead)+CharIndex+1,
                                 RoomLeft - CharIndex);
                        RoomLeft--;
                        continue;
                    } else if ( RxBuffWrite(pSerialHead)[CharIndex] ==
                                pSerialHead->DCB.XonChar ) {
                        pSerialHead->StopXmit = 0;
                        DEBUGMSG (ZONE_FLOW, (TEXT("Received XON\r\n")));
                        memmove (RxBuffWrite(pSerialHead)+CharIndex,
                                 RxBuffWrite(pSerialHead)+CharIndex+1,
                                 RoomLeft - CharIndex);
                        RoomLeft--;
                        // We disabled TX on XOFF, so now we need to start sending
                        // again. Easiest way is to pretend we saw a TX interrupt
                        it |= INTR_TX;
                        continue;
                    }
                    CharIndex++;
                }
            }

            pSerialHead->RxBytes += RoomLeft;
            RxWrite(pSerialHead) = 
                (RxWrite(pSerialHead)+RoomLeft<RxLength(pSerialHead)? RxWrite(pSerialHead)+RoomLeft: RxWrite(pSerialHead)+RoomLeft-RxLength(pSerialHead));
            if ( RoomLeft ) {
                RxDataAvail = TRUE;
            }

            /* Support DTR_CONTROL_HANDSHAKE/RTS_CONTROL_HANDSHAKE
             * signal is cleared when the input buffer is more than 3/4 full.
             */
            if ( (pSerialHead->DCB.fDtrControl == DTR_CONTROL_HANDSHAKE) &&
                 (!pSerialHead->DtrFlow) && 
                 (4*RxBytesAvail(pSerialHead) > (3*RxLength(pSerialHead))) ) {
                DEBUGMSG (ZONE_READ|ZONE_FLOW,
                          (TEXT("DTR_CONTROL_HANDSHAKE Clearing DTR\r\n")));
                pSerialHead->DtrFlow = 1;
                pFuncTbl->HWClearDTR(pHWHead);
            }
            if ( (pSerialHead->DCB.fRtsControl == RTS_CONTROL_HANDSHAKE) &&
                 (!pSerialHead->RtsFlow) &&
                 (4*RxBytesAvail(pSerialHead) > (3*RxLength(pSerialHead))) ) {
                DEBUGMSG (ZONE_READ|ZONE_FLOW,
                          (TEXT("RTS_CONTROL_HANDSHAKE Clearing RTS\r\n")));
                pSerialHead->RtsFlow = 1;
                pFuncTbl->HWClearRTS(pHWHead);
            }

            /* If Xon/Xoff flow control is desired. check the limit against
             * the remaining room and act accordingly.
             */
            if ( pSerialHead->DCB.fInX && !(pSerialHead->SentXoff) &&
                 ( pSerialHead->DCB.XoffLim >=
                   (RxLength(pSerialHead) - RxBytesAvail(pSerialHead))) ) {
                DEBUGMSG (ZONE_FLOW, (TEXT("Sending XOFF\r\n")));
                pFuncTbl->HWXmitComChar(pHWHead, pSerialHead->DCB.XoffChar);

                pSerialHead->SentXoff = 1;
                if ( !pSerialHead->DCB.fTXContinueOnXoff ) {
                    pSerialHead->StopXmit = 1;
                }
            }

            RxLeaveCS(pSerialHead);
        }

        if ( it & INTR_TX ) {
            DEBUGMSG (ZONE_THREAD|ZONE_WRITE , (TEXT("Tx Event\r\n")));
            DoTxData( pSerialHead );
        }

        if ( (it & INTR_MODEM) ) {
            DEBUGMSG (ZONE_THREAD, (TEXT("Other Event, it:%x\r\n"), it));

            /* Call low level status clean up code.
             */
            pFuncTbl->HWModemIntrHandler(pHWHead);
        }

        if ( it & INTR_LINE ) {
            DEBUGMSG (ZONE_THREAD, (TEXT("Line Event, it:%x\r\n"), it));

            /* Call low level line status clean up code.
             * Then unmask the interrupt
             */
            pFuncTbl->HWLineIntrHandler(pHWHead);
        }
    }

    // We kept this till the end to optimize the above loop
    if ( RxDataAvail ) {
        // Signal COM_Read that bytes are available.
        SetEvent(pSerialHead->hReadEvent);
        EvaluateEventFlag(pSerialHead, EV_RXCHAR);
    }

    DEBUGMSG (ZONE_THREAD ,
              (TEXT("-SerialEventHandler, Fifo(R=%d,W=%d,L=%d)\r\n"),
               RxRead(pSerialHead), RxWrite(pSerialHead),
               RxLength(pSerialHead)));

    if ( pSerialHead->pAccessOwner )
        COM_DEC_USAGE_CNT(pSerialHead->pAccessOwner);

    return;
}

/*
 @doc INTERNAL
 @func	ULONG | SerialDispatchThread | Main serial event dispatch thread code.
 *	This is the reading and dispatching thread. It gets the
 *	event associated with the logical interrupt dwIntID and calls
 *	hardware specific routines to determine whether it's a receive event
 *	or a transmit event. If it's a transmit event, it calls the HW tx handler.
 *	If it's a receive event, it calls for the number of characters and calls
 *	atomic GetByte() to extract characters and put them into the drivers
 *	buffer represented by pSerialHead->pTargetBuffer, managing waiting
 *	for events and checking to see if those signals correspond to reading.
 *	It relies on NK masking the interrupts while it does it's thing, calling
 *	InterruptDone() to unmask them for each of the above cases.
 *
 *	Not exported to users.
 *
 @rdesc This thread technically returns a status, but in practice, doesn't return
 *		while the device is open.
 */
static DWORD WINAPI
SerialDispatchThread(
                    PVOID   pContext    /* @parm [IN] Pointer to main data structure. */
                    )
{
    PHW_INDEP_INFO      pSerialHead    = (PHW_INDEP_INFO)pContext;
    ULONG               WaitReturn;

    DEBUGMSG (ZONE_THREAD, (TEXT("Entered SerialDispatchThread %X\r\n"),
                            pSerialHead));

    // It is possible for a PDD to use this routine in its private thread, so
    // don't just assume that the MDD synchronization mechanism is in place.
    if ( pSerialHead->pHWObj->BindFlags & THREAD_IN_MDD ) {
        DEBUGMSG(ZONE_INIT,
                 (TEXT("Spinning in dispatch thread %X %X\n\r"), pSerialHead, pSerialHead->pHWObj));
        while ( !pSerialHead->pDispatchThread ) {
            Sleep(20);
        }
    }

    /* Wait for the event that any serial port action creates.
     */
    while ( !pSerialHead->KillRxThread ) {
        DEBUGMSG (ZONE_THREAD, (TEXT("Event %X, %d\r\n"),
                                pSerialHead->hSerialEvent,
                                pSerialHead->pHWObj->dwIntID ));
        WaitReturn = WaitForSingleObject(pSerialHead->hSerialEvent, INFINITE);

        SerialEventHandler(pSerialHead);
    }

    DEBUGMSG (ZONE_THREAD, (TEXT("SerialDispatchThread %x exiting\r\n"),
                            pSerialHead));
    return(0);
}

// ****************************************************************
//
//	@doc INTERNAL
//	@func		BOOL | StartDispatchThread | Start thread if requested by PDD.
//
//	@parm 		ULONG  | pSerialHead
//
//	 @rdesc		TRUE if success, FALSE if failed.
//
BOOL
StartDispatchThread(
                   PHW_INDEP_INFO  pSerialHead
                   )
{
    // Set up the dispatch thread and it's kill flag. Note that the thread
    // fills in its own handle in pSerialHead.
    pSerialHead->KillRxThread = 0;
    pSerialHead->pDispatchThread = NULL;

    DEBUGMSG(ZONE_INIT,
             (TEXT("Spinning thread%X\n\r"), pSerialHead));

    pSerialHead->pDispatchThread = CreateThread(NULL,0, SerialDispatchThread,
                                                pSerialHead, 0,NULL);
    if ( pSerialHead->pDispatchThread == NULL ) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR,
                 (TEXT("Error creating dispatch thread (%d)\n\r"),
                  GetLastError()));
        return(FALSE);
    }

    DEBUGMSG (ZONE_INIT, (TEXT("Created receive thread %X\r\n"),
                          pSerialHead->pDispatchThread));     
    return(TRUE);
}

// ****************************************************************
//
//	@doc INTERNAL
//	@func		BOOL | StartDispatchThread | Stop thread, disable interrupt.
//
//	@parm 		ULONG  | pSerialHead
//
//	 @rdesc		TRUE if success, FALSE if failed.
//
BOOL
StopDispatchThread(
                  PHW_INDEP_INFO  pSerialHead
                  )

{
    HANDLE              pThisThread = GetCurrentThread();
    ULONG               priority256;

    /* If we have an interrupt handler thread, kill it */
    if ( pSerialHead->pDispatchThread ) {
        DEBUGMSG (ZONE_INIT, (TEXT("\r\nTrying to close dispatch thread\r\n")));        

        /* Set the priority of the dispatch thread to be equal to this one,
         * so that it shuts down before we free its memory. If this routine
         * has been called from SerialDllEntry then RxCharBuffer is set to
         * NULL and the dispatch thread is already dead, so just skip the
         * code which kills the thread.
         */
        priority256 = CeGetThreadPriority(pThisThread);
        CeSetThreadPriority(pSerialHead->pDispatchThread, priority256);        

        /* Signal the Dispatch thread to die.
         */
        pSerialHead->KillRxThread = 1;
        DEBUGMSG (ZONE_INIT, (TEXT("\r\nTrying to signal serial thread.\r\n")));
        SetEvent(pSerialHead->hSerialEvent);

        WaitForSingleObject(pSerialHead->hKillDispatchThread, 3000);
        Sleep(10);

        DEBUGMSG (ZONE_INIT, (TEXT("\r\nTrying to call CloseHandle\r\n")));

        CloseHandle(pSerialHead->pDispatchThread);
        pSerialHead->pDispatchThread = NULL;        
        DEBUGMSG (ZONE_INIT, (TEXT("\r\nReturned from CloseHandle\r\n")));
    }

    return(TRUE);
}


/*
 *  @doc INTERNAL
 *	@func		BOOL | ApplyDCB | Apply the current DCB.
 *
 *	This function will apply the current DCB settings to the device.
 *	It will also call to the PDD to set the PDD values.
 */
BOOL
ApplyDCB (PHW_INDEP_INFO pSerialHead, DCB *pDCB, BOOL fOpen)
{
    PHWOBJ          pHWObj      = pSerialHead->pHWObj;

    DEBUGMSG (ZONE_FUNCTION,
        (TEXT("+ApplyDCB, pSerialHead: 0x%X\r\n"), pSerialHead));

    if ( !pHWObj->pFuncTbl->HWSetDCB(pSerialHead->pHWHead,
                                     pDCB) ) {
        return(FALSE);
    }

    if ( !fOpen ) {
        return(TRUE);
    }

	// If PDD SetDCB was successful, save the supplied DCB and
	// configure port to match these settings.
    memcpy(&(pSerialHead->DCB), pDCB, sizeof(DCB));

    if ( pSerialHead->DCB.fDtrControl == DTR_CONTROL_DISABLE ) {
        pHWObj->pFuncTbl->HWClearDTR(pSerialHead->pHWHead);
    } else if ( pSerialHead->DCB.fDtrControl == DTR_CONTROL_ENABLE ) {
        pHWObj->pFuncTbl->HWSetDTR(pSerialHead->pHWHead);
    }

    if ( pSerialHead->DCB.fRtsControl == RTS_CONTROL_DISABLE ) {
        pHWObj->pFuncTbl->HWClearRTS(pSerialHead->pHWHead);
    } else if ( pSerialHead->DCB.fRtsControl == RTS_CONTROL_ENABLE ) {
        pHWObj->pFuncTbl->HWSetRTS(pSerialHead->pHWHead);
    }

    if ( pSerialHead->DCB.fDtrControl == DTR_CONTROL_HANDSHAKE ) {
        if ( (!pSerialHead->DtrFlow) && 
             (4*RxBytesAvail(pSerialHead) > (3*RxLength(pSerialHead))) ) {
            DEBUGMSG (ZONE_READ|ZONE_FLOW,
                      (TEXT("IOCTL:DTR_CONTROL_HANDSHAKE Clearing DTR\r\n")));
            pSerialHead->DtrFlow = 1;
            pHWObj->pFuncTbl->HWClearDTR(pSerialHead->pHWHead);
        } else {
            DEBUGMSG (ZONE_READ|ZONE_FLOW,
                      (TEXT("IOCTL:DTR_CONTROL_HANDSHAKE Setting DTR\r\n")));
            pSerialHead->DtrFlow = 0;
            pHWObj->pFuncTbl->HWSetDTR(pSerialHead->pHWHead);
        }
    }
    if ( pSerialHead->DCB.fRtsControl == RTS_CONTROL_HANDSHAKE ) {
        if ( (pSerialHead->RtsFlow) &&
             (4*RxBytesAvail(pSerialHead) > (3*RxLength(pSerialHead))) ) {
            DEBUGMSG (ZONE_READ|ZONE_FLOW,
                      (TEXT("IOCTL:RTS_CONTROL_HANDSHAKE Clearing RTS\r\n")));
            pSerialHead->RtsFlow = 1;
            pHWObj->pFuncTbl->HWClearRTS(pSerialHead->pHWHead);
        } else {
            DEBUGMSG (ZONE_READ|ZONE_FLOW,
                      (TEXT("IOCTL:RTS_CONTROL_HANDSHAKE Setting RTS\r\n")));
            pSerialHead->RtsFlow = 0;
            pHWObj->pFuncTbl->HWSetRTS(pSerialHead->pHWHead);
        }
    }

    if ( pSerialHead->DCB.fOutX || pSerialHead->DCB.fInX ) {
        pSerialHead->XFlow = 1;
    } else {
        pSerialHead->XFlow = 0;
    }

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-ApplyDCB\r\n")));

    return(TRUE);
}

// ****************************************************************
//
//	@doc EXTERNAL
//	@func		HANDLE | COM_INIT | Serial device initialization.
//
//	@parm 		ULONG  | Identifier | Port identifier.  The device loader
//				passes in the registry key that contains information
//				about the active device.
//
//	@remark		This routine is called at device load time in order
//	 			to perform any initialization.	 Typically the init
//				routine does as little as possible, postponing memory
//				allocation and device power-on to Open time.
//
//	 @rdesc		Returns a pointer to the serial head which is passed into 
//	 			the COM_OPEN and COM_DEINIT entry points as a device handle.
//
HANDLE
COM_Init(
        ULONG   Identifier
        )
{
    PVOID           pHWHead     = NULL;
    PHW_INDEP_INFO  pSerialHead = NULL;
    ULONG           HWBufferSize;

    /*
     *	INTERNAL: this routine initializes the hardware abstraction interface
     *	via HWInit(). It allocates a data structure representing this
     *	instantiation of the device. It also creates an event and initializes
     *	a critical section for receiving as well as registering the logical
     *	interrupt dwIntID with NK via InterruptInitialize. This call
     *	requires that the hardware dependent portion export apis that return
     *	the physical address of the receive buffer and the size of that buffer.
     *	Finally, it creates a buffer to act as an intermediate
     *	buffer when receiving.
     */
    DEBUGMSG (ZONE_INIT | ZONE_FUNCTION, (TEXT("+COM_Init\r\n")));

    // Allocate our control structure.
    pSerialHead  =  (PHW_INDEP_INFO)LocalAlloc(LPTR, sizeof(HW_INDEP_INFO));

    // Check that LocalAlloc did stuff ok too.
    if ( !pSerialHead ) {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR,
                 (TEXT("Error allocating memory for pSerialHead, COM_Init failed\n\r")));
        return(NULL);
    }

    DEBUGMSG (ZONE_INIT | ZONE_FUNCTION,
        (TEXT("COM_Init - pSerialHead: 0x%X\r\n"), pSerialHead));

    // Initially, open list is empty.
    InitializeListHead( &pSerialHead->OpenList );
    InitializeCriticalSection(&(pSerialHead->OpenCS));
    pSerialHead->pAccessOwner = NULL;
    pSerialHead->fEventMask = 0;

    // Init CommTimeouts.
    pSerialHead->CommTimeouts.ReadIntervalTimeout = VGD_READ_TIMEOUT;
    pSerialHead->CommTimeouts.ReadTotalTimeoutMultiplier = VGD_READ_TIMEOUT_MULTIPLIER;
    pSerialHead->CommTimeouts.ReadTotalTimeoutConstant = VGD_READ_TIMEOUT_CONSTANT;
    pSerialHead->CommTimeouts.WriteTotalTimeoutMultiplier = VGD_WRITE_TIMEOUT_MULTIPLIER;
    pSerialHead->CommTimeouts.WriteTotalTimeoutConstant = VGD_WRITE_TIMEOUT_CONSTANT;

    /* Create tx and rx events and stash in global struct field. Check return.
     */
    pSerialHead->hSerialEvent = CreateEvent(0, FALSE, FALSE, NULL);
    pSerialHead->hKillDispatchThread = CreateEvent(0, FALSE, FALSE, NULL);
    pSerialHead->hTransmitEvent = CreateEvent(0, FALSE, FALSE, NULL);
    pSerialHead->hReadEvent = CreateEvent(0, FALSE, FALSE, NULL);

    if ( !pSerialHead->hSerialEvent || !pSerialHead->hKillDispatchThread ||
         !pSerialHead->hTransmitEvent || !pSerialHead->hReadEvent ) {
        DEBUGMSG(ZONE_ERROR | ZONE_INIT,
                 (TEXT("Error creating event, COM_Init failed\n\r")));
        LocalFree(pSerialHead);
        return(NULL);
    }

    /* Initialize the critical sections that will guard the parts of
     * the receive and transmit buffers.
     */
    InitializeCriticalSection(&(pSerialHead->ReceiveCritSec1));
    InitializeCriticalSection(&(pSerialHead->TransmitCritSec1));

    pSerialHead->Priority256 = VGD_DEFAULT_CE_THREAD_PRIORITY;

    // Initialize hardware dependent data.
    pSerialHead->pHWObj = GetSerialObject(0);
    if ( !pSerialHead->pHWObj ) {
        DEBUGMSG(ZONE_ERROR | ZONE_INIT,
                 (TEXT("Error in GetSerialObject, COM_Init failed\n\r")));
        LocalFree(pSerialHead);
        return(NULL);
    }

    DEBUGMSG (ZONE_INIT, (TEXT("About to call HWInit(%s,0x%X)\r\n"),
                          Identifier, pSerialHead));
    pHWHead = pSerialHead->pHWObj->pFuncTbl->HWInit(Identifier, pSerialHead, pSerialHead->pHWObj);
    pSerialHead->pHWHead = pHWHead;

    /* Check that HWInit did stuff ok.  From here on out, call Deinit function
     * when things fail.
     */
    if ( !pHWHead ) {
        DEBUGMSG (ZONE_INIT | ZONE_ERROR,
                  (TEXT("Hardware doesn't init correctly, COM_Init failed\r\n")));
        COM_Deinit(pSerialHead);
        return(NULL);
    }
    DEBUGMSG (ZONE_INIT,
              (TEXT("Back from hardware init\r\n")));

    // Allocate at least twice the hardware buffer size so we have headroom
    HWBufferSize        = 2 * pSerialHead->pHWObj->pFuncTbl->HWGetRxBufferSize(pHWHead);

    // Init rx buffer and buffer length here.
    pSerialHead->RxBufferInfo.Length =
    HWBufferSize > RX_BUFFER_SIZE ? HWBufferSize:RX_BUFFER_SIZE;

    pSerialHead->RxBufferInfo.RxCharBuffer =
    LocalAlloc(LPTR, pSerialHead->RxBufferInfo.Length);

    if ( !pSerialHead->RxBufferInfo.RxCharBuffer ) {
        DEBUGMSG(ZONE_INIT|ZONE_ERROR,
                 (TEXT("Error allocating receive buffer, COM_Init failed\n\r")));
        COM_Deinit(pSerialHead);
        return(NULL);
    }

    DEBUGMSG (ZONE_INIT, (TEXT("RxHead init'ed\r\n")));

    RxResetFifo(pSerialHead);

    InitializeCriticalSection(&(pSerialHead->RxBufferInfo.CS));
    InitializeCriticalSection(&(pSerialHead->TxBufferInfo.CS));
    DEBUGMSG (ZONE_INIT, (TEXT("RxBuffer init'ed with start at %x\r\n"),
                          pSerialHead->RxBufferInfo.RxCharBuffer));

    if ( pSerialHead->pHWObj->BindFlags & THREAD_AT_INIT ) {
        // Hook the interrupt and start the associated thread.
        if ( ! StartDispatchThread( pSerialHead ) ) {
            // Failed on InterruptInitialize or CreateThread.  Bail.
            COM_Deinit(pSerialHead);
            return(NULL);        
        }

    }

    // OK, now that everything is ready on our end, give the PDD
    // one last chance to init interrupts, etc.
    (void) pSerialHead->pHWObj->pFuncTbl->HWPostInit( pHWHead );

    DEBUGMSG (ZONE_INIT | ZONE_FUNCTION, (TEXT("-COM_Init\r\n")));
    return(pSerialHead);
}

/*
 @doc EXTERNAL
 @func		HANDLE | COM_Open | Serial port driver initialization.
 *	Description: This routine must be called by the user to open the
 *	serial device. The HANDLE returned must be used by the application in
 *	all subsequent calls to the serial driver. This routine starts the thread
 *	which handles the serial events.
 *	Exported to users.
 *
 @rdesc This routine returns a HANDLE representing the device.
 */
HANDLE
COM_Open(
        HANDLE  pHead,          // @parm Handle returned by COM_Init.
        DWORD   AccessCode,     // @parm access code.
        DWORD   ShareMode       // @parm share mode - Not used in this driver.
        )
{
    PHW_INDEP_INFO  pSerialHead = (PHW_INDEP_INFO)pHead;
    PHW_OPEN_INFO   pOpenHead;
    PHWOBJ          pHWObj      = pSerialHead->pHWObj;

    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("+COM_Open handle x%X, access x%X, share x%X\r\n"),
                                        pHead, AccessCode, ShareMode));

    // HACK!!! All users connect in READ and WRITE mode
    if(0 == (AccessCode & VGD_AC_CONFIGURATOR))
        AccessCode |= GENERIC_READ | GENERIC_WRITE;

    // Return NULL if SerialInit failed.
    if ( !pSerialHead ) {
        DEBUGMSG (ZONE_OPEN|ZONE_ERROR,
                  (TEXT("Open attempted on uninited device!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);        
        return(NULL);
    }
    
    if (AccessCode & DEVACCESS_BUSNAMESPACE ) {
        AccessCode &=~(GENERIC_READ |GENERIC_WRITE|GENERIC_EXECUTE|GENERIC_ALL);
    }

    // Return NULL if opening with access & someone else already has
    if ( 0 == (AccessCode & VGD_AC_CONFIGURATOR) && (AccessCode & (GENERIC_READ | GENERIC_WRITE)) &&
         pSerialHead->pAccessOwner ) {
        DEBUGMSG (ZONE_OPEN|ZONE_ERROR,
                  (TEXT("Open requested access %x, handle x%X already has x%X!\r\n"),
                   AccessCode, pSerialHead->pAccessOwner,
                   pSerialHead->pAccessOwner->AccessCode));
        SetLastError(ERROR_INVALID_ACCESS);        
        return(NULL);
    }

    // OK, lets allocate an open structure
    pOpenHead    =  (PHW_OPEN_INFO)LocalAlloc(LPTR, sizeof(HW_OPEN_INFO));
    if ( !pOpenHead ) {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR,
                 (TEXT("Error allocating memory for pOpenHead, COM_Open failed\n\r")));
        return(NULL);
    }

    // Init the structure 
    pOpenHead->pSerialHead = pSerialHead;  // pointer back to our parent
    pOpenHead->StructUsers = 0;
    pOpenHead->AccessCode = AccessCode;
    pOpenHead->ShareMode = ShareMode;
    pOpenHead->CommEvents.hCommEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    pOpenHead->CommEvents.fEventMask = 0;
    pOpenHead->CommEvents.fEventData = 0;
    pOpenHead->CommEvents.fAbort = 0;
    InitializeCriticalSection(&(pOpenHead->CommEvents.EventCS));

    // if we have access permissions, note it in pSerialhead
    if ( AccessCode & (GENERIC_READ | GENERIC_WRITE) ) {
        DEBUGMSG(ZONE_INIT|ZONE_CLOSE,
                 (TEXT("COM_Open: Access permission handle granted x%X\n\r"),
                  pOpenHead));
        pSerialHead->pAccessOwner = pOpenHead;
    }

    // add this open entry to list of open entries.
    // Note that we hold the open CS for the duration of the routine since
    // all of our state info is in flux during this time.  In particular,
    // without the CS is would be possible for an open & close to be going on
    // simultaneously and have bad things happen like spinning a new event
    // thread before the old one was gone, etc.
    EnterCriticalSection(&(pSerialHead->OpenCS));
    InsertHeadList(&pSerialHead->OpenList,
                   &pOpenHead->llist);

    // We do special for Power Manger and Device Manager.
    if (pOpenHead->AccessCode &  DEVACCESS_BUSNAMESPACE ) {
        // OK, We do not need initialize pSerailHead and start any thread. return the handle now.
        LeaveCriticalSection(&(pSerialHead->OpenCS));
        DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("-COM_Open handle x%X, x%X, Ref x%X\r\n"),
                                        pOpenHead, pOpenHead->pSerialHead, pSerialHead->OpenCnt));
        return(pOpenHead);
        
    }
    //

    // If port not yet opened, we need to do some init
    if ( ! pSerialHead->OpenCnt ) {
        DEBUGMSG(ZONE_INIT|ZONE_OPEN,
                 (TEXT("COM_Open: First open : Do Init x%X\n\r"),
                  pOpenHead));

        if ( pSerialHead->pHWObj->BindFlags & THREAD_AT_OPEN ) {
            DEBUGMSG(ZONE_INIT|ZONE_OPEN,
                     (TEXT("COM_Open: Starting DispatchThread x%X\n\r"),
                      pOpenHead));
            // Hook the interrupt and start the associated thread.
            if ( ! StartDispatchThread( pSerialHead ) ) {
                // Failed on InterruptInitialize or CreateThread.  Bail.
                DEBUGMSG(ZONE_INIT|ZONE_OPEN,
                         (TEXT("COM_Open: Failed StartDispatchThread x%X\n\r"),
                          pOpenHead));
                goto OpenFail;
            }
        }

        pSerialHead->RxBytes = 0;
        pSerialHead->TxBytes = 0;
        pSerialHead->TxBytesPending = 0;
        pSerialHead->DroppedBytesMDD = 0;
        pSerialHead->DroppedBytesPDD = 0;

        pSerialHead->DCB.DCBlength  = sizeof(DCB);
        pSerialHead->DCB.BaudRate   = 4800;
        pSerialHead->DCB.fBinary    = TRUE;
        pSerialHead->DCB.fParity    = FALSE;

        pSerialHead->DCB.fOutxCtsFlow = FALSE;
        pSerialHead->DCB.fOutxDsrFlow = FALSE;
        pSerialHead->DCB.fDtrControl = DTR_CONTROL_ENABLE;
        pSerialHead->DCB.fDsrSensitivity = FALSE;
        pSerialHead->DCB.fTXContinueOnXoff = FALSE;
        pSerialHead->DCB.fOutX      = FALSE;
        pSerialHead->DCB.fInX       = FALSE;
        pSerialHead->DCB.fErrorChar = FALSE; //NOTE: ignored
        pSerialHead->DCB.fNull      = FALSE; //NOTE: ignored
        pSerialHead->DCB.fRtsControl = RTS_CONTROL_ENABLE;
        pSerialHead->DCB.fAbortOnError = FALSE; //NOTE: ignored

        pSerialHead->DCB.XonLim     = 512;
        pSerialHead->DCB.XoffLim    = 128;

        pSerialHead->DCB.ByteSize   = 8;
        pSerialHead->DCB.Parity     = NOPARITY;
        pSerialHead->DCB.StopBits   = ONESTOPBIT;

        pSerialHead->DCB.XonChar    = X_ON_CHAR;
        pSerialHead->DCB.XoffChar   = X_OFF_CHAR;
        pSerialHead->DCB.ErrorChar  = ERROR_CHAR;
        pSerialHead->DCB.EofChar    = E_OF_CHAR;
        pSerialHead->DCB.EvtChar    = EVENT_CHAR;

        pSerialHead->StopXmit = 0;
        pSerialHead->SentXoff = 0;
        pSerialHead->DtrFlow = 0;
        pSerialHead->RtsFlow = 0;

        ApplyDCB (pSerialHead, &(pSerialHead->DCB), FALSE);

        pHWObj->pFuncTbl->HWSetCommTimeouts(pSerialHead->pHWHead,
                                            &(pSerialHead->CommTimeouts));

        if ( !pHWObj->pFuncTbl->HWOpen(pSerialHead->pHWHead) ) {
            DEBUGMSG (ZONE_OPEN|ZONE_ERROR, (TEXT("HW Open failed.\r\n")));
            goto OpenFail;
        }

        RxEnterCS( pSerialHead );

        pHWObj->pFuncTbl->HWPurgeComm(pSerialHead->pHWHead, PURGE_RXCLEAR);

        memset( pSerialHead->RxBufferInfo.RxCharBuffer, 0, pSerialHead->RxBufferInfo.Length );
        RxResetFifo( pSerialHead );

        RxLeaveCS( pSerialHead );

        if ( pHWObj->BindFlags & THREAD_IN_MDD ) {
            CeSetThreadPriority(pSerialHead->pDispatchThread,
                                pSerialHead->Priority256);
        }
    }

    ++(pSerialHead->OpenCnt);

    // OK, we are finally back in a stable state.  Release the CS.
    LeaveCriticalSection(&(pSerialHead->OpenCS));

    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("-COM_Open handle x%X, x%X, Ref x%X\r\n"),
                                        pOpenHead, pOpenHead->pSerialHead, pSerialHead->OpenCnt));

    return(pOpenHead);

    OpenFail :
    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("-COM_Open handle x%X, x%X, Ref x%X\r\n"),
                                        NULL, pOpenHead->pSerialHead, pSerialHead->OpenCnt));

    SetLastError(ERROR_OPEN_FAILED);

    // If this was the handle with access permission, remove pointer
    if ( pOpenHead == pSerialHead->pAccessOwner )
        pSerialHead->pAccessOwner = NULL;

    // Remove the Open entry from the linked list
    RemoveEntryList(&pOpenHead->llist);

    // OK, everything is stable so release the critical section
    LeaveCriticalSection(&(pSerialHead->OpenCS));

    // Free all data allocated in open
    if ( pOpenHead->CommEvents.hCommEvent )
        CloseHandle(pOpenHead->CommEvents.hCommEvent);
    DeleteCriticalSection(&(pOpenHead->CommEvents.EventCS));
    LocalFree( pOpenHead );

    return(NULL);


}

// ****************************************************************
//
//	@doc EXTERNAL
//
//	@func BOOL	| COM_PreClose | pre-close the serial device.
//
//	@parm DWORD | pHead		| Context pointer returned from COM_Open
//
//	@rdesc TRUE if success; FALSE if failure
//
//	@remark This routine is called by the device manager to close the device.
//			
//
//
void COM_PreClose(PHW_OPEN_INFO pOpenHead)
{
    PHW_INDEP_INFO  pSerialHead = pOpenHead->pSerialHead;
    PHWOBJ          pHWObj;

    if ( !pSerialHead ) {
        DEBUGMSG (ZONE_ERROR, (TEXT("!!COM_PreClose: pSerialHead == NULL!!\r\n")));
        return;
    }
    pHWObj = (PHWOBJ)pSerialHead->pHWObj;

    // Use the OpenCS to make sure we don't collide with an in-progress open.
    EnterCriticalSection(&(pSerialHead->OpenCS));
    if (!(pOpenHead->AccessCode & DEVACCESS_BUSNAMESPACE)) {
        if ( pSerialHead->OpenCnt ) {
            DEBUGMSG(ZONE_INIT|ZONE_CLOSE,
                     (TEXT("COM_PreClose: %d users in MDD functions\n\r"),pOpenHead->StructUsers));

            // For any open handle, we must free pending waitcommevents
            EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));
            pOpenHead->CommEvents.fEventMask = 0;
            pOpenHead->CommEvents.fAbort = 1;
            SetEvent(pOpenHead->CommEvents.hCommEvent);
            LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));

            // And only for the handle with access permissions do we
            // have to worry about read, write, etc being blocked.
            if ( pOpenHead->AccessCode & (GENERIC_READ | GENERIC_WRITE) ) {
                pSerialHead->fAbortRead=1;
                SetEvent(pSerialHead->hReadEvent);
                pSerialHead->fAbortTransmit=1;
                SetEvent(pSerialHead->hTransmitEvent);
            }

            DEBUGMSG(ZONE_CLOSE|ZONE_INIT|ZONE_ERROR,
                     (TEXT("COM_PreClose: serial users to exit, %d left\n\r"),
                      pOpenHead->StructUsers));

        } else {
            DEBUGMSG (ZONE_ERROR, (TEXT("!!PreClose of non-open serial port\r\n")));
            SetLastError(ERROR_INVALID_HANDLE);        
        }
    }
    // OK, other inits/opens can go ahead.
    LeaveCriticalSection(&(pSerialHead->OpenCS));

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("-COM_PreClose\r\n")));
}

// ****************************************************************
//
//	@doc EXTERNAL
//
//	@func BOOL	| COM_Close | close the serial device.
//
//	@parm DWORD | pHead		| Context pointer returned from COM_Open
//
//	@rdesc TRUE if success; FALSE if failure
//
//	@remark This routine is called by the device manager to close the device.
//			
//
//
BOOL
COM_Close(PHW_OPEN_INFO pOpenHead)
{
    PHW_INDEP_INFO  pSerialHead = pOpenHead->pSerialHead;
    PHWOBJ          pHWObj;
    // int i;
    BOOL            RetCode = TRUE;

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("+COM_Close\r\n")));

    if ( !pSerialHead ) {
        DEBUGMSG (ZONE_ERROR, (TEXT("!!COM_Close: pSerialHead == NULL!!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    pHWObj = (PHWOBJ)pSerialHead->pHWObj;

    // Use the OpenCS to make sure we don't collide with an in-progress open.
    EnterCriticalSection(&(pSerialHead->OpenCS));

    // We do special for Power Manger and Device Manager.
    if (pOpenHead->AccessCode & DEVACCESS_BUSNAMESPACE) {


        // Remove the entry from the linked list
        RemoveEntryList(&pOpenHead->llist);

        // Free all data allocated in open
        DeleteCriticalSection(&(pOpenHead->CommEvents.EventCS));
        if ( pOpenHead->CommEvents.hCommEvent )
            CloseHandle(pOpenHead->CommEvents.hCommEvent);
        LocalFree( pOpenHead );
    }
    else
    if ( pSerialHead->OpenCnt ) {
        --(pSerialHead->OpenCnt);

        DEBUGMSG (1,
                  (TEXT("COM_Close: (%d handles) total RX %d, total TX %d, dropped (mdd, pdd) %d,%d\r\n"),
                   pSerialHead->OpenCnt, pSerialHead->RxBytes, pSerialHead->TxBytes, pSerialHead->DroppedBytesMDD, pSerialHead->DroppedBytesPDD));

        // In multi open case, do we need to restore state later on or something????
        if ( pHWObj && pSerialHead->OpenCnt==0 && (pHWObj->BindFlags & THREAD_IN_MDD) &&
            pSerialHead->pDispatchThread ) {
            SetThreadPriority(pSerialHead->pDispatchThread,
                              THREAD_PRIORITY_NORMAL);
        }

        // If we are closing the last open handle, then close PDD also
        if ( !pSerialHead->OpenCnt ) {
            DEBUGMSG (ZONE_CLOSE, (TEXT("About to call HWClose\r\n")));
            if ( pHWObj )
                pHWObj->pFuncTbl->HWClose(pSerialHead->pHWHead);
            DEBUGMSG (ZONE_CLOSE, (TEXT("Returned from HWClose\r\n")));

            // And if thread was spun in open, kill it now.
            if ( pSerialHead->pHWObj->BindFlags & THREAD_AT_OPEN ) {
                DEBUGMSG (ZONE_CLOSE, (TEXT("COM_Close : Stopping Dispatch Thread\r\n")));
                StopDispatchThread( pSerialHead );
            }
        }


        // If this was the handle with access permission, remove pointer
        if ( pOpenHead == pSerialHead->pAccessOwner ) {
            DEBUGMSG(ZONE_INIT|ZONE_CLOSE,
                     (TEXT("COM_Close: Closed access owner handle\n\r"),
                      pOpenHead));

            pSerialHead->pAccessOwner = NULL;
        }

        // Remove the entry from the linked list
        RemoveEntryList(&pOpenHead->llist);

        // Free all data allocated in open
        DeleteCriticalSection(&(pOpenHead->CommEvents.EventCS));
        if ( pOpenHead->CommEvents.hCommEvent )
            CloseHandle(pOpenHead->CommEvents.hCommEvent);
        LocalFree( pOpenHead );
    } else {
        DEBUGMSG (ZONE_ERROR, (TEXT("!!Close of non-open serial port\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);        
        RetCode = FALSE;
    }

    // OK, other inits/opens can go ahead.
    LeaveCriticalSection(&(pSerialHead->OpenCS));

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("-COM_Close\r\n")));
    return(RetCode);
}

// Routine to handle the PROCESS_EXITING flag.  Should let free any threads blocked on pOpenHead,
// so they can be killed and the process closed.

BOOL
ProcessExiting(PHW_OPEN_INFO pOpenHead)
{
    PHW_INDEP_INFO  pSerialHead = pOpenHead->pSerialHead;
    //PHWOBJ          pHWObj;
    //int i;
    BOOL            RetCode = TRUE;

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("+ProcessExiting\r\n")));

    if ( !pSerialHead ) {
        DEBUGMSG (ZONE_ERROR, (TEXT("!!ProcessExiting: pSerialHead == NULL!!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }
    else {
        COM_PreClose(pOpenHead);
        DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("-ProcessExiting\r\n")));
        return(TRUE);
    }
}
/*
 @doc EXTERNAL
 @func	BOOL | COM_PreDeinit | Pre-De-initialize serial port.
 @parm DWORD | pSerialHead | Context pointer returned from COM_Init
 *
 @rdesc None.
 */
BOOL
COM_PreDeinit(PHW_INDEP_INFO pSerialHead)
{
    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("+COM_PreDeinit\r\n")));

    if ( !pSerialHead ) {
        /* Can't do much without this */
        DEBUGMSG (ZONE_INIT|ZONE_ERROR,
                  (TEXT("COM_PreDeinit can't find pSerialHead\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }
    /*
    ** Call PreClose, if we have a user.  Note that this call will ensure that
    ** all users are out of the serial routines before it returns.
    */
    if ( pSerialHead->OpenCnt ) {
        PLIST_ENTRY     pEntry;
        PHW_OPEN_INFO   pOpenHead;
        EnterCriticalSection(&(pSerialHead->OpenCS));
        pEntry = pSerialHead->OpenList.Flink;
        while ( pEntry != &pSerialHead->OpenList ) {
            pOpenHead = CONTAINING_RECORD( pEntry, HW_OPEN_INFO, llist);
            pEntry = pEntry->Flink;  // advance to next 

            DEBUGMSG (ZONE_INIT | ZONE_CLOSE, (TEXT(" PreDeinit -Pre Closing Handle 0x%X\r\n"),
                                               pOpenHead ));
            COM_PreClose(pOpenHead);
        }
        LeaveCriticalSection(&(pSerialHead->OpenCS));
    }

    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("-COM_PreDeinit\r\n")));
    return TRUE;
}
/*
 @doc EXTERNAL
 @func	BOOL | COM_Deinit | De-initialize serial port.
 @parm DWORD | pSerialHead | Context pointer returned from COM_Init
 *
 @rdesc None.
 */
BOOL
COM_Deinit(PHW_INDEP_INFO pSerialHead)
{
    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("+COM_Deinit\r\n")));

    if ( !pSerialHead ) {
        /* Can't do much without this */
        DEBUGMSG (ZONE_INIT|ZONE_ERROR,
                  (TEXT("COM_Deinit can't find pSerialHead\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    // If we have an interrupt handler thread, kill it
    if ( pSerialHead->pHWObj->BindFlags & THREAD_IN_MDD ) {
        StopDispatchThread( pSerialHead );
    }

    /*
    ** Call close, if we have a user.  Note that this call will ensure that
    ** all users are out of the serial routines before it returns, so we can
    ** go ahead and free our internal memory.
    */
    EnterCriticalSection(&(pSerialHead->OpenCS));
    if ( pSerialHead->OpenCnt ) {
        PLIST_ENTRY     pEntry;
        PHW_OPEN_INFO   pOpenHead;

        pEntry = pSerialHead->OpenList.Flink;
        while ( pEntry != &pSerialHead->OpenList ) {
            pOpenHead = CONTAINING_RECORD( pEntry, HW_OPEN_INFO, llist);
            pEntry = pEntry->Flink;  // advance to next 

            DEBUGMSG (ZONE_INIT | ZONE_CLOSE, (TEXT(" Deinit - Closing Handle 0x%X\r\n"),
                                               pOpenHead ));
            COM_Close(pOpenHead);
        }
    }
    LeaveCriticalSection(&(pSerialHead->OpenCS));

    /* Free our resources */
    if ( pSerialHead->hSerialEvent )
        CloseHandle(pSerialHead->hSerialEvent);
    if ( pSerialHead->hKillDispatchThread )
        CloseHandle(pSerialHead->hKillDispatchThread);
    if ( pSerialHead->hTransmitEvent )
        CloseHandle(pSerialHead->hTransmitEvent);
    if ( pSerialHead->hReadEvent )
        CloseHandle(pSerialHead->hReadEvent);

    DeleteCriticalSection(&(pSerialHead->ReceiveCritSec1));
    DeleteCriticalSection(&(pSerialHead->TransmitCritSec1));
    DeleteCriticalSection(&(pSerialHead->RxBufferInfo.CS));
    DeleteCriticalSection(&(pSerialHead->TxBufferInfo.CS));
    DeleteCriticalSection(&(pSerialHead->OpenCS));

    if ( pSerialHead->RxBufferInfo.RxCharBuffer )
        LocalFree(pSerialHead->RxBufferInfo.RxCharBuffer);

    /* Now, call HW specific deinit function */
    if (pSerialHead->pHWHead && pSerialHead->pHWObj && pSerialHead->pHWObj->pFuncTbl ) {
        DEBUGMSG (ZONE_INIT, (TEXT("About to call HWDeinit\r\n")));
        pSerialHead->pHWObj->pFuncTbl->HWDeinit(pSerialHead->pHWHead);
        DEBUGMSG (ZONE_INIT, (TEXT("Returned from HWDeinit\r\n")));
    }

    LocalFree(pSerialHead);

    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("-COM_Deinit\r\n")));
    return(TRUE);
}


/*
   @doc EXTERNAL
   @func	ULONG | COM_Read | Allows application to receive characters from
   *	serial port. This routine sets the buffer and bufferlength to be used
   *	by the reading thread. It also enables reception and controlling when
   *	to return to the user. It writes to the referent of the fourth argument
   *	the number of bytes transacted. It returns the status of the call.
   *
   *	Exported to users.
   @rdesc This routine returns: -1 if error, or number of bytes read.
   */
ULONG
COM_Read(
        HANDLE      pHead,          //@parm [IN]	 HANDLE returned by COM_Open   
        PUCHAR      pTargetBuffer,  //@parm [IN,OUT] Pointer to valid memory.	  
        ULONG       BufferLength    //@parm [IN]	 Size in bytes of pTargetBuffer.
        )
{
    PHW_OPEN_INFO   pOpenHead = (PHW_OPEN_INFO)pHead;
    PHW_INDEP_INFO  pSerialHead;// = pOpenHead->pSerialHead;
    PHW_VTBL        pFuncTbl;//       = pSerialHead->pHWObj->pFuncTbl;
    PVOID           pHWHead ;//       = pSerialHead->pHWHead;
    ULONG           Ticks;
    ULONG           Timeout;
    ULONG           BytesRead = 0;
    ULONG           IntervalTimeout;    // The interval timeout
    ULONG           AddIntervalTimeout;
    ULONG           TotalTimeout;       // The Total Timeout
    ULONG           TimeSpent = 0;      // How much time have we been waiting?
    ULONG           Len;

    DEBUGMSG (ZONE_USR_READ|ZONE_FUNCTION,
        (TEXT("+COM_READ(0x%X,0x%X,%d)\r\n"),
        pHead, pTargetBuffer, BufferLength));
    if (pOpenHead==NULL) {
        DEBUGMSG (ZONE_USR_READ|ZONE_ERROR, (TEXT("COM_READ, Wrong Handle\r\n") ));
        SetLastError (ERROR_INVALID_HANDLE);
        return(ULONG)-1;
    }
    pSerialHead = pOpenHead->pSerialHead;
    // Check to see that the call is valid.
    if ( !pSerialHead || !pSerialHead->OpenCnt ) {
        DEBUGMSG (ZONE_USR_READ|ZONE_ERROR,
                  (TEXT("COM_READ, device not open\r\n") ));
        SetLastError (ERROR_INVALID_HANDLE);
        return(ULONG)-1;
    }
    pFuncTbl  = pSerialHead->pHWObj->pFuncTbl;
    pHWHead   = pSerialHead->pHWHead;

    DEBUGMSG (ZONE_USR_READ,
        (TEXT("COM_Read - RxBuffRead: 0x%X\r\n"), RxBuffRead(pSerialHead)));

    // Make sure the caller has access permissions
    if ( !(pOpenHead->AccessCode & GENERIC_READ) ) {
        DEBUGMSG(ZONE_USR_READ|ZONE_ERROR,
                 (TEXT("COM_Read: Access permission failure x%X\n\r"),
                  pOpenHead->AccessCode));
        SetLastError (ERROR_INVALID_ACCESS);
        return(ULONG)-1;
    }

#ifdef DEBUG
    if ( IsBadWritePtr(pTargetBuffer, BufferLength) ) {
        BytesRead = (ULONG)-1;
        SetLastError(ERROR_INVALID_PARAMETER);        
        return(ULONG)-1;
    }
#endif

    COM_INC_USAGE_CNT(pOpenHead);

    /* Practice safe threading.
     */
    EnterCriticalSection(&(pSerialHead->ReceiveCritSec1));
    pSerialHead->fAbortRead = 0;

    /* Compute total time to wait. Take product and add constant.
     */
    if ( MAXDWORD != pSerialHead->CommTimeouts.ReadTotalTimeoutMultiplier ) {
        TotalTimeout = pSerialHead->CommTimeouts.ReadTotalTimeoutMultiplier*BufferLength +
                       pSerialHead->CommTimeouts.ReadTotalTimeoutConstant;
        // Because we are using FIFO and water level is set to 8, we have to do following
        AddIntervalTimeout=pSerialHead->CommTimeouts.ReadTotalTimeoutMultiplier*8;
    } else {
        TotalTimeout = pSerialHead->CommTimeouts.ReadTotalTimeoutConstant;
        AddIntervalTimeout=0;
    }
    IntervalTimeout = pSerialHead->CommTimeouts.ReadIntervalTimeout;
    if ((IntervalTimeout < MAXDWORD  - AddIntervalTimeout) && (IntervalTimeout != 0)) {
        IntervalTimeout +=AddIntervalTimeout;
    };

    DEBUGMSG (ZONE_USR_READ, (TEXT("TotalTimeout:%d\r\n"), TotalTimeout));

    while ( BufferLength ) {
        DEBUGMSG (ZONE_USR_READ,
                  (TEXT("Top of Loop Fifo(R=%d,W=%d,L=%d,BA=%d)\r\n"),
                   RxRead(pSerialHead), RxWrite(pSerialHead),
                   RxLength(pSerialHead),
                   RxBytesAvail(pSerialHead)));
        if ( RxBytesAvail(pSerialHead) ) {
            RxEnterCS(pSerialHead);
            // Copy the data over
            // This only copies the continous portion, This will cause a loop
            // if the receive data spans the end of the buffer.
            Len = MIN(RxBytesAvail(pSerialHead),
                      RxLength(pSerialHead)-RxRead(pSerialHead));
            Len = MIN(Len, BufferLength);
            DEBUGMSG (ZONE_USR_READ, (TEXT("About to copy %d bytes\r\n"), Len));
            memcpy (pTargetBuffer, RxBuffRead(pSerialHead), Len);
            // Update Fifo info
            RxRead(pSerialHead) = 
                (RxRead(pSerialHead)+ Len<RxLength(pSerialHead)? RxRead(pSerialHead)+Len: RxRead(pSerialHead)+Len-RxLength(pSerialHead));

            // Update all the pointers.
            BufferLength -= Len;
            pTargetBuffer += Len;
            BytesRead += Len;
            RxLeaveCS(pSerialHead);
        } else {
            // Wait for a serial event?
            if ( IntervalTimeout == MAXDWORD){  // Special Case see Remarks of COMMTIMEOUTS 
                if (TotalTimeout == 0) 
                    // For some reason this means don't wait.
                    break;
                else
                if (BytesRead!=0) // There is data in the buffer or has been readed.
                    break;
            }
            Timeout=(TotalTimeout!=0?TotalTimeout:MAXDWORD);
            // Total timeout is valid
            if ( TimeSpent >= Timeout ) {
                // Timed out.
                break;
            }
            Timeout -= TimeSpent;
            // On first byte we only use interval timeout
            // on subsequent we use minimum of Interval and Timeout
            if ( BytesRead) {
                Timeout = MIN(Timeout, (IntervalTimeout!=0?IntervalTimeout:MAXDWORD));
            }
            Ticks = GetTickCount();
            DEBUGMSG (ZONE_USR_READ, (TEXT("About to wait %dms\r\n"), Timeout));

            if ( WAIT_TIMEOUT == WaitForSingleObject( pSerialHead->hReadEvent, Timeout ) ) {
                // Timeout
                break;
            }
            // Since ticks is a ULONG this handles wrap.
            Ticks = GetTickCount() - Ticks;
            TimeSpent += Ticks;

            // In the absense of WaitForMultipleObjects, we use flags to
            // handle errors/aborts. Check for aborts or asynchronous closes.
            if ( pSerialHead->fAbortRead ) {
                DEBUGMSG(ZONE_USR_READ,(TEXT("COM_Read - Aborting read\r\n")));
                break;
            }

            if ( !pSerialHead->OpenCnt ) {
                DEBUGMSG(ZONE_USR_READ|ZONE_ERROR,
                         (TEXT("COM_Read - device was closed\n\r")));
                SetLastError(ERROR_INVALID_HANDLE);
                break;
            }
        }

        // Are we below the SW flow control limits?
        if ( pSerialHead->DCB.fInX && pSerialHead->SentXoff &&
             (pSerialHead->DCB.XoffLim <
              (RxLength(pSerialHead) - RxBytesAvail(pSerialHead))) ) {
            PHWOBJ  pHWObj  = pSerialHead->pHWObj;

            DEBUGMSG (ZONE_FLOW, (TEXT("Sending XON\r\n")));
            pSerialHead->SentXoff = 0;
            if ( !pSerialHead->DCB.fTXContinueOnXoff ) {
                pSerialHead->StopXmit = 0;
            }
            pHWObj->pFuncTbl->HWXmitComChar(pSerialHead->pHWHead,
                                            pSerialHead->DCB.XonChar);
        }

        // Are we below the HW flow control limits?
        if ( 2*RxBytesAvail(pSerialHead) < RxLength(pSerialHead) ) {
            // When buffer is less then 1/2 full we set RTS/DTR
            if ( pSerialHead->RtsFlow &&
                 (pSerialHead->DCB.fRtsControl == RTS_CONTROL_HANDSHAKE) ) {
                DEBUGMSG (ZONE_USR_READ|ZONE_FLOW,
                          (TEXT("RTS_CONTROL_HANDSHAKE Setting RTS\r\n")));
                pSerialHead->RtsFlow = 0;
                pFuncTbl->HWSetRTS(pHWHead);
            }
            if ( pSerialHead->DtrFlow && 
                 (pSerialHead->DCB.fDtrControl == DTR_CONTROL_HANDSHAKE) ) {
                DEBUGMSG (ZONE_USR_READ|ZONE_FLOW,
                          (TEXT("DTR_CONTROL_HANDSHAKE Setting DTR\r\n")));
                pSerialHead->DtrFlow = 0;
                pFuncTbl->HWSetDTR(pHWHead);
            }
        }

    }

    DEBUGMSG (ZONE_USR_READ, (TEXT("ReceiveBytes exiting\r\n")));

    LeaveCriticalSection(&(pSerialHead->ReceiveCritSec1));

    DEBUGMSG (ZONE_USR_READ|ZONE_FUNCTION,
              (TEXT("-COM_READ: returning %d (total %d, dropped %d,%d)\r\n"),
               BytesRead, pSerialHead->RxBytes, pSerialHead->DroppedBytesMDD,pSerialHead->DroppedBytesPDD));

    COM_DEC_USAGE_CNT(pOpenHead);

    return(BytesRead);
}

/*
   @doc EXTERNAL
   @func ULONG | COM_Write | Allows application to transmit bytes to the serial port. Exported to users.
   *
   @rdesc It returns the number of bytes written or -1 if error.
   *
   *
   */
ULONG
COM_Write(HANDLE pHead,         /*@parm [IN]  HANDLE returned by COM_Open.*/
          PUCHAR pSourceBytes,  /*@parm [IN]  Pointer to bytes to be written.*/
          ULONG  NumberOfBytes  /*@parm [IN]  Number of bytes to be written. */
         )
{
    PHW_OPEN_INFO   pOpenHead = (PHW_OPEN_INFO)pHead;
    PHW_INDEP_INFO  pSerialHead = pOpenHead->pSerialHead;
    ULONG               BytesWritten   = 0;
    ULONG               TotalWritten   = 0;
    PHWOBJ              pHWObj         = NULL;
    PVOID               pHWHead        = NULL;
    PHW_VTBL            pFuncTbl       = NULL;
    ULONG               TotalTimeout;   // The Total Timeout
    ULONG               Timeout;        // The Timeout value actually used
    ULONG               WaitReturn;

    DEBUGMSG (ZONE_WRITE|ZONE_FUNCTION,
              (TEXT("+COM_WRITE(0x%X, 0x%X, %d)\r\n"), pHead,
               pSourceBytes, NumberOfBytes));       


    // Check validity of handle
    if ( !pSerialHead || !pSerialHead->OpenCnt ) {
        DEBUGMSG (ZONE_WRITE|ZONE_ERROR,
                  (TEXT("COM_WRITE, device not open\r\n") ));
        SetLastError (ERROR_INVALID_HANDLE);
        return(ULONG)-1;
    }

    // Make sure the caller has access permissions
    if ( !(pOpenHead->AccessCode & GENERIC_WRITE) ) {
        DEBUGMSG(ZONE_USR_READ|ZONE_ERROR,
                 (TEXT("COM_Write: Access permission failure x%X\n\r"),
                  pOpenHead->AccessCode));
        SetLastError (ERROR_INVALID_ACCESS);
        return(ULONG)-1;
    }

#ifdef DEBUG
    if ( IsBadReadPtr(pSourceBytes, NumberOfBytes) ) {
        DEBUGMSG (ZONE_WRITE|ZONE_ERROR,
                  (TEXT("COM_WRITE, bad read pointer\r\n") ));
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ULONG)-1;
    }
#endif

    COM_INC_USAGE_CNT(pOpenHead);

    pHWObj   = pSerialHead->pHWObj;
    pHWHead  = pSerialHead->pHWHead;
    pFuncTbl = pHWObj->pFuncTbl;

    /* Lock out other threads from messing with these pointers.
     */
    DEBUGMSG (ZONE_WRITE, (TEXT("COM_Write wait for CritSec %x.\r\n"),
                           &(pSerialHead->TransmitCritSec1)));
    EnterCriticalSection(&(pSerialHead->TransmitCritSec1));
    DEBUGMSG (ZONE_WRITE, (TEXT("COM_Write Got CritSec %x.\r\n"),
                           &(pSerialHead->TransmitCritSec1)));

    DEBUGMSG (ZONE_WRITE, (TEXT("COM_Write wait for CritSec %x.\r\n"),
                           &(pSerialHead->TxBufferInfo.CS)));
    TxEnterCS(pSerialHead);
    DEBUGMSG (ZONE_WRITE, (TEXT("COM_Write got CritSec %x.\r\n"),
                           &(pSerialHead->TxBufferInfo.CS)));

    pSerialHead->fAbortTransmit = 0;
    // Clear any pending event
    WaitForSingleObject(pSerialHead->hTransmitEvent,0);

    pSerialHead->TxBufferInfo.Permissions = GetCurrentPermissions();
    pSerialHead->TxBufferInfo.TxCharBuffer = pSourceBytes;
    pSerialHead->TxBufferInfo.Length = NumberOfBytes;
    TxRead(pSerialHead) = 0;
    pSerialHead->TxBytesSent = 0;
    pSerialHead->TxBytesPending = NumberOfBytes;

    // Make sure an event isn't hanging around from a previous write time out.
    ResetEvent( pSerialHead->hTransmitEvent );

    TxLeaveCS(pSerialHead);
    DEBUGMSG (ZONE_WRITE|ZONE_FUNCTION,
              (TEXT("COM_Write released CritSec: %x.\r\n"),
               &(pSerialHead->TxBufferInfo.CS)));

    // We call the same write routine that a TX_INTR does.  It queus as 
    // much data as possible, then returns.  From then on, the normal
    // interrupt mechanism kicks in.
    DoTxData( pSerialHead );

    TotalTimeout = pSerialHead->CommTimeouts.WriteTotalTimeoutMultiplier*NumberOfBytes +
                   pSerialHead->CommTimeouts.WriteTotalTimeoutConstant;

    if ( !TotalTimeout )
        Timeout = INFINITE;
    else
        Timeout = TotalTimeout;

    DEBUGMSG (ZONE_WRITE, (TEXT("COM_Write wait for transmission complete event %x, timeout: %d msec.\r\n"),
                           pSerialHead->hTransmitEvent, Timeout));

    WaitReturn = WaitForSingleObject (pSerialHead->hTransmitEvent, Timeout);

    // In the absense of WaitForMultipleObjects, we use flags to
    // handle errors/aborts. Check for aborts or asynchronous closes.
    if ( pSerialHead->fAbortTransmit ) {
        DEBUGMSG(ZONE_USR_READ,(TEXT("COM_Write - Aborting write\r\n")));
        goto LEAVEWRITE;
    }

    if ( !pSerialHead->OpenCnt ) {
        DEBUGMSG(ZONE_WRITE|ZONE_ERROR,
                 (TEXT("COM_Write - device was closed\n\r")));
        SetLastError(ERROR_INVALID_HANDLE);
        goto LEAVEWRITE;
    }

#ifdef DEBUG
    if ( WAIT_TIMEOUT == WaitReturn ) {
        // Timeout
        DEBUGMSG (ZONE_WARN, (TEXT("Write timeout %d, %d\r\n"), NumberOfBytes, pSerialHead->TxBytesPending));
    } else {
        DEBUGMSG (ZONE_WRITE, (TEXT("COM_Write completed normally.\r\n")));
    }
#endif

    LEAVEWRITE:
    // Regardless of timeout, we need to clear the TxBufferInfo
    // to prevent ISR from possibly coming around and trying to use
    // the buffer after we have returned to the caller.
    DEBUGMSG (ZONE_WRITE, (TEXT("COM_Write wait for CritSec %x.\r\n"),
                           &(pSerialHead->TxBufferInfo.CS)));
    TxEnterCS(pSerialHead);
    DEBUGMSG (ZONE_WRITE, (TEXT("COM_Write got CritSec %x.\r\n"),
                           &(pSerialHead->TxBufferInfo.CS)));
    pSerialHead->TxBufferInfo.Permissions = 0;
    pSerialHead->TxBufferInfo.TxCharBuffer = NULL;
    pSerialHead->TxBufferInfo.Length = 0;
    pSerialHead->TxBytesPending = 0;
    TxRead(pSerialHead) = 0;
    TxLeaveCS(pSerialHead);
    DEBUGMSG (ZONE_WRITE|ZONE_FUNCTION,
              (TEXT("COM_Write released CritSec: %x.\r\n"),
               &(pSerialHead->TxBufferInfo.CS)));


    LeaveCriticalSection(&(pSerialHead->TransmitCritSec1));
    DEBUGMSG (ZONE_WRITE|ZONE_FUNCTION,
              (TEXT("COM_Write released CritSec: %x. Exiting\r\n"),
               &(pSerialHead->TransmitCritSec1)));

    /* OK, the Transmitter has gone empty.
     */
    EvaluateEventFlag(pSerialHead, EV_TXEMPTY);

    if ( pSerialHead->DCB.fRtsControl == RTS_CONTROL_TOGGLE ) {
        pFuncTbl->HWClearRTS(pHWHead);
    }

    COM_DEC_USAGE_CNT(pOpenHead);
    DEBUGMSG (ZONE_WRITE|ZONE_FUNCTION,
              (TEXT("-COM_WRITE, returning %d\n\r"),pSerialHead->TxBytesSent));
    return(pSerialHead->TxBytesSent);
}

ULONG
COM_Seek(
        HANDLE  pHead,
        LONG    Position,
        DWORD   Type
        )
{
    return(ULONG)-1;
}

/*
 @doc EXTERNAL
 @func	BOOL | COM_PowerUp | Turn power on to serial device
 * Exported to users.
 @rdesc This routine returns a status of 1 if unsuccessful and 0 otherwise.
 */
BOOL
COM_PowerUp(
           HANDLE      pHead       /*@parm Handle to device. */
           )
{
    PHW_INDEP_INFO  pHWIHead    = (PHW_INDEP_INFO)pHead;
    PHWOBJ          pHWObj;

    if (pHWIHead) {
        pHWObj = (PHWOBJ)pHWIHead->pHWObj;
        if (pHWObj)
            return(pHWObj->pFuncTbl->HWPowerOn(pHWIHead->pHWHead));
    }
    return FALSE;
}

/*
 @doc EXTERNAL
 @func	BOOL | COM_PowerDown | Turns off power to serial device.
 * Exported to users.
 @rdesc This routine returns a status of 1 if unsuccessful and 0 otherwise.
 */
BOOL
COM_PowerDown(
             HANDLE      pHead       /*@parm Handle to device. */
             )
{
    PHW_INDEP_INFO          pHWIHead    = (PHW_INDEP_INFO)pHead;

    if ( pHWIHead ) {
        PHWOBJ  pHWObj = (PHWOBJ)pHWIHead->pHWObj;
        if ( pHWObj)
            return(pHWObj->pFuncTbl->HWPowerOff(pHWIHead->pHWHead));
    }
    return FALSE;
}



/*
 @doc INTERNAL
 @func	ULONG | SerialGetDroppedByteNumber | Returns Number of dropped bytes.
 * Exported to users.
 @rdesc Returns the ULONG representing the number of dropped bytes.
 */
ULONG
SerialGetDroppedByteNumber(
                          HANDLE  pHead          /*@parm Handle to device. */
                          )
{
    PHW_INDEP_INFO          pHWIHead    = (PHW_INDEP_INFO)pHead;

    if ( !pHWIHead ) {
        return(0);
    }

    return(pHWIHead->DroppedBytesMDD+pHWIHead->DroppedBytesPDD);
}

/*
 @doc INTERNAL
 @func	BOOL | WaitCommEvent | See Win32 documentation.
 * Exported to users.
 */
BOOL
WINAPI
WaitCommEvent(
             PHW_OPEN_INFO   pOpenHead,      // @parm Handle to device.
             PULONG          pfdwEventMask,  // @parm Pointer to ULONG to receive CommEvents.fEventMask.
             LPOVERLAPPED    Unused          // @parm Pointer to OVERLAPPED not used.
             )
{
    PHW_INDEP_INFO  pHWIHead = pOpenHead->pSerialHead;
    DWORD           dwEventData;

    DEBUGMSG(ZONE_FUNCTION|ZONE_EVENTS,(TEXT("+WaitCommEvent x%X x%X, pMask x%X\n\r"),
                                        pOpenHead, pHWIHead , pfdwEventMask));

    if ( !pHWIHead || !pHWIHead->OpenCnt ) {
        DEBUGMSG (ZONE_ERROR|ZONE_EVENTS, (TEXT("-WaitCommEvent - device not open (x%X, %d) \r\n"),
                                           pHWIHead, (pHWIHead == NULL) ? 0 : pHWIHead->OpenCnt));
        *pfdwEventMask = 0;
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    // We should return immediately if mask is 0
    if ( !pOpenHead->CommEvents.fEventMask ) {
        DEBUGMSG (ZONE_ERROR|ZONE_EVENTS, (TEXT("-WaitCommEvent - Mask already clear\r\n")));
        *pfdwEventMask = 0;
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    COM_INC_USAGE_CNT(pOpenHead);

    // Abort should only affect us once we start waiting.  Ignore any old aborts
    pOpenHead->CommEvents.fAbort = 0;

    while ( pHWIHead->OpenCnt ) {
        // Read and clear any event bits
        EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));
        ResetEvent(pOpenHead->CommEvents.hCommEvent);
        dwEventData = InterlockedExchange( &(pOpenHead->CommEvents.fEventData), 0 );
        DEBUGMSG (ZONE_EVENTS, (TEXT(" WaitCommEvent - Events 0x%X, Mask 0x%X, Abort %X\r\n"),
                                dwEventData,
                                pOpenHead->CommEvents.fEventMask,
                                pOpenHead->CommEvents.fAbort ));

        // See if we got any events of interest or mask is reset to zero
        if ( (dwEventData & pOpenHead->CommEvents.fEventMask) != 0 ||
                pOpenHead->CommEvents.fEventMask == 0 ) {
            *pfdwEventMask = dwEventData & pOpenHead->CommEvents.fEventMask;
            LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));
            break;
        }
        else
            LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));

        // Wait for an event from PDD, or from SetCommMask
        WaitForSingleObject(pOpenHead->CommEvents.hCommEvent,
                            (ULONG)-1);    

        // We should return immediately if mask was set via SetCommMask.  
        if ( pOpenHead->CommEvents.fAbort ) {
            // We must have been terminated by SetCommMask()
            // Return TRUE with a mask of 0.
            DEBUGMSG (ZONE_ERROR|ZONE_EVENTS, (TEXT(" WaitCommEvent - Mask was cleared\r\n")));
            *pfdwEventMask = 0;
            break;
        }

    }

    COM_DEC_USAGE_CNT(pOpenHead);

    // Check and see if device was closed while we were waiting
    if ( !pHWIHead->OpenCnt ) {
        // Device was closed.  Get out of here.
        DEBUGMSG (ZONE_EVENTS|ZONE_ERROR,
                  (TEXT("-WaitCommEvent - device was closed\r\n")));
        *pfdwEventMask = 0;
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    } else {
        // We either got an event or a SetCommMask 0.
        DEBUGMSG (ZONE_EVENTS,
                  (TEXT("-WaitCommEvent - *pfdwEventMask 0x%X\r\n"),
                   *pfdwEventMask));
        return(TRUE);
    }
}

/* ****************************************************************
 *
 *	Win32 Comm Api Support follows.
 *
 *	@doc EXTERNAL
 */


// ****************************************************************
//
//	@func	VOID | EvaluateEventFlag | Evaluate an event mask.
//
//	@parm PHW_INDEP_INFO	| pHWIHead		| MDD context pointer
//	@parm ULONG				| fdwEventMask	| Bitmask of events
//
//	@rdesc	No return
//
//	@remark	This function is called by the PDD (and internally in the MDD
//			to evaluate a COMM event.  If the user is waiting for a
//	  		COMM event (see WaitCommEvent()) then it will signal the
//			users thread.
//	
VOID
EvaluateEventFlag(PVOID pHead, ULONG fdwEventMask)
{
    PHW_INDEP_INFO  pHWIHead = (PHW_INDEP_INFO)pHead;
    PLIST_ENTRY     pEntry;
    PHW_OPEN_INFO   pOpenHead;
    DWORD           dwTmpEvent, dwOrigEvent;
    BOOL            fRetCode;

    if ( !pHWIHead->OpenCnt ) {
        DEBUGMSG (ZONE_EVENTS|ZONE_ERROR,
                  (TEXT(" EvaluateEventFlag (eventMask = 0x%x) - device was closed\r\n"),fdwEventMask));
        SetLastError (ERROR_INVALID_HANDLE);
        return;
    }

    DEBUGMSG (ZONE_EVENTS, (TEXT(" CommEvent - Event 0x%X, Global Mask 0x%X\r\n"),
                            fdwEventMask,
                            pHWIHead->fEventMask));

    // Now that we support multiple opens, we must check mask for each open handle
    // To keep this relatively painless, we keep a per-device mask which is the
    // bitwise or of each current open mask.  We can check this first before doing
    // all the linked list work to figure out who to notify
    if ( pHWIHead->fEventMask & fdwEventMask ) {
        pEntry = pHWIHead->OpenList.Flink;
        while ( pEntry != &pHWIHead->OpenList ) {
            pOpenHead = CONTAINING_RECORD( pEntry, HW_OPEN_INFO, llist);
            pEntry = pEntry->Flink;  // advance to next 

            EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));
            // Don't do anything unless this event is of interest to the MDD.
            if ( pOpenHead->CommEvents.fEventMask & fdwEventMask ) {
                // Store the event data
                dwOrigEvent = pOpenHead->CommEvents.fEventData;                    
                do {
                    dwTmpEvent = dwOrigEvent;
                    dwOrigEvent = InterlockedExchange(&(pOpenHead->CommEvents.fEventData), 
                                                      dwTmpEvent | fdwEventMask) ;

                } while ( dwTmpEvent != dwOrigEvent );

                // Signal the MDD that new event data is available.
                fRetCode = SetEvent(pOpenHead->CommEvents.hCommEvent);
                DEBUGMSG (ZONE_EVENTS, (TEXT(" CommEvent - Event 0x%X, Handle 0x%X Mask 0x%X (%X)\r\n"),
                                        dwTmpEvent | fdwEventMask,
                                        pOpenHead,
                                        pOpenHead->CommEvents.fEventMask,
                                        fRetCode));

            }
            LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));
        }
    }

    return;
}

// ****************************************************************
//
//	@func BOOL | COM_IOControl | Device IO control routine
//	@parm DWORD | dwOpenData | value returned from COM_Open call
//	@parm DWORD | dwCode | io control code to be performed
//	@parm PBYTE | pBufIn | input data to the device
//	@parm DWORD | dwLenIn | number of bytes being passed in
//	@parm PBYTE | pBufOut | output data from the device
//	@parm DWORD | dwLenOut |maximum number of bytes to receive from device
//	@parm PDWORD | pdwActualOut | actual number of bytes received from device
//
//	@rdesc		Returns TRUE for success, FALSE for failure
//
//	@remark		Routine exported by a device driver.  "COM" is the string 
//				passed in as lpszType in RegisterDevice

BOOL
COM_IOControl(PHW_OPEN_INFO pOpenHead,
              DWORD dwCode, PBYTE pBufIn,
              DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
              PDWORD pdwActualOut)
{
    BOOL            RetVal           = TRUE;        // Initialize to success
    PHW_INDEP_INFO  pHWIHead; //= pOpenHead->pSerialHead;
    PLIST_ENTRY     pEntry;
    PHWOBJ          pHWObj   = NULL;
    PVOID           pHWHead  = NULL;
    PHW_VTBL        pFuncTbl = NULL;
    UCHAR           stopbits = 0;
    DWORD           dwFlags;

    if (pOpenHead==NULL) {
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    }
    pHWIHead = pOpenHead->pSerialHead;
    if ( pHWIHead ) {
        pHWObj   = (PHWOBJ)pHWIHead->pHWObj;
        pFuncTbl = pHWObj->pFuncTbl;
        pHWHead  = pHWIHead->pHWHead;
    } else {
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    if ((pOpenHead->AccessCode & DEVACCESS_BUSNAMESPACE)!=0) { // Special IOCTL
        switch (dwCode) {
        case IOCTL_POWER_CAPABILITIES:case IOCTL_POWER_SET:case IOCTL_POWER_GET:
        case IOCTL_POWER_QUERY:case IOCTL_REGISTER_POWER_RELATIONSHIP:
            // Power is Handle by PDD.
            // Pass IOCTL through to PDD if hook is provided
            if ( (pFuncTbl->HWIoctl == NULL) ||
                 (pFuncTbl->HWIoctl(pHWHead,dwCode,pBufIn,dwLenIn,pBufOut,dwLenOut, pdwActualOut) == FALSE)) {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = FALSE;
                DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid ioctl 0x%X\r\n"), dwCode));
            }
            break;
        default:
            SetLastError (ERROR_INVALID_HANDLE);
            RetVal = FALSE;
            break;
        }
        return RetVal;
    }

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION,
              (TEXT("+COM_IOControl(0x%X, %d, 0x%X, %d, 0x%X, %d, 0x%X)\r\n"),
               pOpenHead, dwCode, pBufIn, dwLenIn, pBufOut,
               dwLenOut, pdwActualOut));

    if ( !pHWIHead->OpenCnt ) {
        DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,
                  (TEXT(" COM_IOControl - device was closed\r\n")));
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    if ( dwCode == IOCTL_PSL_NOTIFY ) {
        PDEVICE_PSL_NOTIFY pPslPacket = (PDEVICE_PSL_NOTIFY)pBufIn;
        if ( (pPslPacket->dwSize == sizeof(DEVICE_PSL_NOTIFY)) && (pPslPacket->dwFlags == DLL_PROCESS_EXITING) ) {
            DEBUGMSG(ZONE_IOCTL, (TEXT("Process is exiting.\r\n")));
            ProcessExiting(pOpenHead);
        }
        return (TRUE);
    }
    // Make sure the caller has access permissions
    // NOTE : Pay attention here.  I hate to make this check repeatedly
    // below, so I'll optimize it here.  But as you add new ioctl's be
    // sure to account for them in this if check.
    if ( !( (dwCode == IOCTL_SERIAL_GET_WAIT_MASK) ||
            (dwCode == IOCTL_SERIAL_SET_WAIT_MASK) ||
            (dwCode == IOCTL_SERIAL_WAIT_ON_MASK) ||
            (dwCode == IOCTL_SERIAL_GET_MODEMSTATUS) ||
            (dwCode == IOCTL_SERIAL_GET_PROPERTIES) ||
            (dwCode == IOCTL_SERIAL_GET_TIMEOUTS) ||
            (dwCode == IOCTL_POWER_CAPABILITIES) ||
            (dwCode == IOCTL_POWER_QUERY) ||
            (dwCode == IOCTL_POWER_SET)) ) {
        // If not one of the above operations, then read or write
        // access permissions are required.
        if ( !(pOpenHead->AccessCode & (GENERIC_READ | GENERIC_WRITE | VGD_AC_CONFIGURATOR) ) ) {
            DEBUGMSG(ZONE_IOCTL|ZONE_ERROR,
                     (TEXT("COM_Ioctl: Ioctl %x access permission failure x%X\n\r"),
                      dwCode, pOpenHead->AccessCode));
            SetLastError (ERROR_INVALID_ACCESS);
            return(FALSE);
        }

    }

    COM_INC_USAGE_CNT(pOpenHead);

    switch ( dwCode ) {
    // ****************************************************************
    //	
    //	@func BOOL	| IOCTL_SERIAL_SET_BREAK_ON |
    //				Device IO control routine to set the break state.
    //
    //	@parm DWORD | dwOpenData | value returned from COM_Open call
    //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_BREAK_ON
    //	@parm PBYTE | pBufIn | Ignored
    //	@parm DWORD | dwLenIn | Ignored
    //	@parm PBYTE | pBufOut | Ignored
    //	@parm DWORD | dwLenOut | Ignored
    //	@parm PDWORD | pdwActualOut | Ignored
    //
    //	@rdesc		Returns TRUE for success, FALSE for failure (and
    //				sets thread error code)
    //
    //	@remark Sets the transmission line in a break state until
    //				<f IOCTL_SERIAL_SET_BREAK_OFF> is called.
    //
    case IOCTL_SERIAL_SET_BREAK_ON :
        DEBUGMSG (ZONE_IOCTL,
                  (TEXT(" IOCTL_SERIAL_SET_BREAK_ON\r\n")));
        pFuncTbl->HWSetBreak(pHWHead);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_BREAK_OFF |
        //				Device IO control routine to clear the break state.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_BREAK_OFF
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@remark		Restores character transmission for the communications
        //				device and places the transmission line in a nonbreak state
        //				(called after <f IOCTL_SERIAL_SET_BREAK_ON>).
        //
    case IOCTL_SERIAL_SET_BREAK_OFF :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_BREAK_OFF\r\n")));
        pFuncTbl->HWClearBreak(pHWHead);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_DTR |
        //				Device IO control routine to set DTR high.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_DTR
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_CLR_DTR>
        //
    case IOCTL_SERIAL_SET_DTR :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_DTR\r\n")));
        /* It's an error to call this if DCB uses DTR_CONTROL_HANDSHAKE.
         */
        if ( pHWIHead->DCB.fDtrControl != DTR_CONTROL_HANDSHAKE ) {
            pFuncTbl->HWSetDTR(pHWHead);
        } else {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
        }
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_CLR_DTR |
        //				Device IO control routine to set DTR low.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_CLR_DTR
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_DTR>
        //
    case IOCTL_SERIAL_CLR_DTR :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_CLR_DTR\r\n")));
        /* It's an error to call this if DCB uses DTR_CONTROL_HANDSHAKE.
         */
        if ( pHWIHead->DCB.fDtrControl != DTR_CONTROL_HANDSHAKE ) {
            pFuncTbl->HWClearDTR(pHWHead);
        } else {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
        }
        break;

        // ****************************************************************
        //	
        //	@func	BOOL | IOCTL_SERIAL_SET_RTS |
        //				Device IO control routine to set RTS high.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_RTS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_CLR_RTS>
        //
    case IOCTL_SERIAL_SET_RTS :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_RTS\r\n")));
        /* It's an error to call this if DCB uses RTS_CONTROL_HANDSHAKE.
         */
        if ( pHWIHead->DCB.fRtsControl != RTS_CONTROL_HANDSHAKE ) {
            pFuncTbl->HWSetRTS(pHWHead);
        } else {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
        }
        break;

        // ****************************************************************
        //	
        //	@func	BOOL | IOCTL_SERIAL_CLR_RTS |
        //				Device IO control routine to set RTS low.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_CLR_RTS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_RTS>
        //
    case IOCTL_SERIAL_CLR_RTS :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_CLR_RTS\r\n")));
        /* It's an error to call this if DCB uses RTS_CONTROL_HANDSHAKE.
         */
        if ( pHWIHead->DCB.fRtsControl != RTS_CONTROL_HANDSHAKE ) {
            pFuncTbl->HWClearRTS(pHWHead);
        } else {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
        }
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_XOFF |
        //				Device IO control routine to cause transmission
        //				to act as if an XOFF character has been received.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_XOFF
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_XON>
        //
    case IOCTL_SERIAL_SET_XOFF :
        DEBUGMSG (ZONE_IOCTL|ZONE_FLOW, (TEXT(" IOCTL_SERIAL_SET_XOFF\r\n")));
        if ( pHWIHead->XFlow ) {
            pHWIHead->StopXmit = 1;
            pHWIHead->SentXoff = 1;
        }
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_XON |
        //				Device IO control routine to cause transmission
        //				to act as if an XON character has been received.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_XON
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_XOFF>
        //
    case IOCTL_SERIAL_SET_XON :
        DEBUGMSG (ZONE_IOCTL|ZONE_FLOW, (TEXT(" IOCTL_SERIAL_SET_XON\r\n")));
        if ( pHWIHead->XFlow ) {
            pHWIHead->StopXmit = 0;
            pHWIHead->SentXoff = 0;
        }
        break;

        // ****************************************************************
        //	
        //	@func		BOOL | IOCTL_SERIAL_GET_WAIT_MASK |
        //				Device IO control routine to retrieve the value
        //				of the event mask.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_WAIT_MASK
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to DWORD to place event mask
        //	@parm DWORD | dwLenOut | should be sizeof(DWORD) or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(DWORD) if no
        //				error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_WAIT_MASK>
        //				<f IOCTL_SERIAL_WAIT_ON_MASK>
        //
    case IOCTL_SERIAL_GET_WAIT_MASK :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_WAIT_MASK\r\n")));
        if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        // Set The Wait Mask
        *(DWORD *)pBufOut = pOpenHead->CommEvents.fEventMask;

        // Return the size
        *pdwActualOut = sizeof(DWORD);

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_WAIT_MASK |
        //				Device IO control routine to set the value
        //				of the event mask.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_WAIT_MASK
        //	@parm PBYTE | pBufIn | Pointer to the DWORD mask value
        //	@parm DWORD | dwLenIn | should be sizeof(DWORD)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_GET_WAIT_MASK>
        //				<f IOCTL_SERIAL_WAIT_ON_MASK>
        //
    case IOCTL_SERIAL_SET_WAIT_MASK :
        if ( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_WAIT_MASK 0x%X\r\n"),
                               *(DWORD *)pBufIn));

        EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));
        // OK, now we can actually set the mask
        pOpenHead->CommEvents.fEventMask = *(DWORD *)pBufIn;

        // NOTE: If there is an outstanding WaitCommEvent, it should
        // return an error when SET_WAIT_MASK is called.  We accomplish
        // this by generating an hCommEvent which will wake the WaitComm
        // and subsequently return error (since no event bits will be set )
        pOpenHead->CommEvents.fAbort = 1;
        SetEvent(pOpenHead->CommEvents.hCommEvent);

        // And calculate the OR of all masks for this port.  Use a temp
        // variable so that the other threads don't see a partial mask
        dwFlags = 0;
        pEntry = pHWIHead->OpenList.Flink;
        while ( pEntry != &pHWIHead->OpenList ) {
            PHW_OPEN_INFO   pTmpOpenHead;

            pTmpOpenHead = CONTAINING_RECORD( pEntry, HW_OPEN_INFO, llist);
            pEntry = pEntry->Flink;    // advance to next 

            DEBUGMSG (ZONE_EVENTS, (TEXT(" SetWaitMask - handle x%X mask x%X\r\n"),
                                    pTmpOpenHead, pTmpOpenHead->CommEvents.fEventMask));
            dwFlags |= pTmpOpenHead->CommEvents.fEventMask;
        }
        pHWIHead->fEventMask = dwFlags;
        LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));
        DEBUGMSG (ZONE_EVENTS, (TEXT(" SetWaitMask - mask x%X, global mask x%X\r\n"),
                                pOpenHead->CommEvents.fEventMask, pHWIHead->fEventMask));


        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_WAIT_ON_MASK |
        //				Device IO control routine to wait for a communications
        //				event that matches one in the event mask
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_WAIT_ON_MASK
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to DWORD to place event mask.
        //				The returned mask will show the event that terminated
        //				the wait.  If a process attempts to change the device
        //				handle's event mask by using the IOCTL_SERIAL_SET_WAIT_MASK
        //				call the driver should return immediately with (DWORD)0 as
        //				the returned event mask.
        //	@parm DWORD | dwLenOut | should be sizeof(DWORD) or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(DWORD) if no
        //				error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_GET_WAIT_MASK>
        //				<f IOCTL_SERIAL_SET_WAIT_MASK>
        //
    case IOCTL_SERIAL_WAIT_ON_MASK :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_WAIT_ON_MASK\r\n")));
        if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        if ( WaitCommEvent(pOpenHead, (DWORD *)pBufOut, NULL) == FALSE ) {
            // Device may have been closed or removed while we were waiting 
            DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,
                      (TEXT(" COM_IOControl - Error in WaitCommEvent\r\n")));
            RetVal = FALSE;
        }
        // Return the size
        *pdwActualOut = sizeof(DWORD);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_COMMSTATUS |
        //				Device IO control routine to clear any pending
        //				communications errors and return the current communication
        //				status.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_COMMSTATUS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to a <f SERIAL_DEV_STATUS>
        //				structure for the returned status information
        //	@parm DWORD | dwLenOut | should be sizeof(SERIAL_DEV_STATUS)
        //				or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to
        //				sizeof(SERIAL_DEV_STATUS) if no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_GET_COMMSTATUS :
        {
            PSERIAL_DEV_STATUS pSerialDevStat;

            DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_COMMSTATUS\r\n")));
            if ( (dwLenOut < sizeof(SERIAL_DEV_STATUS)) || (NULL == pBufOut) ||
                 (NULL == pdwActualOut) ) {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = FALSE;
                DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
                break;
            }

            pSerialDevStat = (PSERIAL_DEV_STATUS)pBufOut;

            // Set The Error Mask
            pSerialDevStat->Errors = 0;

            // Clear the ComStat structure & get PDD related status
            memset ((char *) &(pSerialDevStat->ComStat), 0, sizeof(COMSTAT));
            ((PSERIAL_DEV_STATUS)pBufOut)->Errors =
            pFuncTbl->HWGetStatus(pHWHead, &(pSerialDevStat->ComStat));

            // PDD set fCtsHold, fDsrHold, fRLSDHold, and fTXim.  The MDD then
            // needs to set fXoffHold, fXoffSent, cbInQue, and cbOutQue.
            pSerialDevStat->ComStat.cbInQue = RxBytesAvail(pHWIHead);
            pSerialDevStat->ComStat.cbOutQue =  pHWIHead->TxBytesPending;
            pSerialDevStat->ComStat.fXoffHold = pHWIHead->StopXmit;
            pSerialDevStat->ComStat.fXoffSent = pHWIHead->SentXoff;

            // Return the size
            *pdwActualOut = sizeof(SERIAL_DEV_STATUS);

        }

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_MODEMSTATUS |
        //				Device IO control routine to retrieve current
        //				modem control-register values
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_MODEMSTATUS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to a DWORD for the returned
        //				modem status information
        //	@parm DWORD | dwLenOut | should be sizeof(DWORD)
        //				or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(DWORD)
        //				if no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_GET_MODEMSTATUS :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_MODEMSTATUS\r\n")));
        if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        // Set the Modem Status dword
        *(DWORD *)pBufOut = 0;

        pFuncTbl->HWGetModemStatus(pHWHead, (PULONG)pBufOut);

        // Return the size
        *pdwActualOut = sizeof(DWORD);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_PROPERTIES |
        //				Device IO control routine to retrieve information
        //				about the communications properties for the device.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_PROPERTIES
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to a <f COMMPROP> structure
        //				for the returned information.
        //	@parm DWORD | dwLenOut | should be sizeof(COMMPROP)
        //				or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(COMMPROP)
        //				if no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_GET_PROPERTIES :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_PROPERTIES\r\n")));
        if ( (dwLenOut < sizeof(COMMPROP)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        // Clear the ComMProp structure
        memset ((char *) ((COMMPROP *)pBufOut), 0,
                sizeof(COMMPROP));

        pFuncTbl->HWGetCommProperties(pHWHead, (LPCOMMPROP)pBufOut);

        // Return the size
        *pdwActualOut = sizeof(COMMPROP);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_TIMEOUTS |
        //				Device IO control routine to set the time-out parameters
        //				for all read and write operations on a specified
        //				communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_TIMEOUTS
        //	@parm PBYTE | pBufIn | Pointer to the <f COMMTIMEOUTS> structure
        //	@parm DWORD | dwLenIn | should be sizeof(COMMTIMEOUTS)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_GET_TIMEOUTS>
        //
    case IOCTL_SERIAL_SET_TIMEOUTS :
        if ( (dwLenIn < sizeof(COMMTIMEOUTS)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL,
                  (TEXT(" IOCTL_SERIAL_SET_COMMTIMEOUTS (%d,%d,%d,%d,%d)\r\n"),
                   ((COMMTIMEOUTS *)pBufIn)->ReadIntervalTimeout,
                   ((COMMTIMEOUTS *)pBufIn)->ReadTotalTimeoutMultiplier,
                   ((COMMTIMEOUTS *)pBufIn)->ReadTotalTimeoutConstant,
                   ((COMMTIMEOUTS *)pBufIn)->WriteTotalTimeoutMultiplier,
                   ((COMMTIMEOUTS *)pBufIn)->WriteTotalTimeoutConstant));

        pHWIHead->CommTimeouts.ReadIntervalTimeout =
        ((COMMTIMEOUTS *)pBufIn)->ReadIntervalTimeout;
        pHWIHead->CommTimeouts.ReadTotalTimeoutMultiplier =
        ((COMMTIMEOUTS *)pBufIn)->ReadTotalTimeoutMultiplier;
        pHWIHead->CommTimeouts.ReadTotalTimeoutConstant =
        ((COMMTIMEOUTS *)pBufIn)->ReadTotalTimeoutConstant;
        pHWIHead->CommTimeouts.WriteTotalTimeoutMultiplier =
        ((COMMTIMEOUTS *)pBufIn)->WriteTotalTimeoutMultiplier;
        pHWIHead->CommTimeouts.WriteTotalTimeoutConstant =
        ((COMMTIMEOUTS *)pBufIn)->WriteTotalTimeoutConstant;

        pFuncTbl->HWSetCommTimeouts(pHWHead, (COMMTIMEOUTS *)pBufIn);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_TIMEOUTS |
        //				Device IO control routine to get the time-out parameters
        //				for all read and write operations on a specified
        //				communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_TIMEOUTS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Pointer to a <f COMMTIMEOUTS> structure
        //				for the returned data
        //	@parm DWORD | dwLenOut | should be sizeof(COMMTIMEOUTS)
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(COMMTIMEOUTS)
        //				if no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_GET_TIMEOUTS>
        //
    case IOCTL_SERIAL_GET_TIMEOUTS :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_TIMEOUTS\r\n")));
        if ( (dwLenOut < sizeof(COMMTIMEOUTS)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        // Clear the structure
        memset ((char *) ((COMMTIMEOUTS *)pBufOut), 0,
                sizeof(COMMTIMEOUTS));

        memcpy((LPCOMMTIMEOUTS)pBufOut, &(pHWIHead->CommTimeouts),
               sizeof(COMMTIMEOUTS));

        // Return the size
        *pdwActualOut = sizeof(COMMTIMEOUTS);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_PURGE |
        //				Device IO control routine to discard characters from the
        //				output or input buffer of a specified communications
        //				resource.  It can also terminate pending read or write
        //				operations on the resource
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_PURGE
        //	@parm PBYTE | pBufIn | Pointer to a DWORD containing the action
        //	@parm DWORD | dwLenIn | Should be sizeof(DWORD)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_PURGE :
        if ( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        dwFlags = *((PDWORD) pBufIn);

        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_PURGE 0x%X\r\n"),dwFlags));

        pFuncTbl->HWPurgeComm(pHWHead,dwFlags);
        if ( dwFlags & PURGE_RXCLEAR ) {
            RxEnterCS(pHWIHead);
            DEBUGMSG(ZONE_IOCTL|ZONE_USR_READ,
                     (TEXT(" Flushing %d bytes from the read buffer\r\n"),RxBytesAvail(pHWIHead)));
            pHWIHead->RxBufferInfo.Read = pHWIHead->RxBufferInfo.Write;
            memset(pHWIHead->RxBufferInfo.RxCharBuffer, 0, pHWIHead->RxBufferInfo.Length);
            RxLeaveCS(pHWIHead);
            // Clear any flow control state
            if ( pHWIHead->DCB.fInX && pHWIHead->SentXoff ) {
                DEBUGMSG (ZONE_IOCTL|ZONE_FLOW, (TEXT("Sending XON\r\n")));
                pHWIHead->SentXoff = 0;
                if ( !pHWIHead->DCB.fTXContinueOnXoff ) {
                    pHWIHead->StopXmit = 0;
                }
                pFuncTbl->HWXmitComChar(pHWIHead->pHWHead,pHWIHead->DCB.XonChar);
            }
            if ( pHWIHead->RtsFlow &&
                 (pHWIHead->DCB.fRtsControl == RTS_CONTROL_HANDSHAKE) ) {
                DEBUGMSG (ZONE_IOCTL|ZONE_USR_READ|ZONE_FLOW,
                          (TEXT("RTS_CONTROL_HANDSHAKE Setting RTS\r\n")));
                pHWIHead->RtsFlow = 0;
                pFuncTbl->HWSetRTS(pHWIHead->pHWHead);
            }
            if ( pHWIHead->DtrFlow && 
                 (pHWIHead->DCB.fDtrControl == DTR_CONTROL_HANDSHAKE) ) {
                DEBUGMSG (ZONE_IOCTL|ZONE_USR_READ|ZONE_FLOW,
                          (TEXT("DTR_CONTROL_HANDSHAKE Setting DTR\r\n")));
                pHWIHead->DtrFlow = 0;
                pFuncTbl->HWSetDTR(pHWHead);
            }
        }

        // Now, free up any threads blocked in MDD. Reads and writes are in
        // loops, so they also need a flag to tell them to abort.
        if ( dwFlags & PURGE_RXABORT ) {
            pHWIHead->fAbortRead = 1;
            PulseEvent(pHWIHead->hReadEvent);
        }
        if ( dwFlags & PURGE_TXABORT ) {
            pHWIHead->fAbortTransmit      = 1;
            // COM_Write() clears event upon entry, so we can use SetEvent
            SetEvent(pHWIHead->hTransmitEvent);
        }

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_QUEUE_SIZE |
        //				Device IO control routine to set the queue sizes of of a
        //				communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_QUEUE_SIZE
        //	@parm PBYTE | pBufIn | Pointer to a <f SERIAL_QUEUE_SIZES>
        //				structure
        //	@parm DWORD | dwLenIn | should be sizeof(<f SERIAL_QUEUE_SIZES>)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_SET_QUEUE_SIZE :
        if ( (dwLenIn < sizeof(SERIAL_QUEUE_SIZES)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL,
                  (TEXT(" IOCTL_SERIAL_SET_QUEUE_SIZE (%d,%d,%d,%d,%d)\r\n"),
                   ((SERIAL_QUEUE_SIZES *)pBufIn)->cbInQueue,
                   ((SERIAL_QUEUE_SIZES *)pBufIn)->cbOutQueue));

        // NOTE: Normally we would do something with the passed in parameter.
        // But we don't think the user has a better idea of queue sizes
        // compared to our infinite knowledge.

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_IMMEDIATE_CHAR |
        //				Device IO control routine to transmit a specified character
        //				ahead of any pending data in the output buffer of the
        //				communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_IMMEDIATE_CHAR
        //	@parm PBYTE | pBufIn | Pointer to a UCHAR to send
        //	@parm DWORD | dwLenIn | should be sizeof(UCHAR)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_IMMEDIATE_CHAR :
        if ( (dwLenIn < sizeof(UCHAR)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_IMMEDIATE_CHAR 0x%X\r\n"),
                               (UCHAR *)pBufIn));

        pFuncTbl->HWXmitComChar(pHWHead, *pBufIn);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_DCB |
        //				Device IO control routine to get the device-control
        //				block from a specified communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_DCB
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Pointer to a <f DCB> structure
        //	@parm DWORD | dwLenOut | Should be sizeof(<f DCB>)
        //	@parm PDWORD | pdwActualOut | Pointer to DWORD to return length
        //				of returned data (should be set to sizeof(<f DCB>) if
        //				no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_GET_DCB :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_DCB\r\n")));
        if ( (dwLenOut < sizeof(DCB)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        memcpy((char *)pBufOut, (char *)&(pHWIHead->DCB), sizeof(DCB));

        // Return the size
        *pdwActualOut = sizeof(DCB);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_DCB |
        //				Device IO control routine to set the device-control
        //				block on a specified communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_DCB
        //	@parm PBYTE | pBufIn | Pointer to a <f DCB> structure
        //	@parm DWORD | dwLenIn | should be sizeof(<f DCB>)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_SET_DCB :
        if ( (dwLenIn < sizeof(DCB)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_DCB\r\n")));

        if ( !ApplyDCB (pHWIHead, (DCB *)pBufIn, TRUE) ) {
            //
            // Most likely an unsupported baud rate was specified
            //
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" ApplyDCB failed\r\n")));
            break;
        }

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_ENABLE_IR |
        //				Device IO control routine to set the device-control
        //				block on a specified communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_ENABLE_IR
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_ENABLE_IR :
        DEBUGMSG (ZONE_IR, 
                  (TEXT("IOCTL Enable IR\r\n")));
        if ( !pFuncTbl->HWEnableIR(pHWHead, pHWIHead->DCB.BaudRate) ) {
            RetVal = FALSE;

            DEBUGMSG (ZONE_IOCTL|ZONE_ERROR|ZONE_IR, 
                      (TEXT("IR mode failed\r\n")));
        }

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_DISABLE_IR |
        //				Device IO control routine to set the device-control
        //				block on a specified communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_DISABLE_IR
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_DISABLE_IR :
        DEBUGMSG (ZONE_IR, 
                  (TEXT("IOCTL Disable IR\r\n")));
        pFuncTbl->HWDisableIR(pHWHead);
        break;
    default :
        // Pass IOCTL through to PDD if hook is provided
        if ( (pFuncTbl->HWIoctl == NULL) ||
             (pFuncTbl->HWIoctl(pHWHead,dwCode,pBufIn,dwLenIn,pBufOut,dwLenOut,
                                pdwActualOut) == FALSE) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid ioctl 0x%X\r\n"), dwCode));
        }
        break;
    }

    COM_DEC_USAGE_CNT(pOpenHead);    

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION|(RetVal == FALSE?ZONE_ERROR:0),
              (TEXT("-COM_IOControl %s Ecode=%d (len=%d)\r\n"),
               (RetVal == TRUE) ? TEXT("Success") : TEXT("Error"),
               GetLastError(), (NULL == pdwActualOut) ? 0 : *pdwActualOut));

    return(RetVal);
}
