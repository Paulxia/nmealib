/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id$
 *
 */

/*! \file */

#include "os.h"

#ifdef NMEA_WIN
#   include <windows.h>
#endif

void * os_mutex_init(void)
{
#ifdef NMEA_WIN
    LPCRITICAL_SECTION pcs = (LPCRITICAL_SECTION)os_malloc(sizeof(CRITICAL_SECTION));

    if(!pcs)
        return 0;

    InitializeCriticalSection(pcs);

    return (void *)pcs;
#endif
}

int os_mutex_lock(void *h)
{
#ifdef NMEA_WIN
    LPCRITICAL_SECTION pcs = (LPCRITICAL_SECTION)h;
    EnterCriticalSection(pcs);
    return 0;
#endif
}

int os_mutex_unlock(void *h)
{
#ifdef NMEA_WIN
    LPCRITICAL_SECTION pcs = (LPCRITICAL_SECTION)h;
    LeaveCriticalSection(pcs);
    return 0;
#endif
}

void os_mutex_done(void *h)
{
#ifdef NMEA_WIN
    LPCRITICAL_SECTION pcs = (LPCRITICAL_SECTION)h;
    DeleteCriticalSection(pcs);
#endif
}
