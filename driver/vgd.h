////////////////////////////////////////////////////////////////////////////
//
// Module Name  : vgd.h
// Author       : Tim (spaceflush@mail.ru)
// Date         : November 2006
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __VGD_H__
#define __VGD_H__

#include <ddkreg.h>

#include "config.h"

#include <nmea/generator.h>

#define VGD_FLG_ABOUTSTR    0x01

#define VGD_IOCTL_START     0xFF
#define VGD_IOCTL_SETLAT    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 1,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_SETLON    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 2,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_GENIDX    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 3,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_ADDGEN    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 4,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_SETSIG    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 5,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_SETFIX    CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 6,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_GETINFO   CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 7,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define VGD_IOCTL_SETINFO   CTL_CODE(FILE_DEVICE_SERIAL_PORT, VGD_IOCTL_START + 8,METHOD_BUFFERED,FILE_ANY_ACCESS)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VGD_INFO
{
    DCB					dcb;
    CRITICAL_SECTION    allow_mutex;
    int                 nmea_pack_mask;
    int                 flags;
    nmeaGENERATOR       *nmea_gen;
    nmeaINFO            *nmea_info;
    DWORD               timer_timeout;
    HANDLE              timer_thread;
    HANDLE              stop_thread_event;
    HANDLE              kill_thread_event;
    INTERRUPT_TYPE      curr_interruptIDs;

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
