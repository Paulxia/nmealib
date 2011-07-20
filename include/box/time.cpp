///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#include "time.h"
#include "config.h"

#include <memory.h>

#ifdef BOX_WIN
#   include <windows.h>
#else
#   include <time.h>
#endif

namespace box
{

///////////////////////////////////////////////////////////
// Time
///////////////////////////////////////////////////////////

Time::Time()
{
    memset(this, 0, sizeof(Time));
}

//=========================================================
Time::Time(const Time &src)
{
    memcpy(this, &src, sizeof(Time));
}

//=========================================================
Time & Time::operator = (const Time &src)
{
    memcpy(this, &src, sizeof(Time));
    return *this;
}

//=========================================================
Time Time::now()
{
    Time tm;

#ifdef BOX_WIN
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    tm._mday = (unsigned char)st.wDay;
    tm._wday = (unsigned char)st.wDayOfWeek;
    tm._mon = (unsigned char)st.wMonth;
    tm._year = (unsigned short)st.wYear;
    tm._hour = (unsigned char)st.wHour;
    tm._min = (unsigned char)st.wMinute;
    tm._sec = (unsigned char)st.wSecond;
    tm._msec = (unsigned short)st.wMilliseconds;
#else
    struct tm *st;
    time_t tt;
    clock_t ct;

    time(&tt);
    st = ::localtime(&tt);
    ct = ::clock();

    tm._mday = (unsigned char)st->tm_mday;
    tm._wday = (unsigned char)st->tm_wday;
    tm._mon = (unsigned char)st->tm_mon + 1;
    tm._year = (unsigned short)st->tm_year + 1900;
    tm._hour = (unsigned char)st->tm_hour;
    tm._min = (unsigned char)st->tm_min;
    tm._sec = (unsigned char)st->tm_sec;
    tm._msec = (unsigned short)(ct / (CLOCKS_PER_SEC / 1000));
#endif

    return tm;
}

} // namespace box
