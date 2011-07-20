///////////////////////////////////////////////////////////
//
// Virtual PPC Driver
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: vgd.c 19 2007-04-03 15:03:25Z xtimor $
//
///////////////////////////////////////////////////////////

#include <windows.h>
#include <serhw.h>
#include <hw16550.h>

#include "config.h"
#include "vgd.h"
#include "pdd.h"
#include "mdd.h"
#include "log.h"

/*
BOOL VGD_SetByteSize(PVOID pHead, ULONG ByteSize);
BOOL VGD_SetStopBits(PVOID pHead, ULONG StopBits);
BOOL VGD_SetParity(PVOID pHead, ULONG Parity);
*/

///////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////

void QueueBuffer_Read(QueueBuffer * pQB, PUCHAR pDestBuffer, ULONG * pBytesToRead)
{
    ULONG BytesAvailable = pQB->WritePos - pQB->ReadPos;
    ULONG BytesToCopy = *pBytesToRead > BytesAvailable ? BytesAvailable : *pBytesToRead;

    if (BytesToCopy > 0)
    {
        EnterCriticalSection(&pQB->CS);

        try
        {
            memcpy(pDestBuffer, pQB->Buffer + pQB->ReadPos, BytesToCopy);
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {}

        pQB->ReadPos += BytesToCopy;

        LeaveCriticalSection(&pQB->CS);
    }

    *pBytesToRead = BytesToCopy;
}

void QueueBuffer_Write(QueueBuffer * pQB, PUCHAR pSourceBuffer, ULONG * pBytesToWrite)
{
    ULONG BytesLeft = pQB->Size - pQB->WritePos;
    ULONG BytesUnnecessary = pQB->ReadPos;
    ULONG BytesFree = BytesLeft + BytesUnnecessary;
    ULONG BytesToCopy = *pBytesToWrite > BytesFree ? BytesFree : *pBytesToWrite;

    if (BytesToCopy > 0)
    {
        EnterCriticalSection(&pQB->CS);

        if (*pBytesToWrite > BytesLeft && BytesUnnecessary > 0)
        {
            ULONG BytesAvailable = pQB->WritePos - pQB->ReadPos;

            memmove(pQB->Buffer, pQB->Buffer + pQB->ReadPos, BytesAvailable);
            pQB->ReadPos = 0;
            pQB->WritePos = BytesAvailable;
        }

        try
        {
            memcpy(pQB->Buffer + pQB->WritePos, pSourceBuffer, BytesToCopy);
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {}

        pQB->WritePos += BytesToCopy;

        LeaveCriticalSection(&pQB->CS);
    }

    *pBytesToWrite = BytesToCopy;
}

void QueueBuffer_Reset(QueueBuffer * pQB)
{
    EnterCriticalSection(&pQB->CS);

    pQB->ReadPos = 0;
    pQB->WritePos = 0;

    LeaveCriticalSection(&pQB->CS);
}


//=========================================================
BOOL SL_Init(
             ULONG   Identifier,
             PVOID   pHead, // @parm points to device head
             PVOID   pMddHead // This is the first parm to callback
             )
{
    PVGD_INFO pHWHead = (PVGD_INFO)(pHead);

    LOGINIT;

    DEBUGMSG(ZONE_INIT,
        (TEXT("+SL_Init: pHWHead: 0x%X, Identifier: %d\r\n"), pHWHead, Identifier));

    pHWHead->flags = VGD_FLG_ABOUTSTR;
    pHWHead->curr_interruptIDs = INTR_NONE;

    DEBUGMSG(ZONE_CLOSE,
            (TEXT("-SL_Init\r\n"), pHWHead));

    return TRUE;
}

//=========================================================
BOOL SL_PostInit(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_INIT,(TEXT("+SL_PostInit, 0x%X\r\n"), pHead));
    DEBUGMSG (ZONE_INIT,(TEXT("-SL_PostInit, 0x%X\r\n"), pHead));

    return(TRUE);
}

//=========================================================
BOOL SL_Open(PVOID   pHead)
{
    BOOL RetVal = TRUE;
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_OPEN,
              (TEXT("+SL_Open 0x%X\r\n"), pHead));

    DEBUGMSG (ZONE_OPEN,
              (TEXT("-SL_Open 0x%X\r\n"), pHead));

    return RetVal;
}

//=========================================================
VOID SL_Close(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;
    PSER_INFO pSerHead = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_CLOSE,
        (TEXT("+SL_Close 0x%X, open count\r\n"), pHead, pSerHead->cOpenCount));

    DEBUGMSG (ZONE_CLOSE,
              (TEXT("-SL_Close 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_Deinit(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_INFO,
        (TEXT("+SL_Deinit, 0x%X\r\n"), pHWHead));

    DEBUGMSG (ZONE_INFO,
        (TEXT("-SL_Deinit, 0x%X\r\n"), pHWHead));

    LOGDONE;
}

//=========================================================
VOID SL_ClearDTR(PVOID pHead)
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_ClearDTR, 0x%X\r\n"), pHead));
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_ClearDTR, 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_SetDTR(PVOID pHead)
{    
    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetDTR, 0x%X\r\n"), pHead));
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetDTR, 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_ClearRTS(PVOID pHead)
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_ClearRTS, 0x%X\r\n"), pHead));
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_ClearRTS, 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_SetRTS(PVOID pHead)
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetRTS, 0x%X\r\n"), pHead));
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetRTS, 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_ClearBreak(PVOID pHead)
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_ClearBreak, 0x%X\r\n"), pHead));
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_ClearBreak, 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_SetBreak(PVOID pHead)
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetBreak, 0x%X\r\n"), pHead));
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetBreak, 0x%X\r\n"), pHead));
}

//=========================================================
BOOL SL_SetBaudRate(PVOID pHead, ULONG BaudRate)
{
    BOOL res = TRUE;
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
        (TEXT("+SL_SetBaudRate - pHWHead: 0x%X, BaudRate: %d\r\n"),
        pHead, BaudRate));

    pHWHead->dcb.BaudRate = BaudRate;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("-SL_SetBaudRate 0x%X (%d Baud)\r\n"), pHead, pHWHead->dcb.BaudRate));

    return res;
}

//=========================================================
BOOL VGD_SetByteSize(PVOID pHead, ULONG ByteSize)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+VGD_SetByteSize 0x%X, x%X\r\n"), pHead, ByteSize));

    pHWHead->dcb.ByteSize = (BYTE)ByteSize;

    DEBUGMSG (ZONE_FUNCTION,
        (TEXT("-VGD_SetByteSize\r\n")));

    return TRUE;
}

//=========================================================
BOOL VGD_SetParity(PVOID pHead, ULONG Parity)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+VGD_SetParity 0x%X, x%X\r\n"), pHead, Parity));

    pHWHead->dcb.Parity = (BYTE)Parity;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("-VGD_SetParity 0x%X\r\n"), pHead));

    return TRUE;
}

//=========================================================
BOOL VGD_SetStopBits(PVOID pHead, ULONG StopBits)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+VGD_SetStopBits 0x%X, x%X\r\n"), pHead, StopBits));

    pHWHead->dcb.StopBits = (BYTE)StopBits;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("-VGD_SetStopBits 0x%X\r\n"), pHead));

    return TRUE;
}

//=========================================================
ULONG SL_GetRxBufferSize(PVOID pHead)
{
    return VGD_BUFFER_SIZE;
}

//=========================================================
INTERRUPT_TYPE SL_GetInterruptType(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL_GetInterruptType, 0x%X\r\n"), pHead));

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("-SL_GetInterruptType, 0x%X, 0x%X\r\n"),
               pHead, pHWHead->curr_interruptIDs));

    return pHWHead->curr_interruptIDs;
}

//=========================================================
ULONG SL_RxIntr(PVOID pHead, PUCHAR pRxBuffer, ULONG *pBufflen)
{
#if (VGD_ABOUT_ENABLE == 1)
    static CHAR drv_about_string[] = VGD_ABOUT_STRING;
    static UCHAR drv_about_len = sizeof(drv_about_string);
    BOOL add_about = FALSE;
#endif

    PVGD_INFO pHWHead = (PVGD_INFO)pHead;
    ULONG dropped = 0;

    if(!pBufflen || 0 == *pBufflen)
        return (0);

    DEBUGMSG (ZONE_READ, (TEXT("+SL_RxIntr - 0x%X, 0x%X, %d.\r\n"),
                  pHead, pRxBuffer, *pBufflen));

#if VGD_ABOUT_ENABLE
    if((pHWHead->flags & VGD_FLG_ABOUTSTR) && *pBufflen > drv_about_len)
    {
        add_about = TRUE;
        memcpy(pRxBuffer, &drv_about_string, drv_about_len);
        *pBufflen -= drv_about_len;
        pRxBuffer += drv_about_len;
        pHWHead->flags &= ~VGD_FLG_ABOUTSTR;
    }
#endif

    /////////////////////////////////////////
    // read

    QueueBuffer_Read(&(pHWHead->buffer), pRxBuffer, pBufflen);

#if VGD_ABOUT_ENABLE
    if(add_about)
        *pBufflen += drv_about_len;
#endif

    pHWHead->curr_interruptIDs &= ~INTR_RX;

    DEBUGMSG (ZONE_READ, (TEXT("-SL_RxIntr - rx'ed %d.\r\n"), *pBufflen));

    return dropped;
}

//=========================================================
VOID SL_TxIntrEx(PVOID pHead, PUCHAR pTxBuffer, ULONG *pBufflen)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_WRITE,
        (TEXT("+SL_TxIntrEx 0x%X, Len %d\r\n"), pHead, (pBufflen)?*pBufflen:0));

    *pBufflen = 0;

    DEBUGMSG (ZONE_WRITE,
        (TEXT("-SL_TxIntrEx - sent %d.\r\n"), *pBufflen));
}

//=========================================================
VOID SL_LineIntr(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_READ,
              (TEXT("+SL_LineIntr 0x%X\r\n"), pHead));

    DEBUGMSG (ZONE_READ,
              (TEXT("-SL_LineIntr 0x%X\r\n"), pHead));
}

//=========================================================
VOID VGD_OtherIntr(PVOID pHead)
{
    DEBUGMSG (0,
              (TEXT("+VGD_OtherIntr 0x%X\r\n"), pHead));
    DEBUGMSG (0,
              (TEXT("-VGD_OtherIntr 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_ModemIntr(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_READ,
              (TEXT("+SL_ModemIntr 0x%X\r\n"), pHead));

    DEBUGMSG (ZONE_READ,
              (TEXT("-SL_ModemIntr 0x%X\r\n"), pHead));
}

//=========================================================
ULONG SL_GetStatus(PVOID pHead, LPCOMSTAT lpStat)
{
    return (1);
}

//=========================================================
VOID SL_Reset(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL_Reset 0x%X\r\n"), pHead));

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("-SL_Reset 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_GetModemStatus(PVOID pHead, PULONG pModemStatus)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL_GetModemStatus 0x%X\r\n"), pHead));

    *pModemStatus = 1;

    DEBUGMSG (ZONE_FUNCTION | ZONE_EVENTS,
              (TEXT("-SL_GetModemStatus 0x%X \r\n"), pHead));
}

//=========================================================
VOID SL_PurgeComm(PVOID pHead, DWORD fdwAction)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL_PurgeComm 0x%X\r\n"), pHead));

    QueueBuffer_Reset(&(pHWHead->buffer));

    DEBUGMSG (ZONE_FUNCTION,
        (TEXT("-SL_PurgeComm\r\n")));
}

//=========================================================
BOOL SL_XmitComChar(PVOID pHead, UCHAR ComChar)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL_XmitComChar 0x%X -> %d\r\n"), pHead, ComChar));

    DEBUGMSG (ZONE_FUNCTION,
        (TEXT("-SL_XmitComChar 0x%X\r\n"), pHead));

    return(TRUE);
}

//=========================================================
VOID SL_PowerOff(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;
    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL_PowerOff 0x%X\r\n"), pHead));
    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("-SL_PowerOff 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_PowerOn(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL_PowerOn 0x%X\r\n"), pHead));

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("-SL_PowerOn 0x%X\r\n"), pHead));
}

//=========================================================
BOOL SL_SetDCB(PVOID pHead, LPDCB lpDCB)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL_SetDCB 0x%X\r\n"), pHead));

    pHWHead->dcb = *lpDCB;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("-SL_SetDCB 0x%X\r\n"), pHead));

    return (TRUE);
}

//=========================================================
ULONG SL_SetCommTimeouts(PVOID pHead, LPCOMMTIMEOUTS lpCommTimeouts)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;
    PHW_INDEP_INFO pSerialHead = (PHW_INDEP_INFO)((PSER_INFO)pHead)->pMddHead;
    ULONG retval = 0;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("+SL_SetCommTimeouts 0x%X, 0x%X\r\n"), pHead, pSerialHead));

    // Now that we have done the right thing, store this DCB
    pSerialHead->CommTimeouts = *lpCommTimeouts;

    // fixed timeout for write
    pSerialHead->CommTimeouts.WriteTotalTimeoutMultiplier = VGD_WRITE_TIMEOUT_MULTIPLIER;
    pSerialHead->CommTimeouts.WriteTotalTimeoutConstant = VGD_WRITE_TIMEOUT_CONSTANT;

    DEBUGMSG (ZONE_FUNCTION,
              (TEXT("-SL_SetCommTimeouts 0x%X\r\n"), pHead));

    return(retval);
}

//=========================================================
BOOL SL_Ioctl(PVOID pHead, DWORD dwCode,PBYTE pBufIn,DWORD dwLenIn,
         PBYTE pBufOut,DWORD dwLenOut,PDWORD pdwActualOut)
{
    BOOL RetVal = TRUE;
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;
    PSER_INFO pHSer = (PSER_INFO)pHead;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_Ioctl 0x%X, 0x%X\r\n"), pHead, pHWHead));

    switch (dwCode)
    {
    case VGD_IOCTL_PUSHDATA:
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): VGD_IOCTL_PUSHDATA...\r\n")));

        if ( (dwLenIn < 0) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        QueueBuffer_Write(&(pHWHead->buffer), pBufIn, &dwLenIn);
        SerialEventHandler((PHW_INDEP_INFO)pHSer->pMddHead);

        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): lat = %f\r\n"), pHWHead->nmea_info->lat));

        break;

    default:
        RetVal = FALSE;
        DEBUGMSG (ZONE_FUNCTION, (TEXT(" Unsupported ioctl 0x%X\r\n"), dwCode));
        break;            
    }

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_Ioctl 0x%X\r\n"), pHead));

    return(RetVal);
}
