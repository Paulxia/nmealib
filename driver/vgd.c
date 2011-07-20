////////////////////////////////////////////////////////////////////////////
//
// Module Name  : vgd.c
// Author       : Tim (spaceflush@mail.ru)
// Date         : November 2006
//
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <serhw.h>
#include <hw16550.h>

#include <nmea/tools.h>

#include "config.h"
#include "vgd.h"
#include "pdd.h"
#include "mdd.h"
#include "log.h"

BOOL VGD_SetByteSize(PVOID pHead, ULONG ByteSize);
BOOL VGD_SetStopBits(PVOID pHead, ULONG StopBits);
BOOL VGD_SetParity(PVOID pHead, ULONG Parity);

///////////////////////////////////////////////////////////
// Useful macros
///////////////////////////////////////////////////////////

#define VGD_BEG_OP(phead) \
    EnterCriticalSection(&(((PVGD_INFO)phead)->allow_mutex))
#define VGD_END_OP(phead) \
    LeaveCriticalSection(&(((PVGD_INFO)phead)->allow_mutex))
    
//=========================================================
VOID VGD_GenCycle(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)(pHead);
    PHW_INDEP_INFO pIndepHead = (PHW_INDEP_INFO)(((PSER_INFO)pHead)->pMddHead);
    pHWHead->curr_interruptIDs |= INTR_RX;
    SerialEventHandler(pIndepHead);
}

//=========================================================
static DWORD WINAPI VGD_TimerThread(PVOID pContext)
{
    PVGD_INFO pHWHead = (PVGD_INFO)(pContext);

    DEBUGMSG(ZONE_INIT,
        (TEXT("+VGD_TimerThread: pHWHead: 0x%X\r\n"), pHWHead));

    do {

        VGD_GenCycle(pContext);

    } while(WAIT_OBJECT_0 != WaitForSingleObject(
        pHWHead->stop_thread_event, pHWHead->timer_timeout));

    SetEvent(pHWHead->kill_thread_event);

    DEBUGMSG(ZONE_INIT,
        (TEXT("-VGD_TimerThread: pHWHead: 0x%X\r\n"), pHWHead));

    return 0;
}

//=========================================================
BOOL VGD_StartTimerThread(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)(pHead);

    DEBUGMSG(ZONE_INIT,
        (TEXT("+VGD_StartTimerThread: pHWHead: 0x%X\r\n"), pHWHead));

    VGD_BEG_OP(pHWHead);

    if(!pHWHead->timer_thread)
    {
        pHWHead->stop_thread_event =
            CreateEvent(0, FALSE, FALSE, NULL);
        pHWHead->kill_thread_event =
            CreateEvent(0, FALSE, FALSE, NULL);
        pHWHead->timer_thread =
            CreateThread(NULL, 0, VGD_TimerThread, pHead, 0, NULL);

        if(pHWHead->timer_thread)
            CeSetThreadPriority(pHWHead->timer_thread, VGD_THREAD_PRIORITY);
        else
        {
            CloseHandle(pHWHead->stop_thread_event);
            CloseHandle(pHWHead->kill_thread_event);
            pHWHead->stop_thread_event = NULL;
            pHWHead->kill_thread_event = NULL;
        }
    }

    VGD_END_OP(pHWHead);

    DEBUGMSG(ZONE_INIT,
        (TEXT("-VGD_StartTimerThread: pHWHead: 0x%X, timer_thread: 0x%X\r\n"), pHWHead->timer_thread));

    return NULL != pHWHead->timer_thread;
}


//=========================================================
VOID VGD_StopTimerThread(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)(pHead);

    DEBUGMSG(ZONE_INIT,
        (TEXT("+VGD_StopTimerThread: pHWHead: 0x%X\r\n"), pHWHead));

    VGD_BEG_OP(pHWHead);

    if(pHWHead->timer_thread)
    {
        VGD_END_OP(pHWHead);

        SetEvent(pHWHead->stop_thread_event);
        WaitForSingleObject(pHWHead->kill_thread_event, INFINITE);

        VGD_BEG_OP(pHWHead);

        CloseHandle(pHWHead->timer_thread);
        CloseHandle(pHWHead->stop_thread_event);
        CloseHandle(pHWHead->kill_thread_event);
        pHWHead->kill_thread_event = NULL;
        pHWHead->stop_thread_event = NULL;
        pHWHead->timer_thread = NULL;
    }

    VGD_END_OP(pHWHead);

    DEBUGMSG(ZONE_INIT,
        (TEXT("-VGD_StopTimerThread\r\n"), pHWHead));
}

//=========================================================
BOOL VGD_PushTapFile(PUCHAR pRxBuffer, ULONG Bufflen)
{
    BOOL res;
    FILE *file;

    file = _tfopen(VGD_TAP_FILE, _T("ab"));

    if(!file)
        res = FALSE;
    else
    {
        fwrite((PVOID)pRxBuffer, sizeof(UCHAR), Bufflen, file);
        fclose(file);
        res = TRUE;
    }

    return res;
};

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

    pHWHead->nmea_pack_mask = GPGGA | GPGSA | GPGSV | GPRMC;
    pHWHead->nmea_info = malloc(sizeof(nmeaINFO));
    pHWHead->nmea_gen = nmea_create_generator(NMEA_GEN_ROTATE, pHWHead->nmea_info);
    pHWHead->flags = VGD_FLG_ABOUTSTR;
    pHWHead->timer_timeout = VGD_DEFAULT_DATATIMER;
    pHWHead->timer_thread = NULL;
    pHWHead->stop_thread_event = NULL;
    pHWHead->kill_thread_event = NULL;
    pHWHead->curr_interruptIDs = INTR_NONE;

    InitializeCriticalSection(&(pHWHead->allow_mutex));

    DEBUGMSG(ZONE_CLOSE,
            (TEXT("-SL_Init\r\n"), pHWHead));

    return 0 != pHWHead->nmea_gen;
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

    RetVal = VGD_StartTimerThread(pHWHead);

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

    if(pSerHead->cOpenCount <= 0)
        VGD_StopTimerThread(pHWHead);

    DEBUGMSG (ZONE_CLOSE,
              (TEXT("-SL_Close 0x%X\r\n"), pHead));
}

//=========================================================
VOID SL_Deinit(PVOID pHead)
{
    PVGD_INFO pHWHead = (PVGD_INFO)pHead;

    DEBUGMSG (ZONE_INFO,
        (TEXT("+SL_Deinit, 0x%X\r\n"), pHWHead));

    VGD_StopTimerThread(pHWHead);

    nmea_gen_destroy(pHWHead->nmea_gen);
    free(pHWHead->nmea_info);

    DeleteCriticalSection(&(pHWHead->allow_mutex));

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

    VGD_BEG_OP(pHWHead);

    *pBufflen = nmea_generate_from(
        pRxBuffer, *pBufflen,
        pHWHead->nmea_info, pHWHead->nmea_gen,
        GPGGA | GPGSA | GPGSV | GPRMC | GPVTG
        );

    VGD_END_OP(pHWHead);

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

    VGD_BEG_OP(pHWHead);

    nmea_gen_reset(pHWHead->nmea_gen, pHWHead->nmea_info);

    VGD_END_OP(pHWHead);

    VGD_GenCycle(pHWHead);

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
    int gen_index;

    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_Ioctl 0x%X, 0x%X\r\n"), pHead, pHWHead));

    VGD_BEG_OP(pHead);

    switch (dwCode)
    {
    case VGD_IOCTL_SETLAT:
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): VGD_IOCTL_SETLAT...\r\n")));
        if ( (dwLenIn != sizeof(double)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        pHWHead->nmea_info->lat = fabs(*(double *)pBufIn);
        pHWHead->nmea_info->ns = nmea_ns_get(*(double *)pBufIn);
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): lat = %f\r\n"), pHWHead->nmea_info->lat));
        break;

    case VGD_IOCTL_SETLON:
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): VGD_IOCTL_SETLON...\r\n")));
        if ( (dwLenIn != sizeof(double)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        pHWHead->nmea_info->lon = fabs(*(double *)pBufIn);
        pHWHead->nmea_info->ew = nmea_ew_get(*(double *)pBufIn);
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): lon = %f\r\n"), pHWHead->nmea_info->lon));
        break;

    case VGD_IOCTL_GENIDX:
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): VGD_IOCTL_GENIDX...\r\n")));
        if ( (dwLenIn < sizeof(int)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        gen_index = *(int *)pBufIn;

        if( gen_index < 0 || gen_index >= NMEA_GEN_LAST ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        nmea_destroy_generator(pHWHead->nmea_gen);
        pHWHead->nmea_gen = nmea_create_generator(gen_index, pHWHead->nmea_info);

        break;

    case VGD_IOCTL_ADDGEN:
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): VGD_IOCTL_ADDGEN...\r\n")));
        if ( (dwLenIn < sizeof(int)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        gen_index = *(int *)pBufIn;

        if( gen_index < 0 || gen_index >= NMEA_GEN_LAST ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        nmea_gen_add(pHWHead->nmea_gen,
            nmea_create_generator(gen_index, pHWHead->nmea_info));

        break;

    case VGD_IOCTL_SETSIG:
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): VGD_IOCTL_SETSIG...\r\n")));
        if ( (dwLenIn < sizeof(int)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        pHWHead->nmea_info->sig = (unsigned char)*(int *)pBufIn;
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): sig = %f\r\n"), pHWHead->nmea_info->sig));
        break;

    case VGD_IOCTL_SETFIX:
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): VGD_IOCTL_SETFIX...\r\n")));
        if ( (dwLenIn < sizeof(int)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        pHWHead->nmea_info->fix = (unsigned char)*(int *)pBufIn;
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): fix = %f\r\n"), pHWHead->nmea_info->fix));
        break;

    case VGD_IOCTL_GETINFO:
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): VGD_IOCTL_GETINFO...\r\n")));
        if ( (dwLenOut < sizeof(nmeaINFO *)) || (NULL == pBufOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        *((nmeaINFO *)pBufOut) = *(pHWHead->nmea_info);
        break;    

    case VGD_IOCTL_SETINFO:
        DEBUGMSG (ZONE_IOCTL,
            (TEXT("SL_Ioctl (IOCTL): VGD_IOCTL_SETINFO...\r\n")));
        if ( (dwLenIn < sizeof(nmeaINFO *)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        *(pHWHead->nmea_info) = *((nmeaINFO *)pBufIn);
        break;    

    default:
        RetVal = FALSE;
        DEBUGMSG (ZONE_FUNCTION, (TEXT(" Unsupported ioctl 0x%X\r\n"), dwCode));
        break;            
    }

    VGD_END_OP(pHead);

    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_Ioctl 0x%X\r\n"), pHead));

    return(RetVal);
}
