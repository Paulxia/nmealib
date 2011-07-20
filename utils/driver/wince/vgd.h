///////////////////////////////////////////////////////////
//
// Virtual PPC Driver
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: vgd.h 19 2007-04-03 15:03:25Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __VGD_H__
#define __VGD_H__

#include <ddkreg.h>

#include "config.h"

#define VGD_FLG_ABOUTSTR    0x01

#define VGD_IOCTL_START     0xFF
#define VGD_IOCTL_PUSHDATA  CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 1,METHOD_BUFFERED,FILE_ANY_ACCESS)

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////
// QueueBuffer
///////////////////////////////////////////////////////////

typedef struct __QueueBuffer
{
    PUCHAR Buffer;
    ULONG Size;
    ULONG ReadPos;
    ULONG WritePos;
    CRITICAL_SECTION CS;
} QueueBuffer;

void QueueBuffer_Read(QueueBuffer * pQB, PUCHAR pDestBuffer, ULONG * pBytesToRead);
void QueueBuffer_Write(QueueBuffer * pQB, PUCHAR pSourceBuffer, ULONG * pBytesToWrite);
void QueueBuffer_Reset(QueueBuffer * pQB);

///////////////////////////////////////////////////////////
// VGD_INFO
///////////////////////////////////////////////////////////

typedef struct _VGD_INFO
{
    DCB					dcb;
    int                 flags;
    INTERRUPT_TYPE      curr_interruptIDs;
    QueueBuffer         buffer;

} VGD_INFO, *PVGD_INFO;

///////////////////////////////////////////////////////////
// And now, all the function prototypes
///////////////////////////////////////////////////////////

BOOL SL_Init(
    ULONG   Identifier,
    PVOID   pHead, //  points to device head
    PVOID   pMddHead   // This is the first parm to callback
    );
BOOL SL_PostInit(
    PVOID   pHead 
    );
VOID SL_Deinit(
    PVOID   pHead //  points to device head
    );
BOOL SL_Open(
    PVOID   pHead 
    );
VOID SL_Close(
    PVOID   pHead
    );
VOID SL_ClearDTR(
    PVOID   pHead 
    );
VOID SL_SetDTR(
    PVOID   pHead 
    );
VOID SL_ClearRTS(
    PVOID   pHead 
    );
VOID SL_SetRTS(
    PVOID   pHead 
    );
VOID SL_ClearBreak(
    PVOID   pHead 
    );
VOID SL_SetBreak(
    PVOID   pHead 
    );
VOID SL_ClearBreak(
    PVOID   pHead 
    );
VOID SL_SetBreak(
    PVOID   pHead
    );
ULONG SL_GetByteNumber(
    PVOID   pHead	     
    );
VOID SL_DisableXmit(
    PVOID   pHead	
    );
VOID SL_EnableXmit(
    PVOID   pHead	
    );
BOOL SL_SetBaudRate(
    PVOID   pHead,
    ULONG   BaudRate	//      ULONG representing decimal baud rate.
    );
BOOL SL_SetDCB(
    PVOID   pHead,	
    LPDCB   lpDCB       //     Pointer to DCB structure
    );
ULONG SL_SetCommTimeouts(
    PVOID   pHead,	
    LPCOMMTIMEOUTS   lpCommTimeouts //  Pointer to CommTimeout structure
    );
ULONG SL_GetRxBufferSize(
    PVOID pHead
    );
INTERRUPT_TYPE SL_GetInterruptType(
    PVOID pHead
    );
ULONG SL_RxIntr(
    PVOID pHead,
    PUCHAR pRxBuffer,       // Pointer to receive buffer
    ULONG *pBufflen         //  In = max bytes to read, out = bytes read
    );
ULONG SL_PutBytes(
    PVOID   pHead,
    PUCHAR  pSrc,	    // 	Pointer to bytes to be sent.
    ULONG   NumberOfBytes,  // 	Number of bytes to be sent.
    PULONG  pBytesSent	    // 	Pointer to actual number of bytes put.
    );
VOID SL_TxIntr(
    PVOID pHead 
    );
VOID SL_LineIntr(
    PVOID pHead
    );
VOID SL_OtherIntr(
    PVOID pHead 
    );
VOID SL_ModemIntr(
    PVOID pHead 
    );
ULONG SL_GetStatus(
    PVOID	pHead,
    LPCOMSTAT	lpStat	// Pointer to LPCOMMSTAT to hold status.
    );
VOID SL_Reset(
    PVOID   pHead
    );
VOID SL_GetModemStatus(
    PVOID   pHead,
    PULONG  pModemStatus    //  PULONG passed in by user.
    );
VOID SL_PurgeComm(
    PVOID   pHead,
    DWORD   fdwAction	    //  Action to take. 
    );
BOOL SL_XmitComChar(
    PVOID   pHead,
    UCHAR   ComChar   //  Character to transmit. 
    );
VOID SL_PowerOn(
    PVOID   pHead
    );
VOID SL_PowerOff(
    PVOID   pHead
    );
BOOL SL_Ioctl(
    PVOID pHead,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut);

VOID SL_TxIntrEx(
    PVOID pHead,
    PUCHAR pTxBuffer,          // @parm Pointer to receive buffer
    ULONG *pBufflen            // @parm In = max bytes to transmit, out = bytes transmitted
    );
    
#ifdef __cplusplus
}
#endif


#endif // __VGD_H__
