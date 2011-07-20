///////////////////////////////////////////////////////////
//
// NMEA library
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: info.c 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#include <memory.h>

#include "info.h"

void nmea_zero_INFO(nmeaINFO *info)
{
    memset(info, 0, sizeof(nmeaINFO));
    nmea_time_now(&info->utc);
    info->sig = NMEA_SIG_BAD;
    info->fix = NMEA_FIX_BAD;
}
