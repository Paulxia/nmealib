#include "log.h"

#include <windows.h>
#include <stdlib.h>

#if VGD_USE_LOG

typedef struct
{
    PTCHAR file_name;
    CRITICAL_SECTION cs;

} LOG_INTERNAL_DATA, *PLOG_INTERNAL_DATA;

PLOG_INTERNAL_DATA __log_data = 0;

//=========================================================
BOOL log_init(const PTCHAR file_name)
{
    FILE *file;

    if(!__log_data)
    {    
        file = _tfopen(file_name, _T("a"));

        if(file)
        {
            if(__log_data = (PLOG_INTERNAL_DATA)LocalAlloc(LPTR, sizeof(LOG_INTERNAL_DATA)))
            {
                __log_data->file_name = _tcsdup(file_name);
                InitializeCriticalSection(&(__log_data->cs));
            }
            fclose(file);
        }
    }

    return (0 != __log_data && 0 != __log_data->file_name);
}

//=========================================================
void log_done()
{
    if(__log_data)
    {
        if(__log_data->file_name)
            free(__log_data->file_name);
        DeleteCriticalSection(&(__log_data->cs));
    }
}

//=========================================================
void log_push(int level, const PTCHAR format, ...)
{
    va_list alist;
    va_start(alist, (format));
    log_args(level, format, &alist);
    va_end(alist);
}

//=========================================================
void log_args(int level, const PTCHAR format, va_list *args)
{
    FILE *file;
    SYSTEMTIME st;
    DWORD tid;

    if(__log_data)
    {
        EnterCriticalSection(&(__log_data->cs));
        
        file = _tfopen(__log_data->file_name, _T("a"));

        if(file)
        {
            GetLocalTime(&st);
            tid = GetCurrentThreadId();

            _ftprintf(file, _T("[%02d%02d%04d %02d%02d%02d.%03d TID 0x%X] "),
                st.wDay, st.wMonth, st.wYear,
                st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
                tid);

            switch(level)
            {
            case LevelTrace:
                _ftprintf(file, _T("[i] "));
                break;
            case LevelWarning:
                _ftprintf(file, _T("[W] "));
                break;
            case LevelError:
                _ftprintf(file, _T("[E] "));
                break;
            }

            _vftprintf(file, format, *args);
            _ftprintf(file, _T("\n"));

            fclose(file);
        }

        LeaveCriticalSection(&(__log_data->cs));
    }    
}

//=========================================================
void log_none(const PTCHAR format, ...)
{
    va_list alist;
    va_start(alist, (format));
    log_args(-1, format, &alist);
    va_end(alist);
}

//=========================================================
void log_trace(const PTCHAR format, ...)
{
    va_list alist;
    va_start(alist, (format));
    log_args(LevelTrace, format, &alist);
    va_end(alist);
}

//=========================================================
void log_warning(const PTCHAR format, ...)
{
    va_list alist;
    va_start(alist, (format));
    log_args(LevelWarning, format, &alist);
    va_end(alist);
}

//=========================================================
void log_error(const PTCHAR format, ...)
{
    va_list alist;
    va_start(alist, (format));
    log_args(LevelError, format, &alist);
    va_end(alist);
}

#else

BOOL log_init(const PTCHAR file_name) { return TRUE; }
void log_done() {}
void log_push(int level, const PTCHAR format, ...) {}
void log_args(int level, const PTCHAR format, va_list *args) {}
void log_trace(const PTCHAR format, ...) {}
void log_warning(const PTCHAR format, ...) {}
void log_error(const PTCHAR format, ...) {}

#endif // VGD_USE_LOG
