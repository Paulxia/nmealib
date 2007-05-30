///////////////////////////////////////////////////////////
//
// NMEA library
// URL: http://nmea.sourceforge.net
// Author: Tim (xtimor@gmail.com)
// Licence: http://www.gnu.org/licenses/lgpl.html
// $Id$
//
///////////////////////////////////////////////////////////

#include <memory.h>

#include "nmea/info.h"

void nmea_zero_INFO(nmeaINFO *info)
{
    memset(info, 0, sizeof(nmeaINFO));
    nmea_time_now(&info->utc);
    info->sig = NMEA_SIG_BAD;
    info->fix = NMEA_FIX_BAD;
}
