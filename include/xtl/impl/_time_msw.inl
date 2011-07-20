///////////////////////////////////////////////////////////
//
// Project: XTL
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: mkspec.h 27 2007-04-04 19:33:20Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __XTL_TIME_MSW_INL
#define __XTL_TIME_MSW_INL

#include <windows.h>

__XTL_BEGIN_NAMESPACE

template<class _TimeTy>
class timespec_msw
{
public:
    static _TimeTy mktime(const tm &t)
    {
        static const FILETIME f1970 = YearToFileTime(1970); 
        SYSTEMTIME s = TmToSystemTime(t);
        FILETIME f;
        SystemTimeToFileTime(&s, &f);
        return (_TimeTy)GetDeltaSecs(f1970, f);
    }

    static tm localtime(_TimeTy t)
    {
        SYSTEMTIME s;
        FILETIME f, fs = Int64ToFileTime((__int64)t);
        ::FileTimeToLocalFileTime(&fs, &f);
        if(!::FileTimeToSystemTime(&f, &s))
            return tm();
        return SystemTimeToTm(s);
    }

    static tm gmtime(_TimeTy t)
    {
        SYSTEMTIME s;
        FILETIME f, fs = Int64ToFileTime((__int64)t);
        if(!::FileTimeToSystemTime(&f, &s))
            return tm();
        return SystemTimeToTm(s);
    }

    static _TimeTy now()
    {
        SYSTEMTIME s;
        GetLocalTime(&s);
        tm t = SystemTimeToTm(s); 
        return mktime(t);
    }

    static _TimeTy clock()
    {
        return (_TimeTy)::GetTickCount();
    }

    static timezone zone()
    {
        TIME_ZONE_INFORMATION tz;
        ::GetTimeZoneInformation(&tz);
        timezone rtz;
        rtz.bias = tz.Bias * 60;
        rtz.daylight = tz.DaylightBias;
        rtz.name = wctot(tz.StandardName);
        rtz.dst = wctot(tz.DaylightName);
        return rtz;
    }

    // Accessory functions

    static __int64 GetDeltaSecs(FILETIME f1, FILETIME f2)
    {
        __int64 t1 = f1.dwHighDateTime;
        t1 <<= 32;				
        t1 |= f1.dwLowDateTime;

        __int64 t2 = f2.dwHighDateTime;
        t2 <<= 32;				
        t2 |= f2.dwLowDateTime;

        __int64 iTimeDiff = (t2 - t1) / 10000000;
        return iTimeDiff;
    }

    static SYSTEMTIME TmToSystemTime(tm &t)
    {
        SYSTEMTIME s;
        s.wYear = t.year() + 1900;
        s.wMonth = t.month() + 1;
        s.wDayOfWeek = t.wday();
        s.wDay = t.day();
        s.wHour = t.hour();
        s.wMinute = t.min();
        s.wSecond = t.sec();
        s.wMilliseconds = 0;
        return s;
    }

    static FILETIME YearToFileTime(WORD wYear)
    {	
        SYSTEMTIME sbase;
        sbase.wYear = wYear;
        sbase.wMonth = 1;
        sbase.wDayOfWeek = 1;
        sbase.wDay = 1;
        sbase.wHour = 0;
        sbase.wMinute = 0;
        sbase.wSecond = 0;
        sbase.wMilliseconds = 0;
        FILETIME fbase;
        ::SystemTimeToFileTime(&sbase, &fbase);
        return fbase;
    }

    static FILETIME Int64ToFileTime(__int64 iTime)
    {
        FILETIME f;
        f.dwHighDateTime = (DWORD)((iTime >> 32) & 0x00000000FFFFFFFF);
        f.dwLowDateTime  = (DWORD)(iTime & 0x00000000FFFFFFFF);
        return f;
    }

    static int SystemTimeToYDay(SYSTEMTIME s)
    {
        FILETIME fMidnightJan1 = YearToFileTime(s.wYear);
        FILETIME f; ::SystemTimeToFileTime(&s, &f);
        return (int)(GetDeltaSecs(fMidnightJan1, f) / (__int64)86400);
    }

    static tm SystemTimeToTm(SYSTEMTIME &s)
    {
        tm t(
            s.wYear - 1900,
            s.wMonth-1,
            s.wDay,
            s.wHour,
            s.wMinute,
            s.wSecond,
            s.wDayOfWeek,
            SystemTimeToYDay(s));
        return t;
    }
};

#ifdef __XTL_MAXBITS_64
    typedef time_base<__int64, timespec_msw<__int64> > time;
#else
    typedef time_base<DWORD, timespec_msw<DWORD> > time;
#endif

__XTL_END_NAMESPACE

#endif /* __XTL_TIME_MSW_INL */
